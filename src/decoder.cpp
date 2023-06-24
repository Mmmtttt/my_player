#include "decoder.h"
#include "frame.h"
#include <iostream>
#include <libswscale/swscale.h>
std::shared_ptr<audioDecoder> static_a_decoder;
packetQueue video_packet_queue;
packetQueue audio_packet_queue;

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
}

Decoder::~Decoder()
{
    avcodec_free_context(&p_codec_ctx);
    //sws_freeContext(sws_ctx);
    //av_packet_unref(p_packet);
}


void Decoder::get_Packet(){
    std::unique_ptr<myAVPacket> temp;
    if(idx==0) {video_packet_queue.packet_queue_pop(temp,1);}//std::cout<<"get video pkt "<<temp->size<<std::endl;}
    else {audio_packet_queue.packet_queue_pop(temp,1);}//std::cout<<"get audio pkt "<<temp->size<<std::endl;}
    int ret = avcodec_send_packet(p_codec_ctx, &temp->mypkt);
    if (ret != 0)
    {
        throw std::runtime_error("avcodec_send_packet() failed ");
    }
}

void Decoder::push_All_Packets(AVFormatContext*p_fmt_ctx){
    int ret=0;
    while(ret==0){
        std::unique_ptr<myAVPacket> temp(new myAVPacket);
        ret=av_read_frame(p_fmt_ctx, &temp->mypkt);
        temp->size=temp->mypkt.size;
        if(temp->mypkt.stream_index==AVMEDIA_TYPE_VIDEO)
            video_packet_queue.packet_queue_push(std::move(temp));
        else if(temp->mypkt.stream_index==AVMEDIA_TYPE_AUDIO){
            audio_packet_queue.packet_queue_push(std::move(temp));
        }
    }
}








videoDecoder::videoDecoder(AVFormatContext* p_fmt_ctx,int _idx,int frame_rate):Decoder(p_fmt_ctx,_idx,frame_rate){
    try{frame=std::make_shared<videoFrame>(p_codec_ctx);}
    catch(const std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
        avcodec_free_context(&p_codec_ctx);
        throw std::runtime_error("create frame failed\n");
    }


    try{renderer=std::make_unique<videoSdlRenderer>(p_codec_ctx,frame,frame_rate);}
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

videoDecoder::~videoDecoder(){
    sws_freeContext(sws_ctx);
    std::cout<<"videoDecoder destoryed"<<std::endl;
}


void videoDecoder::sws_scaling(){
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

int videoDecoder::decode_One_frame(){
    int ret =  avcodec_receive_frame(p_codec_ctx, frame->p_frm_raw);
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
    return ret;
}

int videoDecoder::present_One_frame(){
    try{get_Packet();}
    catch(const std::exception&e){
        throw std::runtime_error(e.what());
    }
    
    int ret ;
    try{ret=decode_One_frame();} 
    catch(const std::exception&e){
        throw std::runtime_error(e.what());
    }
    if(ret==-1)return -1;
    
    sws_scaling();
    renderer->renderFrame();
    //av_packet_unref(p_packet);
    return 1;
}




audioDecoder::audioDecoder(AVFormatContext* p_fmt_ctx,int _idx,int frame_rate):Decoder(p_fmt_ctx,_idx,frame_rate){
    
    
    

    try{renderer=std::make_unique<audioSdlRenderer>(p_codec_ctx,frame_rate);}
    catch(const std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
        avcodec_free_context(&p_codec_ctx);
        throw std::runtime_error("create renderer failed\n");
    }

    frame=renderer->frame;
    



}

audioDecoder::~audioDecoder(){

    std::cout<<"aideoDecoder destoryed"<<std::endl;
}

int audioDecoder::decode_One_frame(){
    int ret = avcodec_receive_frame(p_codec_ctx, frame->frame);
    if (ret != 0)
    {
        if (ret == AVERROR_EOF)
        {
            throw std::runtime_error("audio avcodec_receive_frame(): the decoder has been fully flushed");
        }
        else if (ret == AVERROR(EAGAIN))
        {
            //printf("audio avcodec_receive_frame(): output is not available in this state - "
            //       "user must try to send new input\n");
            return -1;
        }
        else if (ret == AVERROR(EINVAL))
        {
            throw std::runtime_error("audio avcodec_receive_frame(): codec not opened, or it is an encoder");
        }
        else
        {
            throw std::runtime_error("audio avcodec_receive_frame(): legitimate decoding errors");
        }
    }
    return ret;
}

int audioDecoder::present_One_frame(){
    // try{get_Packet();}
    // catch(const std::exception&e){
    //     throw std::runtime_error(e.what());
    // }
    int ret ;
    //while(1){
        get_Packet();
        try{ret=decode_One_frame();} 
        catch(const std::exception&e){
            throw std::runtime_error(e.what());
        }
        if(ret==-1) {get_Packet();return -1;}
        try{renderer->renderFrame(p_codec_ctx);}
        catch(const std::exception&e){
            throw std::runtime_error(e.what());
        }
        SDL_PauseAudio(0);
    //}
    //av_packet_unref(p_packet);
    return 1;
}

void audioDecoder::sdl_audio_callback(void *userdata, uint8_t *stream, int len){
    AVCodecContext *p_codec_ctx = (AVCodecContext *)userdata;
    int copy_len;           // 
    int get_size;           // 获取到解码后的音频数据大小

    static uint8_t s_audio_buf[(MAX_AUDIO_FRAME_SIZE*3)/2]; // 1.5倍声音帧的大小
    static uint32_t s_audio_len = 0;    // 新取得的音频数据大小
    static uint32_t s_tx_idx = 0;       // 已发送给设备的数据量
    int frm_size = 0;
    int ret_size = 0;
    int ret;

    //get_size=static_a_decoder->decode_One_frame();
    // if (get_size < 0)
    // {
    //     // 出错输出一段静音
    //     s_audio_len = 1024; // arbitrary?
    //     memset(s_audio_buf, 0, s_audio_len);
    //     //av_packet_unref(p_packet);
    //     printf("error silence\n");
    // }
    // else if (get_size == 0) // 解码缓冲区被冲洗，整个解码过程完毕
    // {
    //     printf("2\n");
    // }
    // else
    // {
    //     //printf("3\n");
    //     s_audio_len = get_size;
    //     //av_packet_unref(p_packet);
    // }
    // s_tx_idx = 0;
    // copy_len = s_audio_len - s_tx_idx;
    //     if (copy_len > len)
    //     {
    //         copy_len = len;
    //     }
    //     memcpy(stream, (uint8_t *)s_audio_buf + s_tx_idx, copy_len);
    //     len -= copy_len;
    //     stream += copy_len;
    //     s_tx_idx += copy_len;
    if(static_a_decoder->renderer->audio_buf==NULL)throw std::runtime_error("audio_buf is null");
    memcpy(stream,static_a_decoder->renderer->audio_buf,static_a_decoder->renderer->frame->cp_len);
}