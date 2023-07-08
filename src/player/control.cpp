#include "control.h"
#include "win_net.h"
#include "packetQueue.h"

std::chrono::_V2::system_clock::time_point start;
int64_t time_shaft = 0;
int64_t a_last_time = 0;
int64_t v_last_time = 0;
double speed = 1.0;
bool s_playing_pause = false;
bool s_playing_exit = false;
int64_t s_audio_play_time = 0;
int64_t s_video_play_time = 0;


SOCKET listen_socket;
sockaddr_in serverService;
SOCKET accept_socket;
SOCKET client_socket;
sockaddr_in clientService;

void pause(){

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    v_last_time=elapsed.count();
    a_last_time=elapsed.count();
    
    if(!s_playing_pause)SDL_PauseAudio(1);
    s_playing_pause=true;
    printf("player %s\n", s_playing_pause ? "pause" : "continue");
    
}

void action(){

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    v_last_time=elapsed.count();
    a_last_time=elapsed.count();
    
    if(s_playing_pause)SDL_PauseAudio(0);
    s_playing_pause=false;
    printf("player %s\n", s_playing_pause ? "pause" : "continue");

}

void seek_callback(int stream_idx,int64_t id){
    send_all(client_socket,(const char *)&stream_idx,sizeof(int));
    send_all(client_socket,(const char *)&id,sizeof(int64_t));
}
