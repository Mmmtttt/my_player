// tutorial03.c
// A pedagogical video player that will stream through every video frame as fast as it can
// and play audio (out of sync).
//
// This tutorial was written by Stephen Dranger (dranger@gmail.com).
//
// Code based on FFplay, Copyright (c) 2003 Fabrice Bellard, 
// and a tutorial by Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)
// Tested on Gentoo, CVS version 5/01/07 compiled with GCC 4.1.1
//
// Updates tested on:
// Mac OS X 10.11.6
// Apple LLVM version 8.0.0 (clang-800.0.38)
//
// Use 
//
// $ gcc -o tutorial03 tutorial03.c -lavutil -lavformat -lavcodec -lswscale -lz -lm `sdl-config --cflags --libs`
//
// to build (assuming libavutil/libavformat/libavcodec/libswscale are correctly installed your system).
//
// Run using
//
// $ tutorial03 myvideofile.mpg
//
// to play the stream on your screen with voice.

/*---------------------------
//1、消息队列处理函数在处理消息前，先对互斥量进行锁定，以保护消息队列中的临界区资源
//2、若消息队列为空，则调用pthread_cond_wait对互斥量暂时解锁，等待其他线程向消息队列中插入消息数据
//3、待其他线程向消息队列中插入消息数据后，通过pthread_cond_signal像等待线程发出qready信号
//4、消息队列处理线程收到qready信号被唤醒，重新获得对消息队列临界区资源的独占

#include <pthread.h>

struct msg{//消息队列结构体
	struct msg *m_next;//消息队列后继节点
	//more stuff here
}

struct msg *workq;//消息队列指针
pthread_cond_t qready=PTHREAD_COND_INITIALIZER;//消息队列就绪条件变量
pthread_mutex_t qlock=PTHREAS_MUTEX_INITIALIZER;//消息队列互斥量，保护消息队列数据

//消息队列处理函数
void process_msg(void){
	struct msg *mp;//消息结构指针
	for(;;){
		pthread_mutex_lock(&qlock);//消息队列互斥量加锁，保护消息队列数据
		while(workq==NULL){//检查消息队列是否为空，若为空
			pthread_cond_wait(&qready,&qlock);//等待消息队列就绪信号qready，并对互斥量暂时解锁，该函数返回时，互斥量再次被锁住
		}
		mp=workq;//线程醒来，从消息队列中取数据准备处理
		workq=mp->m_next;//更新消息队列，指针后移清除取出的消息
		pthread_mutex_unlock(&qlock);//释放锁
		//now process the message mp
	}
}

//将消息插入消息队列
void enqueue_msg(struct msg *mp){
	pthread_mutex_lock(&qlock);//消息队列互斥量加锁，保护消息队列数据
	mp->m_next=workq;//将原队列头作为插入消息的后继节点
	workq=mp;//将新消息插入队列
	pthread_cond_signal(&qready);//给等待线程发出qready消息，通知消息队列已就绪
	pthread_mutex_unlock(&qlock);//释放锁
} 
---------------------------*/

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include <SDL.h>
#include <SDL_thread.h>

#ifdef __MINGW32__
#undef main // Prevents SDL from overriding main().
#endif

#include <stdio.h>

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

int quit = 0;//全局退出进程标识，在界面上点了退出后，告诉线程退出
/*-------链表节点结构体--------
typedef struct AVPacketList {
    AVPacket pkt;//链表数据
    struct AVPacketList *next;//链表后继节点
} AVPacketList;
---------------------------*/
//数据包队列(链表)结构体
typedef struct PacketQueue {
	AVPacketList *first_pkt, *last_pkt;//队列首尾节点指针
	int nb_packets;//队列长度
	int size;//保存编码数据的缓存长度，size=packet->size
	SDL_mutex *qlock;//队列互斥量，保护队列数据
	SDL_cond *qready;//队列就绪条件变量
} PacketQueue;
PacketQueue audioq;//定义全局队列对象

//队列初始化函数
void packet_queue_init(PacketQueue *q) {
	memset(q, 0, sizeof(PacketQueue));//全零初始化队列结构体对象
	q->qlock = SDL_CreateMutex();//创建互斥量对象
	q->qready = SDL_CreateCond();//创建条件变量对象
}

//向队列中插入数据包
int packet_queue_put(PacketQueue *q, AVPacket *pkt) {
/*-------准备队列(链表)节点对象------*/
	AVPacketList *pktlist;//创建链表节点对象指针
	pktlist = av_malloc(sizeof(AVPacketList));//在堆上创建链表节点对象
	if (!pktlist) {//检查链表节点对象是否创建成功
		return -1;
	}
	pktlist->pkt = *pkt;//将输入数据包赋值给新建链表节点对象中的数据包对象
	pktlist->next = NULL;//链表后继指针为空
//	if (av_packet_ref(pkt, pkt)<0) {//增加pkt编码数据的引用计数(输入参数中的pkt与新建链表节点中的pkt共享同一缓存空间)
//		return -1;
//	}
/*---------将新建节点插入队列-------*/
	SDL_LockMutex(q->qlock);//队列互斥量加锁，保护队列数据
	
	if (!q->last_pkt) {//检查队列尾节点是否存在(检查队列是否为空)
		q->first_pkt = pktlist;//若不存在(队列尾空)，则将当前节点作队列为首节点
	}
	else {
		q->last_pkt->next = pktlist;//若已存在尾节点，则将当前节点挂到尾节点的后继指针上，并作为新的尾节点
	}
	q->last_pkt = pktlist;//将当前节点作为新的尾节点
	q->nb_packets++;//队列长度+1
	q->size += pktlist->pkt.size;//更新队列编码数据的缓存长度
	
	SDL_CondSignal(q->qready);//给等待线程发出消息，通知队列已就绪
	
	SDL_UnlockMutex(q->qlock);//释放互斥量
	return 0;
}

//从队列中提取数据包，并将提取的数据包出队列
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block) {
	AVPacketList *pktlist;//临时链表节点对象指针
	int ret;//操作结果
	
	SDL_LockMutex(q->qlock);//队列互斥量加锁，保护队列数据
	for (;;) {
		if (quit) {//检查退出进程标识
			ret = -1;//操作失败
			break;
		}
		
		pktlist = q->first_pkt;//传递将队列首个数据包指针
		if (pktlist) {//检查数据包是否为空(队列是否有数据)
			q->first_pkt = pktlist->next;//队列首节点指针后移
			if (!q->first_pkt) {//检查首节点的后继节点是否存在
				q->last_pkt = NULL;//若不存在，则将尾节点指针置空
			}
			q->nb_packets--;//队列长度-1
			q->size -= pktlist->pkt.size;//更新队列编码数据的缓存长度
			*pkt = pktlist->pkt;//将队列首节点数据返回
			av_free(pktlist);//清空临时节点数据(清空首节点数据，首节点出队列)
			ret = 1;//操作成功
			break;
		} else if (!block) {
			ret = 0;
			break;
		} else {//队列处于未就绪状态，此时通过SDL_CondWait函数等待qready就绪信号，并暂时对互斥量解锁
			/*---------------------
			 * 等待队列就绪信号qready，并对互斥量暂时解锁
			 * 此时线程处于阻塞状态，并置于等待条件就绪的线程列表上
			 * 使得该线程只在临界区资源就绪后才被唤醒，而不至于线程被频繁切换
			 * 该函数返回时，互斥量再次被锁住，并执行后续操作
			 --------------------*/
			SDL_CondWait(q->qready, q->qlock);//暂时解锁互斥量并将自己阻塞，等待临界区资源就绪(等待SDL_CondSignal发出临界区资源就绪的信号)
		}
	}//end for for-loop
	SDL_UnlockMutex(q->qlock);//释放互斥量
	return ret;
}

/*---------------------------
 * 从缓存队列中提取数据包、解码，并返回解码后的数据长度(对一个完整的packet解码，将解码数据写入audio_buf缓存，并返回多帧解码数据的总长度)
 * aCodecCtx:音频解码器上下文
 * audio_buf：保存解码一个完整的packe后的原始音频数据(缓存中可能包含多帧解码后的音频数据)
 * buf_size：解码后的音频数据长度，未使用
 --------------------------*/
int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size) {
	static AVPacket pkt;//保存从队列中提取的数据包
	static AVFrame frame;//保存从数据包中解码的音频数据
	static uint8_t *audio_pkt_data = NULL;//保存数据包编码数据缓存指针
	static int audio_pkt_size = 0;//数据包中剩余的编码数据长度
	int coded_consumed_size, data_size = 0;//每次消耗的编码数据长度[input](len1)，输出原始音频数据的缓存长度[output]
	
	for (;;) {
		while(audio_pkt_size>0) {//检查缓存中剩余的编码数据长度(是否已完成一个完整的pakcet包的解码，一个数据包中可能包含多个音频编码帧)
			int got_frame = 0;//解码操作成功标识，成功返回非零值
			coded_consumed_size=avcodec_decode_audio4(aCodecCtx,&frame,&got_frame,&pkt);//解码一帧音频数据，并返回消耗的编码数据长度
			if (coded_consumed_size < 0) {//检查是否执行了解码操作
				// if error, skip frame.
				audio_pkt_size = 0;//更新编码数据缓存长度
				break;
			}
			audio_pkt_data += coded_consumed_size;//更新编码数据缓存指针位置
			audio_pkt_size -= coded_consumed_size;//更新缓存中剩余的编码数据长度
			if (got_frame) {//检查解码操作是否成功
				//计算解码后音频数据长度[output]
				data_size=av_samples_get_buffer_size(NULL,aCodecCtx->channels,frame.nb_samples,aCodecCtx->sample_fmt,1);
				memcpy(audio_buf, frame.data[0], data_size);//将解码数据复制到输出缓存
			}
			if (data_size <= 0) {//检查输出解码数据缓存长度
				// No data yet, get more frames.
				continue;
			}
			// We have data, return it and come back for more later.
			return data_size;//返回解码数据缓存长度
		}//end for while

		if (pkt.data) {//检查数据包是否已从队列中提取
			av_packet_unref(&pkt);//释放pkt中保存的编码数据
		}
		
		if (quit) {//检查退出进程标识
			return -1;
		}
		//从队列中提取数据包到pkt
		if (packet_queue_get(&audioq, &pkt,1)<0) {
			return -1;
		}
		audio_pkt_data = pkt.data;//传递编码数据缓存指针
		audio_pkt_size = pkt.size;//传递编码数据缓存长度
	}//end for for-loop
}

/*------Audio Callback-------
 * 音频输出回调函数，sdl通过该回调函数将解码后的pcm数据送入声卡播放,
 * sdl通常一次会准备一组缓存pcm数据，通过该回调送入声卡，声卡根据音频pts依次播放pcm数据
 * 待送入缓存的pcm数据完成播放后，再载入一组新的pcm缓存数据(每次音频输出缓存为空时，sdl就调用此函数填充音频输出缓存，并送入声卡播放)
 * When we begin playing audio, SDL will continually call this callback function 
 * and ask it to fill the audio buffer with a certain number of bytes
 * The audio function callback takes the following parameters: 
 * stream: A pointer to the audio buffer to be filled，输出音频数据到声卡缓存
 * len: The length (in bytes) of the audio buffer,缓存长度wanted_spec.samples=SDL_AUDIO_BUFFER_SIZE(1024)
 --------------------------*/ 
void audio_callback(void *userdata, Uint8 *stream, int len) {
	AVCodecContext *aCodecCtx = (AVCodecContext *)userdata;//传递用户数据
	int wt_stream_len, audio_size;//每次写入stream的数据长度，解码后的数据长度
	
	static uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE*3)/2];//保存解码一个packet后的多帧原始音频数据
	static unsigned int audio_buf_size = 0;//解码后的多帧音频数据长度
	static unsigned int audio_buf_index = 0;//累计写入stream的长度
	
	while (len>0) {//检查音频缓存的剩余长度
		if (audio_buf_index >= audio_buf_size) {//检查是否需要执行解码操作
			// We have already sent all our data; get more，从缓存队列中提取数据包、解码，并返回解码后的数据长度，audio_buf缓存中可能包含多帧解码后的音频数据
			audio_size = audio_decode_frame(aCodecCtx, audio_buf, audio_buf_size);
			if (audio_size < 0) {//检查解码操作是否成功
				// If error, output silence.
				audio_buf_size = 1024; // arbitrary?
				memset(audio_buf, 0, audio_buf_size);//全零重置缓冲区
			} else {
				audio_buf_size = audio_size;//返回packet中包含的原始音频数据长度(多帧)
			}
			audio_buf_index = 0;//初始化累计写入缓存长度
		}//end for if

		wt_stream_len = audio_buf_size-audio_buf_index;//计算解码缓存剩余长度
		if (wt_stream_len > len) {//检查每次写入缓存的数据长度是否超过指定长度(1024)
			wt_stream_len = len;//指定长度从解码的缓存中取数据
		}
		//每次从解码的缓存数据中以指定长度抽取数据并写入stream传递给声卡
		memcpy(stream,(uint8_t*)audio_buf+audio_buf_index,wt_stream_len);
		len -= wt_stream_len;//更新解码音频缓存的剩余长度
		stream += wt_stream_len;//更新缓存写入位置
		audio_buf_index += wt_stream_len;//更新累计写入缓存数据长度
	}//end for while
}

int main(int argc, char *argv[]) {
/*--------------参数定义-------------*/
	AVFormatContext *pFormatCtx = NULL;//保存文件容器封装信息及码流参数的结构体
	AVCodecContext *vCodecCtx = NULL;//视频解码器上下文对象，解码器依赖的相关环境、状态、资源以及参数集的接口指针
	AVCodecContext *aCodecCtx = NULL;//音频解码器上下文对象，解码器依赖的相关环境、状态、资源以及参数集的接口指针
	AVCodec *vCodec = NULL;//保存视频编解码器信息的结构体，提供编码与解码的公共接口，可以看作是编码器与解码器的一个全局变量
	AVCodec *aCodec = NULL;//保存音频编解码器信息的结构体，提供编码与解码的公共接口，可以看作是编码器与解码器的一个全局变量
	AVPacket packet;//负责保存压缩编码数据相关信息的结构体,每帧图像由一到多个packet包组成
	AVFrame *pFrame = NULL;//保存音视频解码后的数据，如状态信息、编解码器信息、宏块类型表，QP表，运动矢量表等数据
	struct SwsContext *sws_ctx = NULL;//描述转换器参数的结构体
	AVDictionary *videoOptionsDict = NULL;
	AVDictionary *audioOptionsDict = NULL;

	SDL_Surface *screen = NULL;//SDL绘图表面，A structure that contains a collection of pixels used in software blitting
	SDL_Overlay *bmp = NULL;//SDL画布
	SDL_Rect rect;//SDL矩形对象
	SDL_AudioSpec wanted_spec, spec;//SDL_AudioSpec a structure that contains the audio output format，创建 SDL_AudioSpec 结构体，设置音频播放数据
	SDL_Event event;//SDL事件对象

	int i, videoStream, audioStream;//循环变量，音视频流类型标号
	int frameFinished;//解码操作是否成功标识

/*-------------参数初始化------------*/	
	if (argc<2) {//检查输入参数个数是否正确
		fprintf(stderr, "Usage: test <file>\n");
		exit(1);
	}
	// Register all formats and codecs，注册所有多媒体格式及编解码器
	av_register_all();
	
	// Open video file，打开视频文件，取得文件容器的封装信息及码流参数
	if (avformat_open_input(&pFormatCtx, argv[1], NULL, NULL) != 0) {
		return -1; // Couldn't open file.
	}
	
	// Retrieve stream information，取得文件中保存的码流信息
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		return -1; // Couldn't find stream information.
	}
	
	// Dump information about file onto standard error，打印pFormatCtx中的码流信息
	av_dump_format(pFormatCtx, 0, argv[1], 0);
	
	// Find the first video stream.
	videoStream = -1;//视频流类型标号初始化为-1
	audioStream = -1;//音频流类型标号初始化为-1
	for (i = 0; i < pFormatCtx->nb_streams; i++) {//遍历文件中包含的所有流媒体类型(视频流、音频流、字幕流等)
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && videoStream < 0) {//若文件中包含有视频流
			videoStream = i;//用视频流类型的标号修改标识，使之不为-1
		}
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && audioStream < 0) {//若文件中包含有音频流
			audioStream = i;//用音频流类型的标号修改标识，使之不为-1
		}
	}
	if (videoStream == -1) {//检查文件中是否存在视频流
		return -1; // Didn't find a video stream.
	}
	if (audioStream == -1) {//检查文件中是否存在音频流
		return -1;
	}
	
	// Get a pointer to the codec context for the video stream，根据流类型标号从pFormatCtx->streams中取得视频流对应的解码器上下文
	vCodecCtx = pFormatCtx->streams[videoStream]->codec;
	/*-----------------------
	 * Find the decoder for the video stream，根据视频流对应的解码器上下文查找对应的解码器，返回对应的解码器(信息结构体)
	 * The stream's information about the codec is in what we call the "codec context.
	 * This contains all the information about the codec that the stream is using
	 -----------------------*/
	vCodec = avcodec_find_decoder(vCodecCtx->codec_id);
	if (vCodec == NULL) {//检查解码器是否匹配
		fprintf(stderr, "Unsupported codec!\n");
		return -1; // Codec not found.
	}
	if (avcodec_open2(vCodecCtx, vCodec, &videoOptionsDict) < 0)// Open codec，打开视频解码器
		return -1; // Could not open codec.

	// Get a pointer to the codec context for the video stream，根据流类型标号从pFormatCtx->streams中取得音频流对应的解码器上下文
	aCodecCtx = pFormatCtx->streams[audioStream]->codec;
	// Find the decoder for the video stream，根据视频流对应的解码器上下文查找对应的解码器，返回对应的解码器(信息结构体)
	aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
	if (!aCodec) {//检查解码器是否匹配
		fprintf(stderr, "Unsupported codec!\n");
		return -1;
	}
	avcodec_open2(aCodecCtx, aCodec, &audioOptionsDict);// Open codec，打开音频解码器

	// Allocate video frame，为解码后的视频信息结构体分配空间并完成初始化操作(结构体中的图像缓存按照下面两步手动安装)
	pFrame = av_frame_alloc();
	// Initialize SWS context for software scaling，设置图像转换像素格式为AV_PIX_FMT_YUV420P
	sws_ctx = sws_getContext(vCodecCtx->width, vCodecCtx->height, vCodecCtx->pix_fmt, vCodecCtx->width, vCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);

	packet_queue_init(&audioq);//缓存队列初始化

	//SDL_Init initialize the Event Handling, File I/O, and Threading subsystems，初始化SDL 
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {//initialize the video audio & timer subsystem 
		fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
		exit(1);
	}
	// Make a screen to put our video,在SDL2.0中SDL_SetVideoMode及SDL_Overlay已经弃用，改为SDL_CreateWindow及SDL_CreateRenderer创建窗口及着色器
#ifndef __DARWIN__
	screen = SDL_SetVideoMode(vCodecCtx->width, vCodecCtx->height, 0, 0);//创建SDL窗口及绘图表面，并指定图像尺寸及像素个数
#else
	screen = SDL_SetVideoMode(vCodecCtx->width, vCodecCtx->height, 24, 0);//创建SDL窗口及绘图表面，并指定图像尺寸及像素个数
#endif
	if (!screen) {//检查SDL(绘图表面)窗口是否创建成功(SDL用绘图表面对象操作窗口)
		fprintf(stderr, "SDL: could not set video mode - exiting\n");
		exit(1);
	}
	SDL_WM_SetCaption(argv[1],0);//用输入文件名设置SDL窗口标题

	// Allocate a place to put our YUV image on that screen，创建画布对象
	bmp = SDL_CreateYUVOverlay(vCodecCtx->width, vCodecCtx->height, SDL_YV12_OVERLAY, screen);

	// Set audio settings from codec info,SDL_AudioSpec a structure that contains the audio output format
	// 创建SDL_AudioSpec结构体，设置音频播放参数
	wanted_spec.freq = aCodecCtx->sample_rate;//采样频率 DSP frequency -- samples per second
	wanted_spec.format = AUDIO_S16SYS;//采样格式 Audio data format
	wanted_spec.channels = aCodecCtx->channels;//声道数 Number of channels: 1 mono, 2 stereo
	wanted_spec.silence = 0;//无输出时是否静音
	wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;//默认每次读音频缓存的大小，推荐值为 512~8192，ffplay使用的是1024 specifies a unit of audio data refers to the size of the audio buffer in sample frames
	wanted_spec.callback = audio_callback;//设置取音频数据的回调接口函数 the function to call when the audio device needs more data
	wanted_spec.userdata = aCodecCtx;//传递用户数据
	
   /*---------------------------
    * 以指定参数打开音频设备，并返回与指定参数最为接近的参数，该参数为设备实际支持的音频参数
	* Opens the audio device with the desired parameters(wanted_spec)
	* return another specs we actually be using 
	* and not guaranteed to get what we asked for
 	--------------------------*/ 
	if (SDL_OpenAudio(&wanted_spec, &spec)<0) {
		fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
		return -1;
	}
	SDL_PauseAudio(0);//audio callback starts running again，开启音频设备，如果这时候没有获得数据那么它就静音
/*--------------循环解码-------------*/	
	i = 0;// Read frames and save first five frames to disk.
	/*-----------------------
	 * read in a packet and store it in the AVPacket struct
	 * ffmpeg allocates the internal data for us,which is pointed to by packet.data
	 * this is freed by the av_free_packet()
	 -----------------------*/
	while (av_read_frame(pFormatCtx, &packet) >= 0) {//从文件中依次读取每个图像编码数据包，并存储在AVPacket数据结构中
		// Is this a packet from the video stream，检查数据包类型
		if (packet.stream_index == videoStream) {//检查视频媒体流类型标识
		   /*-----------------------
	 		* Decode video frame，解码完整的一帧数据，并将frameFinished设置为true
			* 可能无法通过只解码一个packet就获得一个完整的视频帧frame，可能需要读取多个packet才行
	 		* avcodec_decode_video2()会在解码到完整的一帧时设置frameFinished为真
			* Technically a packet can contain partial frames or other bits of data
			* ffmpeg's parser ensures that the packets we get contain either complete or multiple frames
			* convert the packet to a frame for us and set frameFinisned for us when we have the next frame
	 	 	-----------------------*/
			avcodec_decode_video2(vCodecCtx, pFrame, &frameFinished, &packet);
			
			// Did we get a video frame，检查是否解码出完整一帧图像
			if (frameFinished) {
				SDL_LockYUVOverlay(bmp);//locks the overlay for direct access to pixel data，原子操作，保护像素缓冲区，避免非法修改
				
				AVFrame pict;//保存转换为AV_PIX_FMT_YUV420P格式的视频帧
				pict.data[0] = bmp->pixels[0];//将转码后的图像与画布的像素缓冲器关联
				pict.data[1] = bmp->pixels[2];
				pict.data[2] = bmp->pixels[1];
				
				pict.linesize[0] = bmp->pitches[0];//将转码后的图像扫描行长度与画布像素缓冲区的扫描行长度相关联
				pict.linesize[1] = bmp->pitches[2];//linesize-Size, in bytes, of the data for each picture/channel plane
				pict.linesize[2] = bmp->pitches[1];;//For audio, only linesize[0] may be set
				
				// Convert the image into YUV format that SDL uses，将解码后的图像转换为AV_PIX_FMT_YUV420P格式，并赋值到pict对象
				sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data, pFrame->linesize, 0, vCodecCtx->height, pict.data, pict.linesize);
				
				SDL_UnlockYUVOverlay(bmp);//Unlocks a previously locked overlay. An overlay must be unlocked before it can be displayed
				//设置矩形显示区域
				rect.x = 0;
				rect.y = 0;
				rect.w = vCodecCtx->width;
				rect.h = vCodecCtx->height;
				SDL_DisplayYUVOverlay(bmp, &rect);//图像渲染
				av_packet_unref(&packet);//Free the packet that was allocated by av_read_frame，释放AVPacket数据结构中编码数据指针
			}
		} else if (packet.stream_index == audioStream) {//检查音频媒体流类型标识
			packet_queue_put(&audioq, &packet);//向缓存队列中填充编码数据包
		} else {//字幕流类型标识
			//Free the packet that was allocated by av_read_frame，释放AVPacket数据结构中编码数据指针
			av_packet_unref(&packet);
		}

	   /*-------------------------
		* 在每次循环中从SDL后台队列取事件并填充到SDL_Event对象中
		* SDL的事件系统使得你可以接收用户的输入，从而完成一些控制操作
		* SDL_PollEvent() is the favored way of receiving system events 
		* since it can be done from the main loop and does not suspend the main loop
		* while waiting on an event to be posted
		* poll for events right after we finish processing a packet
		------------------------*/
		SDL_PollEvent(&event);
		switch (event.type) {//检查SDL事件对象
			case SDL_QUIT://退出事件
				quit = 1;//退出进程标识置1
				SDL_Quit();//退出操作
				exit(0);//结束进程
				break;
			default:
				break;
		}//end for switch
	}//end for while
/*--------------参数撤销-------------*/ 	
	// Free the YUV frame.
	av_free(pFrame);
	
	// Close the codec.
	avcodec_close(vCodecCtx);
	
	// Close the video file.
	avformat_close_input(&pFormatCtx);
	
	return 0;
}
