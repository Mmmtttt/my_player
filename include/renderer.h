#ifndef RENDERER_H
#define RENDERER_H


#include "frame.h"


#define SDL_USEREVENT_REFRESH  (SDL_USEREVENT + 1)

class SdlRenderer {
public:
    SdlRenderer(AVCodecContext* p_codec_ctx,std::shared_ptr<Frame> frame,int frame_rate);
    ~SdlRenderer();

    void renderFrame();

    static int sdl_thread_handle_refreshing(void *opaque);


private:
    std::shared_ptr<Frame> frame;
    SDL_Window*         screen; 
    SDL_Renderer*       sdl_renderer;
    SDL_Texture*        sdl_texture;
    SDL_Rect            sdl_rect;
    SDL_Thread*         sdl_thread;
    
};

#endif
