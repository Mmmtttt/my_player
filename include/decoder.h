#ifndef DECODER_H
#define DECODER_H


#include "renderer.h"
#include "packetQueue.h"
#include <thread>



class Decoder {
public:
    Decoder(AVCodecParameters *p_codec_par,int idx,double timebase_in_ms);
    ~Decoder();

     

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

        std::shared_ptr<packetQueue> video_packet_queue;

        std::unique_ptr<videoSdlRenderer> renderer;
        std::shared_ptr<videoFrame> frame;
        SwsContext* sws_ctx = NULL; 

        
        static int duration;  //当前帧的duration
};

class audioDecoder:public Decoder{
    public:
        audioDecoder(AVCodecParameters *_p_codec_par,int _idx,double timebase_in_ms);
        ~audioDecoder();

        static void sdl_audio_callback(void *userdata, uint8_t *stream, int len);
        int audio_decode_frame(std::shared_ptr<myAVPacket> p_packet_ptr, uint8_t *audio_buf, int buf_size);

        std::shared_ptr<packetQueue> audio_packet_queue;
};

extern std::shared_ptr<audioDecoder> static_a_decoder;

#endif