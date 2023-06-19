#include "frame.h"

Frame::Frame(AVCodecContext* p_codec_ctx)
{
    // A6. 分配AVFrame
    // A6.1 分配AVFrame结构，注意并不分配data buffer(即AVFrame.*data[])
    p_frm_raw = av_frame_alloc();
    if (p_frm_raw == NULL)
    {
        throw std::runtime_error("av_frame_alloc() for p_frm_raw failed");
    }
    p_frm_yuv = av_frame_alloc();
    if (p_frm_yuv == NULL)
    {
        av_frame_free(&p_frm_raw);
        throw std::runtime_error("av_frame_alloc() for p_frm_yuv failed");
    }

    // A6.2 为AVFrame.*data[]手工分配缓冲区，用于存储sws_scale()中目的帧视频数据
    //     p_frm_raw的data_buffer由av_read_frame()分配，因此不需手工分配
    //     p_frm_yuv的data_buffer无处分配，因此在此处手工分配
    buf_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, 
                        p_codec_ctx->width, 
                        p_codec_ctx->height, 
                        1
                        );
    // buffer将作为p_frm_yuv的视频数据缓冲区
    buffer = (uint8_t *)av_malloc(buf_size);
    if (buffer == NULL)
    {
        av_frame_free(&p_frm_raw);
        av_frame_free(&p_frm_yuv);
        throw std::runtime_error("av_malloc() for buffer failed");
    }
    // 使用给定参数设定p_frm_yuv->data和p_frm_yuv->linesize
    int ret = av_image_fill_arrays(p_frm_yuv->data,     // dst data[]
                    p_frm_yuv->linesize, // dst linesize[]
                    buffer,              // src buffer
                    AV_PIX_FMT_YUV420P,  // pixel format
                    p_codec_ctx->width,  // width
                    p_codec_ctx->height, // height
                    1                    // align
                    );
    if (ret < 0)
    {
        av_frame_free(&p_frm_raw);
        av_frame_free(&p_frm_yuv);
        av_free(buffer);
        throw std::runtime_error("av_image_fill_arrays() failed "); 
    }

    
}

Frame::~Frame()
{
    
    av_frame_free(&p_frm_raw);
    av_frame_free(&p_frm_yuv);
    av_free(buffer);
}

// AVFrame* Frame::getAVFrame() const
// {
//     return avFrame;
// }
