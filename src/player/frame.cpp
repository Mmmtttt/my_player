#include "frame.h"

videoFrame::videoFrame(AVCodecContext* p_codec_ctx)
{
    //timebase=p_codec_ctx->time_base;
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

    width=p_codec_ctx->width;
    height=p_codec_ctx->height;

    init_Buffer();
    
}

videoFrame::~videoFrame()
{
    
    av_frame_free(&p_frm_raw);
    av_frame_free(&p_frm_yuv);
    av_free(buffer);
}

void videoFrame::init_Buffer(){
    av_free(buffer);
    // A6.2 为AVFrame.*data[]手工分配缓冲区，用于存储sws_scale()中目的帧视频数据
    //     p_frm_raw的data_buffer由av_read_frame()分配，因此不需手工分配
    //     p_frm_yuv的data_buffer无处分配，因此在此处手工分配
    buf_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, 
                        width, 
                        height, 
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
                    width,  // width
                    height, // height
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




audioFrame::audioFrame(SwrContext *swr_ctx,AVCodecContext* p_codec_ctx,FF_AudioParams s_audio_param_tgt){
    s_audio_param_src = s_audio_param_tgt;
    
    frame = av_frame_alloc();
    if (frame == NULL)
    {
        throw std::runtime_error("av_frame_alloc() for frame failed");
    }

    //avcodec_receive_frame(p_codec_ctx, frame);

    

}

audioFrame::~audioFrame(){
    av_frame_unref(frame);
}

int audioFrame::reSample(SwrContext *swr_ctx,AVCodecContext* p_codec_ctx){
    if (frame->format         != s_audio_param_src.fmt            ||
                frame->channel_layout != s_audio_param_src.channel_layout ||
                frame->sample_rate    != s_audio_param_src.freq)
    {//std::cout<<"d1"<<std::endl;
        //swr_free(&swr_ctx);std::cout<<"done"<<std::endl;
        // 使用frame(源)和is->audio_tgt(目标)中的音频参数来设置is->swr_ctx
        swr_ctx = swr_alloc_set_opts(NULL,
                                    s_audio_param_src.channel_layout, 
                                    s_audio_param_src.fmt, 
                                    s_audio_param_src.freq,
                                    frame->channel_layout,           
                                    (AVSampleFormat)frame->format, 
                                    frame->sample_rate,
                                    0,
                                    NULL);//std::cout<<"d2"<<std::endl;
        if (swr_ctx == NULL || swr_init(swr_ctx) < 0)
        {
            printf("Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
                    frame->sample_rate, av_get_sample_fmt_name((AVSampleFormat)frame->format), frame->channels,
                    s_audio_param_src.freq, av_get_sample_fmt_name(s_audio_param_src.fmt), s_audio_param_src.channels);
            swr_free(&swr_ctx);
            throw std::runtime_error("Cannot create sample rate converter for conversion ");
        }
        
        // 使用frame中的参数更新s_audio_param_src，第一次更新后后面基本不用执行此if分支了，因为一个音频流中各frame通用参数一样
        s_audio_param_src.channel_layout = frame->channel_layout;
        s_audio_param_src.channels       = frame->channels;
        s_audio_param_src.freq           = frame->sample_rate;
        s_audio_param_src.fmt            = (AVSampleFormat)frame->format;
    }
    if (swr_ctx != NULL)        // 重采样
    {
        // 重采样输入参数1：输入音频样本数是p_frame->nb_samples
        // 重采样输入参数2：输入音频缓冲区
        const uint8_t **in = (const uint8_t **)frame->extended_data;
        // 重采样输出参数1：输出音频缓冲区尺寸
        // 重采样输出参数2：输出音频缓冲区
        uint8_t **out = &s_resample_buf;
        // 重采样输出参数：输出音频样本数(多加了256个样本)
        int out_count = (int64_t)frame->nb_samples * s_audio_param_src.freq / frame->sample_rate + 256;
        // 重采样输出参数：输出音频缓冲区尺寸(以字节为单位)
        int out_size  = av_samples_get_buffer_size(NULL, s_audio_param_src.channels, out_count, s_audio_param_src.fmt, 0);
        if (out_size < 0)
        {
            printf("av_samples_get_buffer_size() failed\n");
            return -1;
        }
        
        if (s_resample_buf == NULL)
        {
            av_fast_malloc(&s_resample_buf, (unsigned int *)&s_resample_buf_len, out_size);
        }
        if (s_resample_buf == NULL)
        {
            return AVERROR(ENOMEM);
        }
        // 音频重采样：返回值是重采样后得到的音频数据中单个声道的样本数
        frame->nb_samples = swr_convert(swr_ctx, out, out_count, in, frame->nb_samples);
        if (frame->nb_samples < 0) {
            printf("swr_convert() failed\n");
            return -1;
        }
        if (frame->nb_samples == out_count)
        {
            printf("audio buffer is probably too small\n");
            if (swr_init(swr_ctx) < 0)
                swr_free(&swr_ctx);
        }

        // 重采样返回的一帧音频数据大小(以字节为单位)
        p_cp_buf = s_resample_buf;
        cp_len = nb_samples * s_audio_param_src.channels * av_get_bytes_per_sample(s_audio_param_src.fmt);
    }
    else    // 不重采样
    {
        // 根据相应音频参数，获得所需缓冲区大小
        frm_size = av_samples_get_buffer_size(
                NULL, 
                p_codec_ctx->channels,
                frame->nb_samples,
                p_codec_ctx->sample_fmt,
                1);
        
        //printf("frame size %d, buffer size %d\n", frm_size, buf_size);
        //assert(frm_size <= buf_size);

        p_cp_buf = frame->data[0];
        cp_len = frm_size;
    }
}