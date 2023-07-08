#ifndef TRANSPORT_SESSION_H
#define TRANSPORT_SESSION_H


#include "win_net.h"
#include "control.h"
#include "video.h"

class Session{
    public:
        Session(std::string name,SOCKET socket,TYPE type);
        ~Session();


        std::shared_ptr<Video> video;
        std::shared_ptr<packetQueue> video_packet_queue;
        std::shared_ptr<packetQueue> audio_packet_queue;
        int64_t v_size;
        int64_t a_size;
        SOCKET server_socket;
        SOCKET client_socket;

        bool close=false;

        void send_Video_information();
        void send_Packet_information();
        void send_Data();

        void receive_Video_information();
        void receive_Packet_information();
        void receive_Data();

        void seek_handle();
};





#endif