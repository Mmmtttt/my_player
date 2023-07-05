#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include "video.h"
#include "win_net.h"


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

    std::string filename;

    if (argc < 2) {
        std::cin>>filename;
    }
    else{filename=argv[1];}
    Video video(filename);

    video.v_decoder->push_All_Packets(video.p_fmt_ctx);

    
    std::cout<<"prepare ok"<<std::endl;
    // send(accept_socket, (const char *)&video.v_idx,sizeof(video.v_idx),0);
    // send(accept_socket, (const char *)video.a_p_codec_par, sizeof(*video.a_p_codec_par), 0);
    SEND_ALL(video.v_idx);
    SEND_ALL(*video.v_p_codec_par);
    send_all(accept_socket,(const char *)video.v_p_codec_par->extradata,video.v_p_codec_par->extradata_size);
    SEND_ALL(video.v_timebase_in_ms);

    SEND_ALL(video.a_idx);
    SEND_ALL(*video.a_p_codec_par);
    send_all(accept_socket,(const char *)video.a_p_codec_par->extradata,video.a_p_codec_par->extradata_size);
    SEND_ALL(video.a_timebase_in_ms);

    int64_t v_size=video_packet_queue.pkts_ptr.size(),a_size=audio_packet_queue.pkts_ptr.size();
    SEND_ALL(v_size);
    SEND_ALL(a_size);

    int64_t aaa=123456789;
    SEND_ALL(aaa);

    video.~Video();

    // auto vq=&video_packet_queue;  //调试用到
    // auto aq=&audio_packet_queue;

    while(video_packet_queue.curr_decode_pos<video_packet_queue.pkts_ptr.size()&&audio_packet_queue.curr_decode_pos<audio_packet_queue.pkts_ptr.size()){
        SDL_Delay(50);
        if((video_packet_queue.pkts_ptr[video_packet_queue.curr_decode_pos]->num)<=(audio_packet_queue.pkts_ptr[audio_packet_queue.curr_decode_pos]->num)){
            SEND_ALL(video_packet_queue.pkts_ptr[video_packet_queue.curr_decode_pos]->size);
            SEND_ALL(*video_packet_queue.pkts_ptr[video_packet_queue.curr_decode_pos]);
            
            send_all(accept_socket,(const char *)video_packet_queue.pkts_ptr[video_packet_queue.curr_decode_pos]->mypkt.data,video_packet_queue.pkts_ptr[video_packet_queue.curr_decode_pos]->size);
            video_packet_queue.curr_decode_pos++;
        }
        else{
            SEND_ALL(audio_packet_queue.pkts_ptr[audio_packet_queue.curr_decode_pos]->size);
            SEND_ALL(*audio_packet_queue.pkts_ptr[audio_packet_queue.curr_decode_pos]);
            
            send_all(accept_socket,(const char *)audio_packet_queue.pkts_ptr[audio_packet_queue.curr_decode_pos]->mypkt.data,audio_packet_queue.pkts_ptr[audio_packet_queue.curr_decode_pos]->size);
            audio_packet_queue.curr_decode_pos++;
        }
    }
    std::cout<<"send done . enter any key to close"<<std::endl;
    int a;
    std::cin>>a;

    return 0;

}
