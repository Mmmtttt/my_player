#include "decoder.h"
#include "frame.h"
#include <iostream>
#include <libswscale/swscale.h>

Decoder::Decoder(AVFormatContext* p_fmt_ctx,int _idx,int frame_rate):idx(_idx)
{
    // A5.1 获取解码器参数AVCodecParameters
    AVCodecParameters *p_codec_par = p_fmt_ctx->streams[_idx]->codecpar;

    // A5.2 获取解码器
    const AVCodec* p_codec = avcodec_find_decoder(p_codec_par->codec_id);
    if (p_codec == NULL)
    {
        throw std::runtime_error("Cann't find codec!");
    }


    // A5.3 构建解码器AVCodecContext
    // A5.3.1 p_codec_ctx初始化：分配结构体，使用p_codec初始化相应成员为默认值
    p_codec_ctx = avcodec_alloc_context3(p_codec);
    if (p_codec_ctx == NULL)
    {
        throw std::runtime_error("avcodec_alloc_context3() failed ");
    }
    // A5.3.2 p_codec_ctx初始化：p_codec_par ==> p_codec_ctx，初始化相应成员
    int ret = avcodec_parameters_to_context(p_codec_ctx, p_codec_par);
    if (ret < 0)
    {
        avcodec_free_context(&p_codec_ctx);
        throw std::runtime_error("avcodec_parameters_to_context() failed ");
    }
    // A5.3.3 p_codec_ctx初始化：使用p_codec初始化p_codec_ctx，初始化完成
    ret = avcodec_open2(p_codec_ctx, p_codec, NULL);
    if (ret < 0)
    {
        avcodec_free_context(&p_codec_ctx);
        throw std::runtime_error("avcodec_open2() failed ");
    }





    try{frame=std::make_shared<Frame>(p_codec_ctx);}
    catch(const std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
        avcodec_free_context(&p_codec_ctx);
        throw std::runtime_error("create frame failed\n");
    }

    try{renderer=std::make_unique<SdlRenderer>(p_codec_ctx,frame,frame_rate);}
    catch(const std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
        avcodec_free_context(&p_codec_ctx);
        throw std::runtime_error("create renderer failed\n");
    }



    // A7. 初始化SWS context，用于后续图像转换
    //     此处第6个参数使用的是FFmpeg中的像素格式，对比参考注释B4
    //     FFmpeg中的像素格式AV_PIX_FMT_YUV420P对应SDL中的像素格式SDL_PIXELFORMAT_IYUV
    //     如果解码后得到图像的不被SDL支持，不进行图像转换的话，SDL是无法正常显示图像的
    //     如果解码后得到图像的能被SDL支持，则不必进行图像转换
    //     这里为了编码简便，统一转换为SDL支持的格式AV_PIX_FMT_YUV420P==>SDL_PIXELFORMAT_IYUV
    sws_ctx = sws_getContext(p_codec_ctx->width,    // src width
                p_codec_ctx->height,   // src height
                p_codec_ctx->pix_fmt,  // src format
                p_codec_ctx->width,    // dst width
                p_codec_ctx->height,   // dst height
                AV_PIX_FMT_YUV420P,    // dst format
                SWS_BICUBIC,           // flags
                NULL,                  // src filter
                NULL,                  // dst filter
                NULL                   // param
                );
    if (sws_ctx == NULL)
    {
        avcodec_free_context(&p_codec_ctx);
        throw std::runtime_error("sws_getContext() failed ");
    }
    
}

Decoder::~Decoder()
{
    avcodec_free_context(&p_codec_ctx);
    sws_freeContext(sws_ctx);
    //av_packet_unref(p_packet);
}



int Decoder::decode_packet(){
    return avcodec_receive_frame(p_codec_ctx, frame->p_frm_raw);
}

void Decoder::sws_scaling(){
    // A10. 图像转换：p_frm_raw->data ==> p_frm_yuv->data
    // 将源图像中一片连续的区域经过处理后更新到目标图像对应区域，处理的图像区域必须逐行连续
    // plane: 如YUV有Y、U、V三个plane，RGB有R、G、B三个plane
    // slice: 图像中一片连续的行，必须是连续的，顺序由顶部到底部或由底部到顶部
    // stride/pitch: 一行图像所占的字节数，Stride=BytesPerPixel*Width+Padding，注意对齐
    // AVFrame.*data[]: 每个数组元素指向对应plane
    // AVFrame.linesize[]: 每个数组元素表示对应plane中一行图像所占的字节数
    sws_scale(sws_ctx,                                  // sws context
        (const uint8_t *const *)frame->p_frm_raw->data,  // src slice
        frame->p_frm_raw->linesize,                      // src stride
        0,                                        // src slice y
        p_codec_ctx->height,                      // src slice height
        frame->p_frm_yuv->data,                          // dst planes
        frame->p_frm_yuv->linesize                       // dst strides
        );
}

void Decoder::get_Frame_packet(){
    std::unique_ptr<myAVPacket> temp;
    packet_queue.packet_queue_pop(temp,1);
    int ret = avcodec_send_packet(p_codec_ctx, &temp->mypkt);
    if (ret != 0)
    {
        throw std::runtime_error("avcodec_send_packet() failed ");
    }
}

int Decoder::present_One_frame(){
    try{get_Frame_packet();}
    catch(const std::exception&e){
        throw std::runtime_error(e.what());
    }
    int ret = decode_packet();
    if (ret != 0)
    {
        if (ret == AVERROR_EOF)
        {
            throw std::runtime_error("avcodec_receive_frame(): the decoder has been fully flushed");
        }
        else if (ret == AVERROR(EAGAIN))
        {
            std::cout<<"avcodec_receive_frame(): output is not available in this state - "
                    "user must try to send new input"<<std::endl;
            return -1;
        }
        else if (ret == AVERROR(EINVAL))
        {
            throw std::runtime_error("avcodec_receive_frame(): codec not opened, or it is an encoder");
        }
        else
        {
            throw std::runtime_error("avcodec_receive_frame(): legitimate decoding errors");
        }
    }
    sws_scaling();
    renderer->renderFrame();
    //av_packet_unref(p_packet);
    return 1;
}

void Decoder::get_Packets(AVFormatContext*p_fmt_ctx){
    int ret=0;
    while(ret==0){
        std::unique_ptr<myAVPacket> temp(new myAVPacket);
        ret=av_read_frame(p_fmt_ctx, &temp->mypkt);
        if(temp->mypkt.stream_index==idx)
            packet_queue.packet_queue_push(std::move(temp));
    }
}