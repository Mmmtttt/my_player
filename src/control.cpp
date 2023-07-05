#include "control.h"

void pause(){
    if(s_playing_pause)return;
    else{
        s_playing_pause=!s_playing_pause;
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        v_last_time=elapsed.count();
        a_last_time=elapsed.count();
        
        SDL_PauseAudio(1);
        printf("player %s\n", s_playing_pause ? "pause" : "continue");
    }
}

void action(){
    if(!s_playing_pause)return;
    else{
        s_playing_pause=!s_playing_pause;
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        v_last_time=elapsed.count();
        a_last_time=elapsed.count();
        
        SDL_PauseAudio(0);
        printf("player %s\n", s_playing_pause ? "pause" : "continue");
    }
}