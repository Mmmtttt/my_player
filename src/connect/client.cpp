#include "video.h"
#include "win_net.h"
#include "connect.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <functional>
#include <thread>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")



int main(int argc, char* argv[]) {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != NO_ERROR) {
        std::cout << "Initialization error.\n";
        return 1;
    }
    std::string IP;
    std::cin>>IP;

    int port;
    std::cin>>port;

    Client client(IP,port);
    client.startConnection();

 
    return 0;
}