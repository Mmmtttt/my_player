#include "video.h"
#include "win_net.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <functional>
#include <thread>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

// ... 客户端的其他FFmpeg代码在这里 ...


std::chrono::_V2::system_clock::time_point start;
int64_t time_shaft = 0;
int64_t a_last_time = 0;
int64_t v_last_time = 0;
double speed = 1.0;
bool s_playing_pause = false;
bool s_playing_exit = false;
int64_t s_audio_play_time = 0;
int64_t s_video_play_time = 0;


SOCKET client_socket;
sockaddr_in clientService;
SOCKET listen_socket;
sockaddr_in serverService;
SOCKET accept_socket;


int main(int argc, char* argv[]) {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != NO_ERROR) {
        std::cout << "Initialization error.\n";
        return 1;
    }

    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == INVALID_SOCKET) {
        std::cout << "Error at socket: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    clientService;
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr("127.0.0.1");  // 服务器地址
    clientService.sin_port = htons(12345);  // 同一端口

    if (connect(client_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
        std::cout << "Failed to connect.\n";
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server.\n";

    // ... 从服务器接收数据 ...

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



    int64_t aaa;
    RECV_ALL(aaa);

    Video video(v_idx,&v_p_codec_par,v_timebase_in_ms,a_idx,&a_p_codec_par,a_timebase_in_ms);
    video_packet_queue.initial(v_size);
    audio_packet_queue.initial(a_size);

    // std::cout<<sizeof(myAVPacket)<<" "<<sizeof(AVPacket)<<std::endl;
    
    
    std::thread t([&]{
        auto vQ = &video_packet_queue;
        auto aQ = &audio_packet_queue;
        for(int i=0;i<v_size+a_size;i++){
            std::shared_ptr<myAVPacket> temp=std::shared_ptr<myAVPacket>(new myAVPacket);
            int64_t size;
            RECV_ALL(size);
            av_new_packet(&temp->mypkt, size);
            char *buf=(char *)temp->mypkt.buf,*data=(char *)temp->mypkt.data;
            RECV_ALL(*temp);
            //uint8_t *data = (uint8_t*)malloc(size*sizeof(uint8_t));
            recv_all(client_socket,(char *)data,size);
            temp->mypkt.data=(uint8_t *)data;
            temp->mypkt.buf=(AVBufferRef *)buf;

            if(temp->mypkt.stream_index==AVMEDIA_TYPE_VIDEO){
                video_packet_queue.insert(temp);
                //video_packet_queue.cond.notify_all();
            }
            else if(temp->mypkt.stream_index==AVMEDIA_TYPE_AUDIO){
                audio_packet_queue.insert(temp);
                //audio_packet_queue.cond.notify_all();
            }
        }
    });


    video.play();
    t.join();

    return 0;
}