#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include "video.h"


#pragma comment(lib, "Ws2_32.lib")

// ... 服务器的其他FFmpeg代码在这里 ...


std::chrono::_V2::system_clock::time_point start;
int64_t time_shaft = 0;
int64_t a_last_time = 0;
int64_t v_last_time = 0;
double speed = 1.0;
bool s_playing_pause = false;
bool s_playing_exit = false;
int64_t s_audio_play_time = 0;
int64_t s_video_play_time = 0;


int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cout << "Please provide a movie file" << std::endl;
        return -1;
    }





    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != NO_ERROR) {
        std::cout << "Initialization error.\n";
        return 1;
    }

    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket == INVALID_SOCKET) {
        std::cout << "Error at socket: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverService;
    serverService.sin_family = AF_INET;
    serverService.sin_addr.s_addr = INADDR_ANY;
    serverService.sin_port = htons(12345);  // 选择一个端口

    if (bind(listen_socket, (SOCKADDR*)&serverService, sizeof(serverService)) == SOCKET_ERROR) {
        std::cout << "bind() failed.\n";
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    if (listen(listen_socket, 1) == SOCKET_ERROR) {
        std::cout << "Error listening on socket.\n";
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    SOCKET accept_socket = accept(listen_socket, NULL, NULL);
    if (accept_socket == INVALID_SOCKET) {
        std::cout << "accept() failed: " << WSAGetLastError() << '\n';
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Client connected.\n";

    // ... 发送数据到客户端 ...

    Video video(argv[1]);
    video.v_decoder->push_All_Packets(video.p_fmt_ctx);

    

    // send(accept_socket, (const char *)&video.v_idx,sizeof(video.v_idx),0);
    // send(accept_socket, (const char *)video.a_p_codec_par, sizeof(*video.a_p_codec_par), 0);
    SEND(video.v_idx);
    SEND(*video.v_p_codec_par);
    SEND(video.v_timebase_in_ms);

    SEND(video.a_idx);
    SEND(*video.a_p_codec_par);
    SEND(video.a_timebase_in_ms);

    int64_t v_size=video_packet_queue.pkts_ptr.size(),a_size=audio_packet_queue.pkts_ptr.size();
    SEND(v_size);
    SEND(a_size);

    int64_t aaa=123456789;
    SEND(aaa);

    while(video_packet_queue.curr_decode_pos<video_packet_queue.pkts_ptr.size()&&audio_packet_queue.curr_decode_pos<audio_packet_queue.pkts_ptr.size()){
        if((video_packet_queue.pkts_ptr[video_packet_queue.curr_decode_pos]->num)<=(audio_packet_queue.pkts_ptr[audio_packet_queue.curr_decode_pos]->num)){
            SEND(*video_packet_queue.pkts_ptr[video_packet_queue.curr_decode_pos]);
            SEND(video_packet_queue.pkts_ptr[video_packet_queue.curr_decode_pos]->size);
            send(accept_socket,(const char *)video_packet_queue.pkts_ptr[video_packet_queue.curr_decode_pos]->mypkt.data,video_packet_queue.pkts_ptr[video_packet_queue.curr_decode_pos]->size,0);
            video_packet_queue.curr_decode_pos++;
        }
        else{
            SEND(*audio_packet_queue.pkts_ptr[audio_packet_queue.curr_decode_pos]);
            SEND(audio_packet_queue.pkts_ptr[audio_packet_queue.curr_decode_pos]->size);
            send(accept_socket,(const char *)audio_packet_queue.pkts_ptr[audio_packet_queue.curr_decode_pos]->mypkt.data,audio_packet_queue.pkts_ptr[audio_packet_queue.curr_decode_pos]->size,0);
            audio_packet_queue.curr_decode_pos++;
        }
    }
    std::cout<<"send done . enter any key to close"<<std::endl;
    int a;
    std::cin>>a;

    return 0;

}