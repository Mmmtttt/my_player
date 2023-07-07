#include "transport_session.h"


Session::Session(std::string filename,SOCKET socket,TYPE type){
    if(type==SERVER){
        Video video(filename,&video_packet_queue,&audio_packet_queue);
        send_Video_information();
        send_Packet_information();
        std::thread seek_handle_thread([&]{
            while(!close){
                seek_handle();
            }
        });
        while(!close){
            send_Data();
        }
        seek_handle_thread.join();
    }
    else if(type==CLIENT){
        receive_Video_information();
        receive_Packet_information();
        std::thread receive_data_thread([&]{
            receive_Data();
        });
        video.play();
        receive_data_thread.join();
    }
}

void Session::send_Video_information(){
    SEND_ALL(video.v_idx);
    SEND_ALL(*video.v_p_codec_par);
    send_all(server_socket,(const char *)video.v_p_codec_par->extradata,video.v_p_codec_par->extradata_size);
    SEND_ALL(video.v_timebase_in_ms);

    SEND_ALL(video.a_idx);
    SEND_ALL(*video.a_p_codec_par);
    send_all(server_socket,(const char *)video.a_p_codec_par->extradata,video.a_p_codec_par->extradata_size);
    SEND_ALL(video.a_timebase_in_ms);

    v_size=video_packet_queue.get_pkt_count();
    a_size=audio_packet_queue.get_pkt_count();
    SEND_ALL(v_size);
    SEND_ALL(a_size);
}

void Session::send_Packet_information(){
    video_packet_queue.set_curr_pos(0);
    audio_packet_queue.set_curr_pos(0);

    while(video_packet_queue.get_curr_pos()+audio_packet_queue.get_curr_pos()<v_size+a_size-2){

        if((video_packet_queue.get_curr_num())<=(audio_packet_queue.get_curr_num())){
            std::unique_lock<std::mutex>(video_packet_queue.Mutex);
            SEND_ALL(video_packet_queue.get_curr_pkt()->size);
            SEND_ALL(*video_packet_queue.get_curr_pkt());

            video_packet_queue.curr_decode_pos++;
        }
        else{
            std::unique_lock<std::mutex>(audio_packet_queue.Mutex);
            SEND_ALL(audio_packet_queue.get_curr_pkt()->size);
            SEND_ALL(*audio_packet_queue.get_curr_pkt());

            audio_packet_queue.curr_decode_pos++;
        }
        

    }
    

    video_packet_queue.set_curr_pos(0);
    audio_packet_queue.set_curr_pos(0);
}

void Session::send_Data(){
    while(video_packet_queue.get_curr_pos()+audio_packet_queue.get_curr_pos()<v_size+a_size-2){
        //SDL_Delay(30);
        std::unique_lock<std::mutex> vlock(video_packet_queue.Mutex);
        std::unique_lock<std::mutex> alock(audio_packet_queue.Mutex);
        if((video_packet_queue.get_curr_num())<=(audio_packet_queue.get_curr_num())){
            if(video_packet_queue.get_curr_pos()>=v_size)break;
            std::shared_ptr<myAVPacket> temp=video_packet_queue.get_curr_pkt();
            if(temp->is_sended)continue;
            
            SEND_ALL(temp->size);
            SEND_ALL(*temp);
            
            send_all(server_socket,(const char *)temp->mypkt.data,temp->size);
            
            temp->is_sended=true;
            video_packet_queue.curr_decode_pos++;
            if(video_packet_queue.curr_decode_pos>=v_size)video_packet_queue.set_curr_pos(v_size-1);
        }
        else{
            if(audio_packet_queue.get_curr_pos()>=a_size)break;
            std::shared_ptr<myAVPacket> temp=audio_packet_queue.get_curr_pkt();
            if(temp->is_sended)continue;

            SEND_ALL(temp->size);
            SEND_ALL(*temp);
            
            send_all(server_socket,(const char *)temp->mypkt.data,temp->size);
            temp->is_sended=true;
            audio_packet_queue.curr_decode_pos++;
            if(audio_packet_queue.curr_decode_pos>=a_size)audio_packet_queue.set_curr_pos(a_size-1);
        }
    }
}


void Session::seek_handle(){
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

void Session::receive_Video_information(){
    int v_idx;
    RECV_ALL(v_idx);


    AVCodecParameters v_p_codec_par;
    RECV_ALL(v_p_codec_par);
    v_p_codec_par.extradata=(uint8_t*)malloc(v_p_codec_par.extradata_size*sizeof(uint8_t));
    recv_all(client_socket,(char *)v_p_codec_par.extradata,v_p_codec_par.extradata_size);

    double v_timebase_in_ms;
    RECV_ALL(v_timebase_in_ms);

    
    
    int a_idx;
    RECV_ALL(a_idx);

    AVCodecParameters a_p_codec_par;
    RECV_ALL(a_p_codec_par);
    a_p_codec_par.extradata=(uint8_t*)malloc(a_p_codec_par.extradata_size*sizeof(uint8_t));
    recv_all(client_socket,(char *)a_p_codec_par.extradata,a_p_codec_par.extradata_size);
    
    double a_timebase_in_ms;
    RECV_ALL(a_timebase_in_ms);


    int64_t v_size,a_size;
    RECV_ALL(v_size);
    RECV_ALL(a_size);

    Video video(v_idx,&v_p_codec_par,v_timebase_in_ms,a_idx,&a_p_codec_par,a_timebase_in_ms);
}

void Session::receive_Packet_information(){
    for(int64_t i=0;i<v_size+a_size-2;i++){
        std::shared_ptr<myAVPacket> temp=std::shared_ptr<myAVPacket>(new myAVPacket);
        int64_t size;
        RECV_ALL(size);
        RECV_ALL(*temp);
        temp->mypkt.data=NULL;
        temp->mypkt.buf=NULL;
        temp->is_recived=false;

        if(temp->mypkt.stream_index==AVMEDIA_TYPE_VIDEO){
            video_packet_queue.packet_queue_push(temp);
        }
        else if(temp->mypkt.stream_index==AVMEDIA_TYPE_AUDIO){
            audio_packet_queue.packet_queue_push(temp);
        }
    }
}

void Session::receive_Data(){
    while(!close&&(video_packet_queue.curr_decode_pos+audio_packet_queue.curr_decode_pos<v_size+a_size-2)){
        std::shared_ptr<myAVPacket> temp=std::shared_ptr<myAVPacket>(new myAVPacket);
        int64_t size;
        RECV_ALL(size);
        av_new_packet(&temp->mypkt, size);
        char *buf=(char *)temp->mypkt.buf,*data=(char *)temp->mypkt.data;
        RECV_ALL(*temp);
        recv_all(client_socket,(char *)data,size);
        temp->mypkt.data=(uint8_t *)data;
        temp->mypkt.buf=(AVBufferRef *)buf;

        bool ret;
        if(temp->mypkt.stream_index==AVMEDIA_TYPE_VIDEO){
            temp->is_recived=true;
            ret=video_packet_queue.insert(temp);
            //std::cout<<"video receive packet "<<temp->id_in_queue<<std::endl;
        }
        else if(temp->mypkt.stream_index==AVMEDIA_TYPE_AUDIO){
            temp->is_recived=true;
            ret=audio_packet_queue.insert(temp);
            //std::cout<<"audio receive packet "<<temp->id_in_queue<<std::endl;
        }
        if(!ret)return;
        
    }
    return;
}