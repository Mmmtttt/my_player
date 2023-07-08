#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include "video.h"
#include "win_net.h"
#include "connect.h"


#pragma comment(lib, "Ws2_32.lib")

// ... 服务器的其他FFmpeg代码在这里 ...






int main(int argc, char* argv[]) {


    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != NO_ERROR) {
        std::cout << "Initialization error.\n";
        return 1;
    }

    // listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // if (listen_socket == INVALID_SOCKET) {
    //     std::cout << "Error at socket: " << WSAGetLastError() << "\n";
    //     WSACleanup();
    //     return 1;
    // }

    // serverService;
    // serverService.sin_family = AF_INET;
    // serverService.sin_addr.s_addr = INADDR_ANY;
    // serverService.sin_port = htons(12345);  // 选择一个端口

    // if (bind(listen_socket, (SOCKADDR*)&serverService, sizeof(serverService)) == SOCKET_ERROR) {
    //     std::cout << "bind() failed.\n";
    //     closesocket(listen_socket);
    //     WSACleanup();
    //     return 1;
    // }

    // if (listen(listen_socket, 1) == SOCKET_ERROR) {
    //     std::cout << "Error listening on socket.\n";
    //     closesocket(listen_socket);
    //     WSACleanup();
    //     return 1;
    // }

    // accept_socket = accept(listen_socket, NULL, NULL);
    // if (accept_socket == INVALID_SOCKET) {
    //     std::cout << "accept() failed: " << WSAGetLastError() << '\n';
    //     closesocket(listen_socket);
    //     WSACleanup();
    //     return 1;
    // }


    // Server server(12345);
    // server.listenConnections();

    // std::cout << "Client connected.\n";

    // // ... 发送数据到客户端 ...

    // std::string filename;

    // if (argc < 2) {
    //     std::cin>>filename;
    // }
    // else{filename=argv[1];}
    // Video video(filename);

    // video.v_decoder->push_All_Packets(video.p_fmt_ctx);

    
    // //std::cout<<"prepare ok"<<std::endl;
    // SEND_ALL(video.v_idx);
    // SEND_ALL(*video.v_p_codec_par);
    // send_all(server_socket,(const char *)video.v_p_codec_par->extradata,video.v_p_codec_par->extradata_size);
    // SEND_ALL(video.v_timebase_in_ms);

    // SEND_ALL(video.a_idx);
    // SEND_ALL(*video.a_p_codec_par);
    // send_all(server_socket,(const char *)video.a_p_codec_par->extradata,video.a_p_codec_par->extradata_size);
    // SEND_ALL(video.a_timebase_in_ms);

    // int64_t v_size=video_packet_queue.get_pkt_count(),a_size=audio_packet_queue.get_pkt_count();
    // SEND_ALL(v_size);
    // SEND_ALL(a_size);


    // video.~Video();

    // auto vq=&video_packet_queue;  //调试用到
    // auto aq=&audio_packet_queue;
    // video_packet_queue.set_curr_pos(0);
    // audio_packet_queue.set_curr_pos(0);

    // while(video_packet_queue.get_curr_pos()+audio_packet_queue.get_curr_pos()<v_size+a_size-2){

    //     if((video_packet_queue.get_curr_num())<=(audio_packet_queue.get_curr_num())){
    //         std::unique_lock<std::mutex>(video_packet_queue.Mutex);
    //         SEND_ALL(video_packet_queue.get_curr_pkt()->size);
    //         SEND_ALL(*video_packet_queue.get_curr_pkt());

    //         video_packet_queue.curr_decode_pos++;
    //     }
    //     else{
    //         std::unique_lock<std::mutex>(audio_packet_queue.Mutex);
    //         SEND_ALL(audio_packet_queue.get_curr_pkt()->size);
    //         SEND_ALL(*audio_packet_queue.get_curr_pkt());

    //         audio_packet_queue.curr_decode_pos++;
    //     }
        

    // }
    

    // video_packet_queue.set_curr_pos(0);
    // audio_packet_queue.set_curr_pos(0);

    // std::thread t0([&]{
    //     while(1){
    //         seek_handle();
    //         //std::this_thread::sleep_for()
    //     }
    // });
    

    // while(1){
    //     while(video_packet_queue.get_curr_pos()+audio_packet_queue.get_curr_pos()<v_size+a_size-2){
    //         //SDL_Delay(30);
    //         std::unique_lock<std::mutex> vlock(video_packet_queue.Mutex);
    //         std::unique_lock<std::mutex> alock(audio_packet_queue.Mutex);
    //         if((video_packet_queue.get_curr_num())<=(audio_packet_queue.get_curr_num())){
    //             if(video_packet_queue.get_curr_pos()>=v_size)break;
    //             std::shared_ptr<myAVPacket> temp=video_packet_queue.get_curr_pkt();
    //             if(temp->is_sended)continue;
                
    //             SEND_ALL(temp->size);
    //             SEND_ALL(*temp);
                
    //             send_all(server_socket,(const char *)temp->mypkt.data,temp->size);
                
    //             temp->is_sended=true;
    //             video_packet_queue.curr_decode_pos++;
    //             if(video_packet_queue.curr_decode_pos>=v_size)video_packet_queue.set_curr_pos(v_size-1);
    //         }
    //         else{
    //             if(audio_packet_queue.get_curr_pos()>=a_size)break;
    //             std::shared_ptr<myAVPacket> temp=audio_packet_queue.get_curr_pkt();
    //             if(temp->is_sended)continue;

    //             SEND_ALL(temp->size);
    //             SEND_ALL(*temp);
                
    //             send_all(server_socket,(const char *)temp->mypkt.data,temp->size);
    //             temp->is_sended=true;
    //             audio_packet_queue.curr_decode_pos++;
    //             if(audio_packet_queue.curr_decode_pos>=a_size)audio_packet_queue.set_curr_pos(a_size-1);
    //         }
    //     }
    //     std::cout<<"send done . enter any key to close"<<std::endl;
    // }

    Server server(12345);
    server.listenConnections();

    
    int a;
    std::cin>>a;

    return 0;

}