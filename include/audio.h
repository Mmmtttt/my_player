#ifndef AUDIO_H
#define AUDIO_H




#include"control.h"
#include"packetQueue.h"
#include"frame.h"
#include <assert.h>

// struct FF_AudioParams {
//     int freq;
//     int channels;
//     int64_t channel_layout;
//     enum AVSampleFormat fmt;
//     int frame_size;
//     int bytes_per_sec;
// };

static FF_AudioParams s_audio_param_src;
static FF_AudioParams s_audio_param_tgt;
static struct SwrContext *s_audio_swr_ctx;
static uint8_t *s_resample_buf = NULL;  // 重采样输出缓冲区
static int s_resample_buf_len = 0;      // 重采样输出缓冲区长度

static bool s_input_finished = false;   // 文件读取完毕
static bool s_decode_finished = false;  // 解码完毕

// 新增全局变量以处理播放速率和播放位置
//static int s_audio_play_time = 0;         // 当前音频播放时间（毫秒）（解了当前包之后，应该处于的时间）
static float s_audio_playback_rate = 1.0; // 音频播放速率


int audio_decode_frame(AVCodecContext *p_codec_ctx, std::shared_ptr<myAVPacket> p_packet_ptr, uint8_t *audio_buf, int buf_size);
void sdl_audio_callback(void *userdata, uint8_t *stream, int len);

extern    SDL_AudioSpec       wanted_spec;
    //SDL_AudioSpec       actual_spec;




#endif // !AUDIO_H