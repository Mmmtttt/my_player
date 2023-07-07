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

void seek_handle()
{
    int stream_idx;
    int64_t id;
    int ret=recv_all(server_socket,(char*)&stream_idx,sizeof(stream_idx));
    ret=recv_all(server_socket,(char*)&id,sizeof(id));
    if(ret<=0)return;
    
    
    if(stream_idx==1){
        std::unique_lock<std::mutex> lock(audio_packet_queue.Mutex);
        audio_packet_queue.set_curr_pos(id);

        std::shared_ptr<myAVPacket> temp=audio_packet_queue.get_curr_pkt();
        SEND_ALL(temp->size);
        SEND_ALL(*temp);
        
        send_all(server_socket,(const char *)temp->mypkt.data,temp->size);
        std::cout<<"call back   audio packet "<<temp->id_in_queue<<" sended"<<std::endl;
        audio_packet_queue.curr_decode_pos++;
        
    }
    else if(stream_idx==0){
        std::unique_lock<std::mutex> lock(video_packet_queue.Mutex);
        video_packet_queue.set_curr_pos(id);

        std::shared_ptr<myAVPacket> temp=video_packet_queue.get_curr_pkt();
        SEND_ALL(temp->size);
        SEND_ALL(*temp);
        
        send_all(server_socket,(const char *)temp->mypkt.data,temp->size);
        std::cout<<"call back   video packet "<<temp->id_in_queue<<" sended"<<std::endl;
        video_packet_queue.curr_decode_pos++;
        
    }
    
}