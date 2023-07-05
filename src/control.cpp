#include "control.h"
#include "win_net.h"
#include "packetQueue.h"

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

void seek_callback(int64_t &num){
    send(client_socket,(const char *)&num,sizeof(int64_t),0);
}

void seek_handle()
{
    int64_t num;
    int ret=recv(accept_socket,(char*)&num,sizeof(num),0);
    if(ret<=0)return;
    
    if(num_mapping_id_in_queue[num].first==0){
        video_packet_queue.curr_decode_pos=num_mapping_id_in_queue[num].second;

    }
    else if(num_mapping_id_in_queue[num].first==1){
        audio_packet_queue.curr_decode_pos=num_mapping_id_in_queue[num].second;

    }
    
}