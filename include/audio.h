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

extern FF_AudioParams s_audio_param_src;
extern FF_AudioParams s_audio_param_tgt;
extern struct SwrContext *s_audio_swr_ctx;
extern uint8_t *s_resample_buf;  // 重采样输出缓冲区
extern int s_resample_buf_len;      // 重采样输出缓冲区长度

extern bool s_input_finished;   // 文件读取完毕
extern bool s_decode_finished;  // 解码完毕

// 新增全局变量以处理播放速率和播放位置
//static int s_audio_play_time = 0;         // 当前音频播放时间（毫秒）（解了当前包之后，应该处于的时间）
extern float s_audio_playback_rate; // 音频播放速率


int audio_decode_frame(AVCodecContext *p_codec_ctx, std::shared_ptr<myAVPacket> p_packet_ptr, uint8_t *audio_buf, int buf_size);
void sdl_audio_callback(void *userdata, uint8_t *stream, int len);

extern    SDL_AudioSpec       wanted_spec;
    //SDL_AudioSpec       actual_spec;




#endif // !AUDIO_H