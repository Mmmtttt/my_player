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

extern std::chrono::_V2::system_clock::time_point start;  //时间轴计时器的开始点
extern int64_t time_shaft;
extern int64_t a_last_time;
extern int64_t v_last_time;
extern double speed;
extern bool s_playing_pause;
extern bool s_playing_exit;
extern int64_t s_audio_play_time;         // 当前音频播放时间（毫秒）（解了当前包之后，应该处于的时间）
extern int64_t s_video_play_time;

//diff是 当前的时间轴time_shaft-当前正在播放的时间s_audio_play_time
#define Video_Delay_in_Range(diff) -300<diff&&diff<300
#define Video_Delay_Behind(diff) 300<=diff&&diff<=1000
#define Video_Delay_Advanced(diff) -300>=diff&&diff>=-1000
#define Video_Should_Seek(diff) diff>1000||diff<-1000

#define Audio_Delay_in_Range(diff) -50<diff&&diff<50
#define Audio_Delay_Behind(diff) 50<=diff&&diff<=300
#define Audio_Delay_Advanced(diff) -50>=diff&&diff>=-300
#define Audio_Should_Seek(diff) diff>300||diff<-300

void pause();
void action();




#endif
