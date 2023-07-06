#include "control.h"
#include "win_net.h"
#include "packetQueue.h"

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

void seek_callback(int stream_idx,int64_t &id){
    send_all(client_socket,(const char *)&stream_idx,sizeof(int));
    send_all(client_socket,(const char *)&id,sizeof(int64_t));
}

void seek_handle()
{
    int stream_idx;
    int64_t id;
    int ret=recv_all(accept_socket,(char*)&stream_idx,sizeof(stream_idx));
    ret=recv_all(accept_socket,(char*)&id,sizeof(id));
    if(ret<=0)return;
    
    // if(num_mapping_id_in_queue[num].first==0){
    //     std::unique_lock<std::mutex> lock(video_packet_queue.Mutex);
    //     video_packet_queue.curr_decode_pos=num_mapping_id_in_queue[num].second;
    //     if(video_packet_queue.curr_decode_pos<0)video_packet_queue.curr_decode_pos=0;
    //     int64_t a = video_packet_queue.curr_decode_pos;
    //     std::cout<<"                    a= "<<a<<std::endl;
    //     auto temp=&num_mapping_id_in_queue;
    // }
    // else if(num_mapping_id_in_queue[num].first==1){
    //     std::unique_lock<std::mutex> lock(audio_packet_queue.Mutex);
    //     audio_packet_queue.curr_decode_pos=num_mapping_id_in_queue[num].second;
    //     if(audio_packet_queue.curr_decode_pos<0)audio_packet_queue.curr_decode_pos=0;
    //     int64_t a = audio_packet_queue.curr_decode_pos;
    //     std::cout<<"                    a= "<<a<<std::endl;
    // }
    std::cout<<"                    id= "<<id<<"    queue= "<<stream_idx<<std::endl;
    // {
    //     std::unique_lock<std::mutex> lock(video_packet_queue.Mutex);
    //     for(int64_t i=0;i<=num;i++){
    //         if(i>=video_packet_queue.pkts_ptr.size())break;
    //         if(video_packet_queue.pkts_ptr[i]->num==num){
    //             video_packet_queue.curr_decode_pos=video_packet_queue.pkts_ptr[i]->id_in_queue;
    //             std::cout<<video_packet_queue.curr_decode_pos<<std::endl;
    //             return;
    //         }
    //     }
    // }
    // {
    //     std::unique_lock<std::mutex> lock(audio_packet_queue.Mutex);
    //     for(int64_t i=0;i<=num;i++){
    //         if(i>=video_packet_queue.pkts_ptr.size())return;
    //         if(audio_packet_queue.pkts_ptr[i]->num==num){
    //             audio_packet_queue.curr_decode_pos=audio_packet_queue.pkts_ptr[i]->id_in_queue;
    //             std::cout<<audio_packet_queue.curr_decode_pos<<std::endl;
    //             return;
    //         }
    //     }
    // }
    if(stream_idx==1){
        std::unique_lock<std::mutex> lock(audio_packet_queue.Mutex);
        audio_packet_queue.curr_decode_pos=id;
        if(audio_packet_queue.curr_decode_pos>=audio_packet_queue.pkts_ptr.size()){audio_packet_queue.curr_decode_pos=audio_packet_queue.pkts_ptr.size()-1;return;};
        SEND_ALL(audio_packet_queue.pkts_ptr[audio_packet_queue.curr_decode_pos]->size);
        SEND_ALL(*audio_packet_queue.pkts_ptr[audio_packet_queue.curr_decode_pos]);
        
        send_all(accept_socket,(const char *)audio_packet_queue.pkts_ptr[audio_packet_queue.curr_decode_pos]->mypkt.data,audio_packet_queue.pkts_ptr[audio_packet_queue.curr_decode_pos]->size);
        std::cout<<"call back   audio packet "<<audio_packet_queue.pkts_ptr[audio_packet_queue.curr_decode_pos]->id_in_queue<<" sended"<<std::endl;
        audio_packet_queue.curr_decode_pos++;
        
    }
    else if(stream_idx==0){
        std::unique_lock<std::mutex> lock(video_packet_queue.Mutex);
        video_packet_queue.curr_decode_pos=id;
        if(video_packet_queue.curr_decode_pos>=video_packet_queue.pkts_ptr.size()){video_packet_queue.curr_decode_pos=video_packet_queue.pkts_ptr.size()-1;return;};
        SEND_ALL(video_packet_queue.pkts_ptr[video_packet_queue.curr_decode_pos]->size);
        SEND_ALL(*video_packet_queue.pkts_ptr[video_packet_queue.curr_decode_pos]);
        
        send_all(accept_socket,(const char *)video_packet_queue.pkts_ptr[video_packet_queue.curr_decode_pos]->mypkt.data,video_packet_queue.pkts_ptr[video_packet_queue.curr_decode_pos]->size);
        std::cout<<"call back   video packet "<<video_packet_queue.pkts_ptr[video_packet_queue.curr_decode_pos]->id_in_queue<<" sended"<<std::endl;
        video_packet_queue.curr_decode_pos++;
        
    }
    
}