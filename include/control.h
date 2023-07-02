#ifndef CONTROL_H
#define CONTROL_H

#include <list>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <chrono>
#include <deque>

extern std::chrono::_V2::system_clock::time_point start;
extern int64_t time_shaft;
extern int64_t last_time;
extern double speed;
extern bool s_playing_pause;


#endif
