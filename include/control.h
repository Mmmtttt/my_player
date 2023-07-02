#ifndef CONTROL_H
#define CONTROL_H

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
#include <SDL.h>
#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL_rect.h>
#include <libavutil/frame.h>
}

#include <list>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <chrono>
#include <deque>

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

extern std::chrono::_V2::system_clock::time_point start;
extern int64_t time_shaft;
extern int64_t last_time;
extern double speed;
extern bool s_playing_pause;
extern int64_t s_audio_play_time;         // 当前音频播放时间（毫秒）（解了当前包之后，应该处于的时间）
extern int64_t s_video_play_time;



#endif
