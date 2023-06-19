#include "renderer.h"
#include <iostream>

SdlRenderer::SdlRenderer(AVCodecContext* p_codec_ctx,std::shared_ptr<Frame> _frame,int frame_rate):frame(_frame)
{
    // B1. 初始化SDL子系统：缺省(事件处理、文件IO、线程)、视频、音频、定时器
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER))
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
                    SDL_WINDOW_OPENGL
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
    sdl_texture = SDL_CreateTexture(sdl_renderer, 
                    SDL_PIXELFORMAT_IYUV, 
                    SDL_TEXTUREACCESS_STREAMING,
                    p_codec_ctx->width,
                    p_codec_ctx->height
                    );
    if (sdl_texture == NULL)
    {  
        SDL_DestroyWindow(screen);
        SDL_DestroyRenderer(sdl_renderer);
        throw std::runtime_error("SDL_CreateTexture() failed: ");  
    }

    sdl_rect.x = 0;
    sdl_rect.y = 0;
    sdl_rect.w = p_codec_ctx->width;
    sdl_rect.h = p_codec_ctx->height;


    static int _frame_rate=frame_rate;
    
    // B5. 创建定时刷新事件线程，按照预设帧率产生刷新事件
    sdl_thread = SDL_CreateThread(sdl_thread_handle_refreshing, NULL, (void *)&_frame_rate);
    if (sdl_thread == NULL)
    {  
        SDL_DestroyTexture(sdl_texture);
        SDL_DestroyRenderer(sdl_renderer);
        SDL_DestroyWindow(screen);
        throw std::runtime_error("SDL_CreateThread() failed: ");  
    }
    
}

SdlRenderer::~SdlRenderer()
{
    SDL_DestroyTexture(sdl_texture);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(screen);
}




// 按照opaque传入的播放帧率参数，按固定间隔时间发送刷新事件
int SdlRenderer::sdl_thread_handle_refreshing(void *opaque)
{
    SDL_Event sdl_event;

    int frame_rate = *((int *)opaque);
    int interval = (frame_rate > 0) ? 1000/frame_rate : 40;

    printf("frame rate %d FPS, refresh interval %d ms\n", frame_rate, interval);

    while (!s_playing_exit)
    {
        if (!s_playing_pause)
        {
            sdl_event.type = SDL_USEREVENT_REFRESH;
            SDL_PushEvent(&sdl_event);
        }
        SDL_Delay(interval);
    }

    return 0;
}

void SdlRenderer::renderFrame(){
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