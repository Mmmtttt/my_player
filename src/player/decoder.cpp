#include "decoder.h"
#include "frame.h"
#include "audio.h"
#include <iostream>
#include <libswscale/swscale.h>
// std::shared_ptr<audioDecoder> static_a_decoder;


int videoDecoder::duration=0;

Decoder::Decoder(AVCodecParameters *_p_codec_par,int _idx,double _timebase_in_ms):idx(_idx),timebase_in_ms(_timebase_in_ms)
{
    // A5.1 获取解码器参数AVCodecParameters
    

    // A5.2 获取解码器
    const AVCodec* p_codec = avcodec_find_decoder(_p_codec_par->codec_id);
    if (p_codec == NULL)
    {
        throw std::runtime_error("Cann't find codec!");
    }


    // A5.3 构建解码器AVCodecContext
    // A5.3.1 p_codec_ctx初始化：分配结构体，使用p_codec初始化相应成员为默认值
    p_codec_ctx = avcodec_alloc_context3(p_codec);
    if (p_codec_ctx == NULL)
    {
        throw std::runtime_error("avcodec_alloc_context3() failed ");
    }
    // A5.3.2 p_codec_ctx初始化：p_codec_par ==> p_codec_ctx，初始化相应成员
    int ret = avcodec_parameters_to_context(p_codec_ctx, _p_codec_par);
    if (ret < 0)
    {
        avcodec_free_context(&p_codec_ctx);
        throw std::runtime_error("avcodec_parameters_to_context() failed ");
    }
    // A5.3.3 p_codec_ctx初始化：使用p_codec初始化p_codec_ctx，初始化完成
    ret = avcodec_open2(p_codec_ctx, p_codec, NULL);
    if (ret < 0)
    {
        avcodec_free_context(&p_codec_ctx);
        throw std::runtime_error("avcodec_open2() failed ");
    }
}

Decoder::~Decoder()
{
    if(p_codec_ctx)avcodec_free_context(&p_codec_ctx);
    std::cout<<"~Decoder"<<std::endl;
    //sws_freeContext(sws_ctx);
    //av_packet_unref(p_packet);
}


void videoDecoder::get_Packet(){
    std::shared_ptr<myAVPacket> temp;
    while(1){
        video_packet_queue->packet_queue_pop(temp,1);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        time_shaft+=(elapsed.count()-std::max(a_last_time,v_last_time))*speed;//时间轴，是一次
        v_last_time=elapsed.count();


        s_video_play_time=temp->mypkt.dts * timebase_in_ms;
        duration=temp->mypkt.duration * timebase_in_ms;

        int64_t diff=time_shaft-s_video_play_time;
        if(Video_Delay_in_Range(diff))break;   
        else if (Video_Delay_Behind(diff))
        {
            continue;
        }
        else if(Video_Delay_Advanced(diff))
        {
            video_packet_queue->curr_decode_pos-=2;
            continue;
        }
        else if(Video_Should_Seek(diff)){
            video_packet_queue->seek(time_shaft,timebase_in_ms);
            avcodec_flush_buffers(p_codec_ctx);
        }
    }
    

    int ret = avcodec_send_packet(p_codec_ctx, &temp->mypkt);
    if (ret != 0)
    {
        throw std::runtime_error("avcodec_send_packet() failed ");
    }
}










videoDecoder::videoDecoder(AVCodecParameters *_p_codec_par,int _idx,double _timebase_in_ms):Decoder(_p_codec_par,_idx,_timebase_in_ms){
    

    try{frame=std::make_shared<videoFrame>(p_codec_ctx);}
    catch(const std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
        avcodec_free_context(&p_codec_ctx);
        throw std::runtime_error("create frame failed\n");
    }


    try{renderer=std::make_unique<videoSdlRenderer>(p_codec_ctx,frame);}
    catch(const std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
        avcodec_free_context(&p_codec_ctx);
        throw std::runtime_error("create renderer failed\n");
    }


    // A7. 初始化SWS context，用于后续图像转换
    //     此处第6个参数使用的是FFmpeg中的像素格式，对比参考注释B4
    //     FFmpeg中的像素格式AV_PIX_FMT_YUV420P对应SDL中的像素格式SDL_PIXELFORMAT_IYUV
    //     如果解码后得到图像的不被SDL支持，不进行图像转换的话，SDL是无法正常显示图像的
    //     如果解码后得到图像的能被SDL支持，则不必进行图像转换
    //     这里为了编码简便，统一转换为SDL支持的格式AV_PIX_FMT_YUV420P==>SDL_PIXELFORMAT_IYUV
    sws_ctx = sws_getContext(p_codec_ctx->width,    // src width
                p_codec_ctx->height,   // src height
                p_codec_ctx->pix_fmt,  // src format
                p_codec_ctx->width,    // dst width
                p_codec_ctx->height,   // dst height
                AV_PIX_FMT_YUV420P,    // dst format
                SWS_BICUBIC,           // flags
                NULL,                  // src filter
                NULL,                  // dst filter
                NULL                   // param
                );
    if (sws_ctx == NULL)
    {
        avcodec_free_context(&p_codec_ctx);
        throw std::runtime_error("sws_getContext() failed ");
    }
}

videoDecoder::~videoDecoder(){
    sws_freeContext(sws_ctx);
    std::cout<<"videoDecoder destoryed"<<std::endl;
}


void videoDecoder::sws_scaling(){
    // A10. 图像转换：p_frm_raw->data ==> p_frm_yuv->data
    // 将源图像中一片连续的区域经过处理后更新到目标图像对应区域，处理的图像区域必须逐行连续
    // plane: 如YUV有Y、U、V三个plane，RGB有R、G、B三个plane
    // slice: 图像中一片连续的行，必须是连续的，顺序由顶部到底部或由底部到顶部
    // stride/pitch: 一行图像所占的字节数，Stride=BytesPerPixel*Width+Padding，注意对齐
    // AVFrame.*data[]: 每个数组元素指向对应plane
    // AVFrame.linesize[]: 每个数组元素表示对应plane中一行图像所占的字节数
    sws_scale(sws_ctx,                                  // sws context
        (const uint8_t *const *)frame->p_frm_raw->data,  // src slice
        frame->p_frm_raw->linesize,                      // src stride
        0,                                        // src slice y
        p_codec_ctx->height,                      // src slice height
        frame->p_frm_yuv->data,                          // dst planes
        frame->p_frm_yuv->linesize                       // dst strides
        );
}

int videoDecoder::decode_One_frame(){
    int ret =  avcodec_receive_frame(p_codec_ctx, frame->p_frm_raw);
    if (ret != 0)
    {
        if (ret == AVERROR_EOF)
        {
            throw std::runtime_error("avcodec_receive_frame(): the decoder has been fully flushed");
        }
        else if (ret == AVERROR(EAGAIN))
        {
            std::cout<<"avcodec_receive_frame(): output is not available in this state - "
                    "user must try to send new input"<<std::endl;
            return -1;
        }
        else if (ret == AVERROR(EINVAL))
        {
            throw std::runtime_error("avcodec_receive_frame(): codec not opened, or it is an encoder");
        }
        else
        {
            throw std::runtime_error("avcodec_receive_frame(): legitimate decoding errors");
        }
    }
    return ret;
}

int videoDecoder::present_One_frame(){
    try{get_Packet();}
    catch(const std::exception&e){
        throw std::runtime_error(e.what());
    }
    
    int ret ;
    try{ret=decode_One_frame();} 
    catch(const std::exception&e){
        throw std::runtime_error(e.what());
    }
    if(ret==-1)return -1;
    
    sws_scaling();
    renderer->renderFrame();
    //av_packet_unref(p_packet);
    return 1;
}


std::shared_ptr<audioDecoder> static_a_decoder;

audioDecoder::audioDecoder(AVCodecParameters *_p_codec_par,int _idx,double _timebase_in_ms):Decoder(_p_codec_par,_idx,_timebase_in_ms){
    
    
    
    // A5.1 获取解码器参数AVCodecParameters
    // AVCodecParameters *p_codec_par = _p_fmt_ctx->streams[_idx]->codecpar;

    // A5.2 获取解码器
    const AVCodec* p_codec = avcodec_find_decoder(_p_codec_par->codec_id);
    if (p_codec == NULL)
    {
        throw std::runtime_error("Cann't find codec!");
    }


    // A5.3 构建解码器AVCodecContext
    // A5.3.1 p_codec_ctx初始化：分配结构体，使用p_codec初始化相应成员为默认值
    p_codec_ctx = avcodec_alloc_context3(p_codec);
    if (p_codec_ctx == NULL)
    {
        throw std::runtime_error("avcodec_alloc_context3() failed ");
    }
    // A5.3.2 p_codec_ctx初始化：p_codec_par ==> p_codec_ctx，初始化相应成员
    int ret = avcodec_parameters_to_context(p_codec_ctx, _p_codec_par);
    if (ret < 0)
    {
        avcodec_free_context(&p_codec_ctx);
        throw std::runtime_error("avcodec_parameters_to_context() failed ");
    }
    // A5.3.3 p_codec_ctx初始化：使用p_codec初始化p_codec_ctx，初始化完成
    ret = avcodec_open2(p_codec_ctx, p_codec, NULL);
    if (ret < 0)
    {
        avcodec_free_context(&p_codec_ctx);
        throw std::runtime_error("avcodec_open2() failed ");
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO))
    {  
        throw std::runtime_error("SDL_Init() failed"); 
    }


    // B2. 打开音频设备并创建音频处理线程
    // B2.1 打开音频设备，获取SDL设备支持的音频参数actual_spec(期望的参数是wanted_spec，实际得到actual_spec)
    // 1) SDL提供两种使音频设备取得音频数据方法：
    //    a. push，SDL以特定的频率调用回调函数，在回调函数中取得音频数据
    //    b. pull，用户程序以特定的频率调用SDL_QueueAudio()，向音频设备提供数据。此种情况wanted_spec.callback=NULL
    // 2) 音频设备打开后播放静音，不启动回调，调用SDL_PauseAudio(0)后启动回调，开始正常播放音频
    wanted_spec.freq = p_codec_ctx->sample_rate;    // 采样率
    wanted_spec.format = AUDIO_S16SYS;              // S表带符号，16是采样深度，SYS表采用系统字节序
    wanted_spec.channels = p_codec_ctx->channels;   // 声道数
    wanted_spec.silence = 0;                        // 静音值
    wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;    // SDL声音缓冲区尺寸，单位是单声道采样点尺寸x通道数
    wanted_spec.callback = static_a_decoder->sdl_audio_callback;      // 回调函数，若为NULL，则应使用SDL_QueueAudio()机制
    wanted_spec.userdata = NULL;             // 提供给回调函数的参数
    if (SDL_OpenAudio(&wanted_spec, NULL) < 0)
    {
        printf("SDL_OpenAudio() failed: %s\n", SDL_GetError());
    }

    // B2.2 根据SDL音频参数构建音频重采样参数
    // wanted_spec是期望的参数，actual_spec是实际的参数，wanted_spec和auctual_spec都是SDL中的参数。
    // 此处audio_param是FFmpeg中的参数，此参数应保证是SDL播放支持的参数，后面重采样要用到此参数
    // 音频帧解码后得到的frame中的音频格式未必被SDL支持，比如frame可能是planar格式，但SDL2.0并不支持planar格式，
    // 若将解码后的frame直接送入SDL音频缓冲区，声音将无法正常播放。所以需要先将frame重采样(转换格式)为SDL支持的模式，
    // 然后送再写入SDL音频缓冲区
    s_audio_param_tgt.fmt = AV_SAMPLE_FMT_S16;
    s_audio_param_tgt.freq = wanted_spec.freq;
    s_audio_param_tgt.channel_layout = av_get_default_channel_layout(wanted_spec.channels);;
    s_audio_param_tgt.channels =  wanted_spec.channels;
    s_audio_param_tgt.frame_size = av_samples_get_buffer_size(NULL, wanted_spec.channels, 1, s_audio_param_tgt.fmt, 1);
    s_audio_param_tgt.bytes_per_sec = av_samples_get_buffer_size(NULL, wanted_spec.channels, wanted_spec.freq, s_audio_param_tgt.fmt, 1);
    if (s_audio_param_tgt.bytes_per_sec <= 0 || s_audio_param_tgt.frame_size <= 0)
    {
        printf("av_samples_get_buffer_size failed\n");
    }
    s_audio_param_src = s_audio_param_tgt;
    


}

audioDecoder::~audioDecoder(){
    SDL_CloseAudio();
    std::cout<<"aideoDecoder destoryed"<<std::endl;
}
