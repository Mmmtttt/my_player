#ifndef FRAME_H
#define FRAME_H

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <SDL.h>
#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL_rect.h>
#include <libavutil/frame.h>
}

#include <iostream>
#include <memory>

static bool s_playing_exit =false;
static bool s_playing_pause =false;


class Frame {
public:
    Frame(AVCodecContext* p_codec_ctx);
    ~Frame();

    AVFrame* getAVFrame() const;


    AVFrame* p_frm_raw = NULL; 
    AVFrame* p_frm_yuv = NULL; 
private:     
    int  buf_size;
    uint8_t* buffer = NULL;
};

#endif
