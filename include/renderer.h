#ifndef RENDERER_H
#define RENDERER_H


#include "frame.h"


#define SDL_USEREVENT_REFRESH  (SDL_USEREVENT + 1)
#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

class SdlRenderer{
    public:
        SdlRenderer(){}
        ~SdlRenderer(){}
    
        //std::shared_ptr<Frame> frame;
};


class videoSdlRenderer :public SdlRenderer{
public:
    videoSdlRenderer(AVCodecContext* p_codec_ctx,std::shared_ptr<videoFrame> frame);
    ~videoSdlRenderer();

    void renderFrame();

    static int sdl_thread_handle_refreshing(void *opaque);

    static std::shared_ptr<videoFrame> frame;

    SDL_Window*         screen; 
    int width;
    int height;
private:    
    SDL_Renderer*       sdl_renderer;
    SDL_Texture*        sdl_texture;
    SDL_Rect            sdl_rect;
    SDL_Thread*         sdl_thread;
    
};




// class audioSdlRenderer:public SdlRenderer{
//     public:
//         audioSdlRenderer(AVCodecContext* p_codec_ctx,int frame_rate);
//         ~audioSdlRenderer();

//         int renderFrame(AVCodecContext *p_codec_ctx);

//         //static void sdl_audio_callback(void *userdata, uint8_t *stream, int len);
//         std::shared_ptr<audioFrame> frame;
    
//         SDL_AudioSpec       wanted_spec;
        
//         FF_AudioParams s_audio_param_tgt;

//         SwrContext *swr_ctx;
        

//         uint8_t *audio_buf;
//         int buf_size;
// };


#endif