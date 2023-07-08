#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include "video.h"
#include "win_net.h"
#include "connect.h"


#pragma comment(lib, "Ws2_32.lib")

// ... 服务器的其他FFmpeg代码在这里 ...




SOCKET listen_socket;
sockaddr_in serverService;
SOCKET accept_socket;
SOCKET client_socket;
sockaddr_in clientService;

std::vector<std::pair<int,int64_t>> num_mapping_id_in_queue;

void seek_handle(Video*);

int main(int argc, char* argv[]) {


    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != NO_ERROR) {
        std::cout << "Initialization error.\n";
        return 1;
    }

    
    Server server(12345);
    server.listenConnections();

    
    int a;
    std::cin>>a;

    return 0;
}