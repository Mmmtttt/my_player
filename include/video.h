#ifndef VIDEO_H
#define VIDEO_H

#include <string>
#include <memory>
#include "decoder.h"


class Video {
public:
    Video(){}
    Video(const std::string& filename);
    ~Video();

    Video(int v_idx,AVCodecParameters *v_p_codec_par,double v_timebase_in_ms,int a_idx,AVCodecParameters *a_p_codec_par,double a_timebase_in_ms);
    Video(const std::string& filename,packetQueue* video_packet_queue,packetQueue* audio_packet_queue);

    void play();
    void show_IFO(){av_dump_format(p_fmt_ctx, 0, filename.c_str(), 0);};
    

    AVFormatContext* p_fmt_ctx = NULL;
    //AVCodecContext* p_codec_ctx = NULL;
    
    AVCodecParameters *v_p_codec_par=NULL;
    AVCodecParameters *a_p_codec_par=NULL;
    double v_timebase_in_ms;
    double a_timebase_in_ms;


    // int width;
    // int height;
    // int i;
    int v_idx;
    int a_idx;
    // int frame_rate;




    std::string filename;

    std::unique_ptr<videoDecoder> v_decoder;
    std::shared_ptr<audioDecoder> a_decoder;

    packetQueue* video_packet_queue;
    packetQueue* audio_packet_queue;

    SDL_Event sdl_event;

};

#endif

