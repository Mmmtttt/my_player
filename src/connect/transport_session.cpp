#include "transport_session.h"
#include "my_portocol.h"

Session::Session(std::string filename,SOCKET socket,TYPE type){
    if(type==SERVER){
        server_socket=socket;
        try{video=std::make_shared<Video>(filename,type);}
        catch(const std::exception& e){
            std::cout<<e.what()<<std::endl;
            std::string message("NO_FOUND");
            Send_Message(server_socket,message);
            return;
        }

        std::string message("FOUNDED");
        Send_Message(server_socket,message);
        video->push_All_Packets();
        video_packet_queue=video->video_packet_queue;
        audio_packet_queue=video->audio_packet_queue;
        send_Video_information();
        send_Packet_information();
        std::thread seek_handle_thread([&]{
            while(!close){
                //std::cout<<"seek_handle"<<std::endl;
                seek_handle();
            }
        });
        while(!close){
            send_Data();
            Sleep(50);
        }
        close=true;
        seek_handle_thread.join();

        std::string message2("exit_ack");
        Send_Message(server_socket,message2);
    }
    else if(type==CLIENT){
        client_socket=socket;
        ::client_socket=socket;

        std::string message;
        Recv_Message(client_socket,message);
        if(message=="NO_FOUND"){close=true; return;};

        receive_Video_information();
        video_packet_queue=video->video_packet_queue;
        audio_packet_queue=video->audio_packet_queue;
        receive_Packet_information();
        std::thread receive_data_thread([&]{
            receive_Data();
        });
        video->play();
        std::string message2("exit");
        Send_Message(client_socket,message2);

        close=true;
        std::cout<<"waiting join"<<std::endl;
        receive_data_thread.join();
        std::cout<<"joined"<<std::endl;
    }
}

Session::~Session(){std::cout<<"session destory"<<std::endl;}

void Session::send_Video_information(){
    SEND_ALL(video->v_idx);
    SEND_ALL(*video->v_p_codec_par);
    send_all(server_socket,(const char *)video->v_p_codec_par->extradata,video->v_p_codec_par->extradata_size);
    SEND_ALL(video->v_timebase_in_ms);

    SEND_ALL(video->a_idx);
    SEND_ALL(*video->a_p_codec_par);
    send_all(server_socket,(const char *)video->a_p_codec_par->extradata,video->a_p_codec_par->extradata_size);
    SEND_ALL(video->a_timebase_in_ms);

    v_size=video_packet_queue->get_pkt_count();
    a_size=audio_packet_queue->get_pkt_count();
    SEND_ALL(v_size);
    SEND_ALL(a_size);

    int a=sizeof(int);
    int b=sizeof(int64_t);

    std::cout<<"Video_info_size : "<<sizeof(*video->v_p_codec_par)+video->v_p_codec_par->extradata_size+sizeof(*video->a_p_codec_par)+video->a_p_codec_par->extradata_size+2*a+2*b<<std::endl;
}

void Session::send_Packet_information(){
    video_packet_queue->set_curr_pos(0);
    audio_packet_queue->set_curr_pos(0);


    int64_t Packet_info_size=0;
    while(video_packet_queue->get_curr_pos()+audio_packet_queue->get_curr_pos()<v_size+a_size-2){

        std::shared_ptr<myAVPacket> temp_ptr;
        if((video_packet_queue->get_curr_num())<=(audio_packet_queue->get_curr_num())){
            std::unique_lock<std::mutex>(video_packet_queue->Mutex);
            //SEND_ALL(video_packet_queue->get_curr_pkt()->size);
            //SEND_ALL(*video_packet_queue->get_curr_pkt());
            temp_ptr=video_packet_queue->get_curr_pkt();
            

            video_packet_queue->curr_decode_pos++;
            // Packet_info_size+=sizeof(*video_packet_queue->get_curr_pkt());
        }
        else{
            std::unique_lock<std::mutex>(audio_packet_queue->Mutex);
            //SEND_ALL(audio_packet_queue->get_curr_pkt()->size);
            //SEND_ALL(*audio_packet_queue->get_curr_pkt());
            temp_ptr=audio_packet_queue->get_curr_pkt();

            audio_packet_queue->curr_decode_pos++;
            // Packet_info_size+=sizeof(*audio_packet_queue->get_curr_pkt());
        }
        SEND_ALL(temp_ptr->id_in_queue);
        SEND_ALL(temp_ptr->num);
        SEND_ALL(temp_ptr->mypkt.dts);
        SEND_ALL(temp_ptr->mypkt.stream_index);
        SEND_ALL(temp_ptr->mypkt.flags);
        Packet_info_size+=8+8+8+4+4;
        
    }
    std::cout<<"Packet_info_size : "<<Packet_info_size<<std::endl;

    video_packet_queue->set_curr_pos(0);
    audio_packet_queue->set_curr_pos(0);
}

void Session::send_Data(){
    while(!close&&(video_packet_queue->get_curr_pos()+audio_packet_queue->get_curr_pos()<v_size+a_size-2)){
        //SDL_Delay(40);
        std::unique_lock<std::mutex> vlock(video_packet_queue->Mutex);
        std::unique_lock<std::mutex> alock(audio_packet_queue->Mutex);
        std::string message("data");
        Send_Message(server_socket,message);


        if((video_packet_queue->get_curr_num())<=(audio_packet_queue->get_curr_num())){
            if(video_packet_queue->get_curr_pos()>=v_size)break;
            std::shared_ptr<myAVPacket> temp=video_packet_queue->get_curr_pkt();
            //if(temp->is_sended)continue;
            
            SEND_ALL(temp->size);
            SEND_ALL(*temp);
            
            send_all(server_socket,(const char *)temp->mypkt.data,temp->size);
            
            temp->is_sended=true;
            video_packet_queue->curr_decode_pos++;
            if(video_packet_queue->curr_decode_pos>=v_size)video_packet_queue->set_curr_pos(v_size-1);
            //std::cout<<"packet "<<video_packet_queue->get_curr_num()<<" sended"<<std::endl;
        }
        else{
            if(audio_packet_queue->get_curr_pos()>=a_size)break;
            std::shared_ptr<myAVPacket> temp=audio_packet_queue->get_curr_pkt();
            //if(temp->is_sended)continue;

            SEND_ALL(temp->size);
            SEND_ALL(*temp);
            
            send_all(server_socket,(const char *)temp->mypkt.data,temp->size);
            temp->is_sended=true;
            audio_packet_queue->curr_decode_pos++;
            if(audio_packet_queue->curr_decode_pos>=a_size)audio_packet_queue->set_curr_pos(a_size-1);
            //std::cout<<"packet "<<audio_packet_queue->get_curr_num()<<" sended"<<std::endl;
        }
    }
}


void Session::seek_handle(){
    int stream_idx;
    int64_t id;

    std::string message;
    Recv_Message(server_socket,message);
    if(message=="exit"){close=true; return;};

    int ret=recv_all(server_socket,(char*)&stream_idx,sizeof(stream_idx));
    ret=recv_all(server_socket,(char*)&id,sizeof(id));
    if(ret<=0){close=true; return;};
    
    
    if(stream_idx==1){
        std::unique_lock<std::mutex> lock(audio_packet_queue->Mutex);
        audio_packet_queue->set_curr_pos(id);

        std::shared_ptr<myAVPacket> temp=audio_packet_queue->get_curr_pkt();
        SEND_ALL(temp->size);
        SEND_ALL(*temp);
        
        send_all(server_socket,(const char *)temp->mypkt.data,temp->size);
        std::cout<<"call back   audio packet "<<temp->id_in_queue<<" sended"<<std::endl;
        audio_packet_queue->curr_decode_pos++;
        
    }
    else if(stream_idx==0){
        std::unique_lock<std::mutex> lock(video_packet_queue->Mutex);
        video_packet_queue->set_curr_pos(id);

        std::shared_ptr<myAVPacket> temp=video_packet_queue->get_curr_pkt();
        SEND_ALL(temp->size);
        SEND_ALL(*temp);
        
        send_all(server_socket,(const char *)temp->mypkt.data,temp->size);
        std::cout<<"call back   video packet "<<temp->id_in_queue<<" sended"<<std::endl;
        video_packet_queue->curr_decode_pos++;
        
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



    RECV_ALL(v_size);
    RECV_ALL(a_size);

    video=std::make_shared<Video>(v_idx,&v_p_codec_par,v_timebase_in_ms,a_idx,&a_p_codec_par,a_timebase_in_ms);
}

void Session::receive_Packet_information(){
    for(int64_t i=0;i<v_size+a_size-2;i++){
        std::shared_ptr<myAVPacket> temp=std::shared_ptr<myAVPacket>(new myAVPacket);
        // int64_t size;
        // RECV_ALL(size);
        RECV_ALL(temp->id_in_queue);
        RECV_ALL(temp->num);
        RECV_ALL(temp->mypkt.dts);
        RECV_ALL(temp->mypkt.stream_index);
        RECV_ALL(temp->mypkt.flags);
        temp->mypkt.data=NULL;
        temp->mypkt.buf=NULL;
        temp->is_recived=false;

        if(temp->mypkt.stream_index==AVMEDIA_TYPE_VIDEO){
            video_packet_queue->packet_queue_push(temp);
        }
        else if(temp->mypkt.stream_index==AVMEDIA_TYPE_AUDIO){
            audio_packet_queue->packet_queue_push(temp);
        }
    }
}

void Session::receive_Data(){
    while(!close&&(video_packet_queue->curr_decode_pos+audio_packet_queue->curr_decode_pos<v_size+a_size-2)){
        std::shared_ptr<myAVPacket> temp=std::shared_ptr<myAVPacket>(new myAVPacket);
        std::string message;
        Recv_Message(client_socket,message);
        if(message=="exit_ack"){close=true;return;}


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
            ret=video_packet_queue->insert(temp);
            //std::cout<<"video receive packet "<<temp->id_in_queue<<std::endl;
        }
        else if(temp->mypkt.stream_index==AVMEDIA_TYPE_AUDIO){
            temp->is_recived=true;
            ret=audio_packet_queue->insert(temp);
            //std::cout<<"audio receive packet "<<temp->id_in_queue<<std::endl;
        }
        if(!ret)return;
        
    }
    return;
}