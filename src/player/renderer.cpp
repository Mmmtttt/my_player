#include "renderer.h"
#include "decoder.h"
#include <iostream>

std::shared_ptr<videoFrame> videoSdlRenderer::frame=NULL;

videoSdlRenderer::videoSdlRenderer(AVCodecContext* p_codec_ctx,std::shared_ptr<videoFrame> _frame)
{
    frame=_frame;
    // B1. 初始化SDL子系统：缺省(事件处理、文件IO、线程)、视频、音频、定时器
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO))
    {  
        throw std::runtime_error("SDL_Init() failed"); 
    }

    // B2. 创建SDL窗口，SDL 2.0支持多窗口
    //     SDL_Window即运行程序后弹出的视频窗口，同SDL 1.x中的SDL_Surface
    screen = SDL_CreateWindow("simple ffplayer", 
                    SDL_WINDOWPOS_UNDEFINED,// 不关心窗口X坐标
                    SDL_WINDOWPOS_UNDEFINED,// 不关心窗口Y坐标
                    p_codec_ctx->width, 
                    p_codec_ctx->height,
                    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
                    );

    if (screen == NULL)
    {  
        throw std::runtime_error("SDL_CreateWindow() failed: ");  
    }

    // B3. 创建SDL_Renderer
    //     SDL_Renderer：渲染器
    sdl_renderer = SDL_CreateRenderer(screen, -1, 0);
    if (sdl_renderer == NULL)
    {  
        SDL_DestroyWindow(screen);
        throw std::runtime_error("SDL_CreateRenderer() failed: ");  
    }

    // B4. 创建SDL_Texture
    //     一个SDL_Texture对应一帧YUV数据，同SDL 1.x中的SDL_Overlay
    //     此处第2个参数使用的是SDL中的像素格式，对比参考注释A7
    //     FFmpeg中的像素格式AV_PIX_FMT_YUV420P对应SDL中的像素格式SDL_PIXELFORMAT_IYUV
    // width=p_codec_ctx->width;
    // height=p_codec_ctx->height;
    width=&frame->width;
    height=&frame->height;

    init_SDL_Texture();

    
    // B5. 创建定时刷新事件线程，按照预设帧率产生刷新事件
    sdl_thread = SDL_CreateThread(sdl_thread_handle_refreshing, NULL, (void *)&videoDecoder::duration);
    if (sdl_thread == NULL)
    {  
        SDL_DestroyTexture(sdl_texture);
        SDL_DestroyRenderer(sdl_renderer);
        SDL_DestroyWindow(screen);
        throw std::runtime_error("SDL_CreateThread() failed: ");  
    }
    
}

videoSdlRenderer::~videoSdlRenderer()
{
    SDL_DestroyTexture(sdl_texture);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(screen);
    std::cout<<"videoSdlRenderer destoryed"<<std::endl;
}

void videoSdlRenderer::init_SDL_Texture(){
    frame->init_Buffer();
    sdl_texture = SDL_CreateTexture(sdl_renderer, 
                    SDL_PIXELFORMAT_IYUV, 
                    SDL_TEXTUREACCESS_STREAMING,
                    *width,
                    *height
                    );
    if (sdl_texture == NULL)
    {  
        SDL_DestroyWindow(screen);
        SDL_DestroyRenderer(sdl_renderer);
        throw std::runtime_error("SDL_CreateTexture() failed: ");  
    }

    sdl_rect.x = 0;
    sdl_rect.y = 0;
    sdl_rect.w = *width;
    sdl_rect.h = *height;
}


// 按照opaque传入的播放帧率参数，按固定间隔时间发送刷新事件
int videoSdlRenderer::sdl_thread_handle_refreshing(void *opaque)
{
    SDL_Event sdl_event;
    std::cout<<"refreshing thread created"<<std::endl;
    s_playing_exit=false;
    s_playing_pause=false;

    while (!s_playing_exit)
    {
        int duration = *((int *)opaque);
        int interval = (duration > 0) ? duration : 40;
        //std::cout<<"interval : "<<interval<<std::endl; 
        if (!s_playing_pause)
        {
            sdl_event.type = SDL_USEREVENT_REFRESH;
            SDL_PushEvent(&sdl_event);
        }
        SDL_Delay(interval/speed);
    }
    std::cout<<"refreshing thread destoryed"<<std::endl;

    return 0;
}

void videoSdlRenderer::renderFrame(){
    if(*width!=sdl_rect.w||*height!=sdl_rect.h){
        frame->init_Buffer();
        SDL_DestroyTexture(sdl_texture);
        init_SDL_Texture();
    }


    // B7. 使用新的YUV像素数据更新SDL_Rect
    SDL_UpdateYUVTexture(sdl_texture,                   // sdl texture
            &sdl_rect,                     // sdl rect
            frame->p_frm_yuv->data[0],            // y plane
            frame->p_frm_yuv->linesize[0],        // y pitch
            frame->p_frm_yuv->data[1],            // u plane
            frame->p_frm_yuv->linesize[1],        // u pitch
            frame->p_frm_yuv->data[2],            // v plane
            frame->p_frm_yuv->linesize[2]         // v pitch
            );

    // B8. 使用特定颜色清空当前渲染目标
    SDL_RenderClear(sdl_renderer);
    // B9. 使用部分图像数据(texture)更新当前渲染目标
    SDL_RenderCopy(sdl_renderer,                        // sdl renderer
            sdl_texture,                         // sdl texture
            NULL,                                // src rect, if NULL copy texture
            &sdl_rect                            // dst rect
            );

    // B10. 执行渲染，更新屏幕显示
    SDL_RenderPresent(sdl_renderer);


}




// audioSdlRenderer::audioSdlRenderer(AVCodecContext* p_codec_ctx,int frame_rate){
//     if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO))
//     {  
//         throw std::runtime_error("SDL_Init() failed"); 
//     }


//     // B2. 打开音频设备并创建音频处理线程
//     // B2.1 打开音频设备，获取SDL设备支持的音频参数actual_spec(期望的参数是wanted_spec，实际得到actual_spec)
//     // 1) SDL提供两种使音频设备取得音频数据方法：
//     //    a. push，SDL以特定的频率调用回调函数，在回调函数中取得音频数据
//     //    b. pull，用户程序以特定的频率调用SDL_QueueAudio()，向音频设备提供数据。此种情况wanted_spec.callback=NULL
//     // 2) 音频设备打开后播放静音，不启动回调，调用SDL_PauseAudio(0)后启动回调，开始正常播放音频
//     wanted_spec.freq = p_codec_ctx->sample_rate;    // 采样率
//     wanted_spec.format = AUDIO_S16SYS;              // S表带符号，16是采样深度，SYS表采用系统字节序
//     wanted_spec.channels = p_codec_ctx->channels;   // 声道数
//     wanted_spec.silence = 0;                        // 静音值
//     wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;    // SDL声音缓冲区尺寸，单位是单声道采样点尺寸x通道数
//     wanted_spec.callback = audioDecoder::sdl_audio_callback;      // 回调函数，若为NULL，则应使用SDL_QueueAudio()机制
//     wanted_spec.userdata = p_codec_ctx;             // 提供给回调函数的参数
//     if (SDL_OpenAudio(&wanted_spec, NULL) < 0)
//     {
//         throw std::runtime_error("SDL_OpenAudio() failed: ");
//     }

//     // B2.2 根据SDL音频参数构建音频重采样参数
//     // wanted_spec是期望的参数，actual_spec是实际的参数，wanted_spec和auctual_spec都是SDL中的参数。
//     // 此处audio_param是FFmpeg中的参数，此参数应保证是SDL播放支持的参数，后面重采样要用到此参数
//     // 音频帧解码后得到的frame中的音频格式未必被SDL支持，比如frame可能是planar格式，但SDL2.0并不支持planar格式，
//     // 若将解码后的frame直接送入SDL音频缓冲区，声音将无法正常播放。所以需要先将frame重采样(转换格式)为SDL支持的模式，
//     // 然后送再写入SDL音频缓冲区
//     s_audio_param_tgt.fmt = AV_SAMPLE_FMT_S16;
//     s_audio_param_tgt.freq = wanted_spec.freq;
//     s_audio_param_tgt.channel_layout = av_get_default_channel_layout(wanted_spec.channels);;
//     s_audio_param_tgt.channels =  wanted_spec.channels;
//     s_audio_param_tgt.frame_size = av_samples_get_buffer_size(NULL, wanted_spec.channels, 1, s_audio_param_tgt.fmt, 1);
//     s_audio_param_tgt.bytes_per_sec = av_samples_get_buffer_size(NULL, wanted_spec.channels, wanted_spec.freq, s_audio_param_tgt.fmt, 1);
//     if (s_audio_param_tgt.bytes_per_sec <= 0 || s_audio_param_tgt.frame_size <= 0)
//     {
//         throw std::runtime_error("av_samples_get_buffer_size failed");
//     }
    

//     try{frame=std::make_shared<audioFrame>(swr_ctx,p_codec_ctx,s_audio_param_tgt);}
//     catch(const std::exception& e)
//     {
//         std::cout<<e.what()<<std::endl;
//         avcodec_free_context(&p_codec_ctx);
//         throw std::runtime_error("create audioframe failed\n");
//     }
    
    
// }

// audioSdlRenderer::~audioSdlRenderer(){
//     swr_free(&swr_ctx);
//     std::cout<<"audioSdlRenderer destoryed"<<std::endl;
// }

// int audioSdlRenderer::renderFrame(AVCodecContext *p_codec_ctx){
    //std::cout<<"111"<<std::endl;
    // try{frame->reSample(swr_ctx,p_codec_ctx);}
    // catch(const std::exception&e){
    //         throw std::runtime_error(e.what());
    // }
    // //std::cout<<"222"<<std::endl;
    // // 将音频帧拷贝到函数输出参数audio_buf
    // buf_size=frame->cp_len;
    // audio_buf=(uint8_t *)malloc(sizeof(uint8_t)*buf_size);
    // memcpy(audio_buf, frame->p_cp_buf, frame->cp_len);
    //std::cout<<"333"<<std::endl;
    //res = cp_len;
// }


