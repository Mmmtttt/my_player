#ifndef DECODER_H
#define DECODER_H


#include "renderer.h"
#include "packetQueue.h"
#include <thread>


class Decoder {
public:
    Decoder(AVFormatContext* p_fmt_ctx,int idx,int frame_rate);
    ~Decoder();

    int present_One_frame(); 

    void get_Packets(AVFormatContext*);


    AVCodecContext* p_codec_ctx = NULL; 
    SwsContext* sws_ctx = NULL; 
    //AVPacket* p_packet = NULL;
    packetQueue packet_queue;
    int idx;
private:
    std::shared_ptr<Frame> frame;
    std::unique_ptr<SdlRenderer> renderer;

    
    void get_Frame_packet();
    int decode_packet();
    void sws_scaling();//图像缩放
};

class videoDecoder:public Decoder{
    public:

};

class audioDecoder:public Decoder{
    public:

};

#endif
