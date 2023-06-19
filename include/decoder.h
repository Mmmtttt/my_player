#ifndef DECODER_H
#define DECODER_H


#include "renderer.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

class Decoder {
public:
    Decoder(AVFormatContext* p_fmt_ctx,int v_idx,int frame_rate);
    ~Decoder();

    int present_One_frame(); 

    


    AVCodecContext* p_codec_ctx = NULL; 
    SwsContext* sws_ctx = NULL; 
    AVPacket* p_packet = NULL;
private:
    std::shared_ptr<Frame> frame;
    std::unique_ptr<SdlRenderer> renderer;

    
    void get_Frame_packet();
    int decode_packet();
    void sws_scaling();//图像缩放
};

#endif
