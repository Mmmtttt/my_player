#ifndef FRAME_H
#define FRAME_H



#include <iostream>
#include <memory>

#include "control.h"


typedef struct AudioParams {
    int freq;
    int channels;
    int64_t channel_layout;
    enum AVSampleFormat fmt;
    int frame_size;
    int bytes_per_sec;
} FF_AudioParams;


class videoFrame {
public:
    videoFrame(AVCodecContext* p_codec_ctx);
    ~videoFrame();

    void init_Buffer();

    AVFrame* p_frm_raw = NULL; 
    AVFrame* p_frm_yuv = NULL; 
    //AVRational timebase;
    int width;
    int height;
private:     
    int  buf_size;
    uint8_t* buffer = NULL;
};

class audioFrame {
public:
    audioFrame(SwrContext *swr_ctx,AVCodecContext* p_codec_ctx,FF_AudioParams s_audio_param_tgt);
    ~audioFrame();

    int reSample(SwrContext *swr_ctx,AVCodecContext* p_codec_ctx);

    AVFrame* frame = NULL; 
    FF_AudioParams s_audio_param_src;

    
    int frm_size = 0;
    int nb_samples = 0; 
    uint8_t *p_cp_buf = NULL;
    int cp_len = 0;
    //int  buf_size;
    uint8_t* buffer = NULL;
    uint8_t *s_resample_buf = NULL;  // 重采样输出缓冲区
    int s_resample_buf_len = 0;      // 重采样输出缓冲区长度
};

#endif