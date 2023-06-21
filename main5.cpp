/*
 *
 * FFmpeg+SDL的简易播放器：音频播放器
 *
 */
#include <stdio.h>
 
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "SDL.h"
};
#define AUDIO_FILE "decoded_audio.pcm"
FILE* output_file; 
static Uint8 *audio_chunk;
static int audio_len;
static Uint8 *audio_pos;
 
#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio
 
// 音频处理回调函数。
// \param[in]  userdata用户在注册回调函数时指定的参数
// \param[out] stream 音频数据缓冲区地址，将解码后的音频数据填入此缓冲区
// \param[out] len    音频数据缓冲区大小，单位字节
// 回调函数返回后，stream指向的音频缓冲区将变为无效
// 双声道采样点的顺序为LRLRLR
void sdl_audio_callback(void *userdata, uint8_t *stream, int len)
{
	SDL_memset(stream, 0, len);
	if (audio_len == 0)
		return;
 
	len = (len > audio_len ? audio_len : len); /*  Mix  as  much  data  as  possible  */
 
	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	fwrite(stream, 1, len, output_file);
	audio_pos += len;
	audio_len -= len;
}
 
int main(int argc, char *argv[])
{
	AVFormatContext *pFormatCtx = NULL;
	AVCodecContext *pCodecCtx = NULL;
	AVCodec *pCodec = NULL;
	AVCodecParameters *pCodecPar = NULL;
	AVPacket *pPacket = NULL;
	AVFrame *pFrame = NULL;
	SwrContext *swrCtx = NULL;
 
	SDL_AudioSpec wantedSpec;
//	SDL_AudioSpec actualSpec;
 
	int streamIndex = -1;
	unsigned int i = 0;
	int ret = 0;
	int index = 0;
 
 
	//const char *inFilename = "input.mp4";
 
	//构建AVFormatContext
	//打开视频文件：读取文件头，将文件格式信息存储在pFormatCtx
	ret = avformat_open_input(&pFormatCtx, argv[1], NULL, NULL);
	if (ret != 0)
	{
		printf("avformat_open_input() failed: %d\n", ret);
		return ret;
	}
 
	//搜索流信息：读取一段视频文件数据，尝试解码，将取到的流信息填入AVFormatContext->streams
	//AVFormatContext->streams 是一个指针数组，数组大小是AVFormatContext->nb_streams
	ret = avformat_find_stream_info(pFormatCtx, NULL);
	if (ret < 0)
	{
		printf("avformat_find_stream_info() failed: %d\n", ret);
		return ret;
	}
 
	//将文件相关信息打印在标准错误设备上
	av_dump_format(pFormatCtx, 0, argv[1], 0);
 
	//查找第一个音频流
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			streamIndex = i;
			printf("Find a audio stream, index = %d\n", streamIndex);
			break;
		}
	}
 
	if (streamIndex == -1)
	{
		printf("can't find audio stream\n");
		return -1;
	}
 
	//为音频流构建解码器AVCodecContext
	//获取编码器参数AVCodecParameters
	pCodecPar = pFormatCtx->streams[streamIndex]->codecpar;
 
	//获取解码器
	pCodec = avcodec_find_decoder(pCodecPar->codec_id);
	if (!pCodec)
	{
		printf("can't fidn codec\n");
		return -1;
	}
 
	//构建解码器AVCodecContext
	//pCodecCtx初始化：分配结构体，使用pCodec初始化相应成员为默认值
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx)
	{
		printf("avcodec_alloc_context3() failed");
		return -1;
	}
 
	//pCodecCtx初始化：pCodecPar ==> pCodecCtx,初始化相应成员
	ret = avcodec_parameters_to_context(pCodecCtx, pCodecPar);
	if (ret < 0)
	{
		printf("avcodec_parameters_to_context() failed: %d\n", ret);
		return -1;
	}
 
 
	//没有此句会出现：Could not update timestamps for skipped samples
	pCodecCtx->pkt_timebase = pFormatCtx->streams[streamIndex]->time_base;
 
	//pCodecCtx初始化：使用pCodec初始化pCodecCtx，初始化完成
	ret = avcodec_open2(pCodecCtx, pCodec, NULL);
	if (ret < 0)
	{
		printf("avcodec_open2() failed: %d\n", ret);
		return ret;
	}
 
	pPacket = (AVPacket*) av_malloc(sizeof(AVPacket));
	if (!pPacket)
	{
		printf("av_malloc() failed\n");
		return -1;
	}
	//初始化packet
	av_init_packet(pPacket);
 
 
	//初始化SDL子系统：缺省（事件处理、文件IO、线程）、音频、视频、定时器
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER))
	{
		printf("SDL_Init() failed: %s\n", SDL_GetError());
		return -1;
	}
 
	//SDL提供两种使音频设备取得音频数据的方法
	//a.push:SDL以特定的频率调用回调函数，在回调函数中取得音频数据
	//b.pull:用户程序以特定的频率调用SDL_QueueAudio(),向音频设备提供数据。此种情况wanted_spec.callback=NULL
	//音频设备打开后播放静音，不启动回调，调用SDL_PauseAudio(0)后启动回调，开始正常播放音频
	wantedSpec.freq = pCodecCtx->sample_rate;		//采样率
	wantedSpec.format = AUDIO_S16SYS;				//S表示带符号，16是采样深度，SYS表示采用系统字节序
	wantedSpec.channels = pCodecCtx->channels;		//声道数
	wantedSpec.silence = 0;							//静音值
	wantedSpec.samples = pCodecCtx->frame_size;		//SDL声音缓冲区尺寸，单位是单声道采样点尺寸x通道数
	wantedSpec.callback = sdl_audio_callback;		//回调函数，若为NULL，则应使用SDL_QueueAudio()机制
	wantedSpec.userdata = pCodecCtx;				//提供给回调函数的参数
 
	printf("%d %d %d %d\n", wantedSpec.freq, wantedSpec.format, wantedSpec.channels, wantedSpec.samples);
 
	if (SDL_OpenAudio(&wantedSpec, NULL) < 0)
	{
		printf("SDL_OpenAudio() failed: %s\n", SDL_GetError());
		return -1;
	}
	output_file = fopen(AUDIO_FILE, "wb");
 
	//根据SDL音频参数构建音频重采样参数
	// 音频帧解码后得到的frame中的音频格式未必被SDL支持，比如frame可能是planar格式，但SDL2.0并不支持planar格式，
	// 若将解码后的frame直接送入SDL音频缓冲区，声音将无法正常播放。所以需要先将frame重采样(转换格式)为SDL支持的模式，
	// 然后送再写入SDL音频缓冲区
	uint64_t outChannelLayout = AV_CH_LAYOUT_STEREO;	//AV_CH_LAYOUT_STEREO;	//
	AVSampleFormat outSampleFmt = AV_SAMPLE_FMT_S16;
	int outSampleRate = 44100;
	uint64_t inChannelLayout = av_get_default_channel_layout(pCodecCtx->channels);
	AVSampleFormat inSampleFmt = pCodecCtx->sample_fmt;
	int inSampleRate = pCodecCtx->sample_rate;
 
	swrCtx = swr_alloc();
	if (!swrCtx)
	{
		printf("swr_alloc() failed\n");
		return -1;
	}
 
	swr_alloc_set_opts(swrCtx, outChannelLayout, outSampleFmt, outSampleRate, inChannelLayout, inSampleFmt,
			inSampleRate, 0, NULL);
	swr_init(swrCtx);
 
	//播放
	//暂停/继续音频回调处理。参数1表暂停，0表继续。
	//打开音频设备后默认未启动回调处理，通过调用SDL_PauseAudio(0)来启动回调处理。
	//这样就可以在打开音频设备后先为回调函数安全初始化数据，一切就绪后再启动音频回调。
	//在暂停期间，会将静音值往音频设备写。
	SDL_PauseAudio(0);
 
	pFrame = av_frame_alloc();
	if (!pFrame)
	{
		printf("av_frame_alloc() failed\n");
		return -1;
	}
 
	int outChannels = av_get_channel_layout_nb_channels(outChannelLayout);
	int outBufferSize = av_samples_get_buffer_size(NULL, outChannels, pCodecCtx->frame_size, outSampleFmt, 1);
	uint8_t *outBuffer = (uint8_t*) av_malloc(MAX_AUDIO_FRAME_SIZE * 2);
 
	printf("outChannels = %d, outBufferSize = %d\n", outChannels, outBufferSize);
 
	printf("outChannelLayout = %I64d, outSampleFmt = %d, outSampleRate = %d\n", outChannelLayout, outSampleFmt,
			outSampleRate);
	printf("inChannelLayout = %I64d, in_sample_fmt = %d, in_sample_rate = %d\n", inChannelLayout, pCodecCtx->sample_fmt,
			pCodecCtx->sample_rate);
 
	//从视频文件中读取一个packet，此处仅处理音频packet
	//对于音频来说，若是帧长固定的格式则一个packet可包含整数个frame
	//           ，若是帧长可变的格式则一个packet只包含一个frame
	while (av_read_frame(pFormatCtx, pPacket) >= 0)
	{
		if (pPacket->stream_index == streamIndex)
		{
			//接收解码器输出的数据，每次接收一个frame
			ret = avcodec_send_packet(pCodecCtx, pPacket);
			if (ret < 0)
			{
				printf("can't decode data\n");
				break;
			}
 
 
    		while (avcodec_receive_frame(pCodecCtx, pFrame) >= 0)
			{
				swr_convert(swrCtx, &outBuffer, MAX_AUDIO_FRAME_SIZE, (const uint8_t**) pFrame->data,
						pFrame->nb_samples);
 
				index++;
				//printf("index = %d, pts = %d, size = %d\n", index, (int) pPacket->pts, pPacket->size);
 
				while (audio_len > 0)
				{
					SDL_Delay(1);
				}
				//设置音频缓冲区（PCM数据)
				audio_chunk = (Uint8*) outBuffer;
				//音频缓冲区长度
				audio_len = outBufferSize;
				audio_pos = audio_chunk;
			}
 
		}
 
		av_packet_unref(pPacket);
	}
 
 
    swr_free(&swrCtx);
	SDL_CloseAudio();
	SDL_Quit();
	av_free(outBuffer);
	av_frame_free(&pFrame);
	av_free(&pPacket);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
 
 
	return 0;
}