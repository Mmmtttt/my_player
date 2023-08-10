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



    Server server(12345);
    server.listenConnections();

    
    int a;
    std::cin>>a;

    return 0;

}