#ifndef DECODER_H
#define DECODER_H


#include "renderer.h"
#include "packetQueue.h"
#include <thread>

extern packetQueue video_packet_queue;
extern packetQueue audio_packet_queue;

class Decoder {
public:
    Decoder(){}
    Decoder(AVCodecParameters *p_codec_par,int idx,double timebase_in_ms);
    ~Decoder();

     

    void push_All_Packets(AVFormatContext*);

    AVCodecContext* p_codec_ctx = NULL; 

    

    int idx=-1;

    double timebase_in_ms;

    
    

    
};

class videoDecoder:public Decoder{
    public:
        videoDecoder(AVCodecParameters *_p_codec_par,int _idx,double timebase_in_ms);
        ~videoDecoder();

        int present_One_frame();
        void sws_scaling();//图像缩放
        int decode_One_frame();

        void get_Packet();

        std::unique_ptr<videoSdlRenderer> renderer;
        std::shared_ptr<videoFrame> frame;
        SwsContext* sws_ctx = NULL; 

        
        static int duration;  //当前帧的duration
};

class audioDecoder:public Decoder{
    public:
        audioDecoder(AVCodecParameters *_p_codec_par,int _idx,double timebase_in_ms);
        ~audioDecoder();

        // int decode_One_frame();
        // int present_One_frame();
        // static void sdl_audio_callback(void *userdata, uint8_t *stream, int len);

        // std::unique_ptr<audioSdlRenderer> renderer;
        // std::shared_ptr<audioFrame> frame;
        //SwrContext *swr_ctx;
        //double timebase_in_ms;
};

extern std::shared_ptr<audioDecoder> static_a_decoder;

#endif
