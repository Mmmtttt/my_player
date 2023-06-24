#ifndef VIDEO_H
#define VIDEO_H

#include <string>
#include <memory>
#include "decoder.h"


class Video {
public:
    Video(const std::string& filename);
    ~Video();


    void play();
    void show_IFO(){av_dump_format(p_fmt_ctx, 0, filename.c_str(), 0);};
    

    AVFormatContext* p_fmt_ctx = NULL;



    int width;
    int height;
    int i;
    int v_idx;
    int a_idx;
    int frame_rate;



private:
    std::string filename;

    std::unique_ptr<videoDecoder> v_decoder;
    std::shared_ptr<audioDecoder> a_decoder;

    SDL_Event sdl_event;

    void read_One_frame_packet();

};

#endif

