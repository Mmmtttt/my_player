#include "transport_session.h"

SOCKET connect_socket;

SOCKET  set_server(){
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKADDR_IN serverService;
    serverService.sin_family = AF_INET;
    serverService.sin_addr.s_addr = INADDR_ANY;
    serverService.sin_port = htons(12346);

    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket == INVALID_SOCKET) {
        std::cout << "Error at socket: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 0;
    }

    if (bind(listen_socket, (SOCKADDR*)&serverService, sizeof(serverService)) == SOCKET_ERROR) {
        std::cout << "bind() failed.\n";
        closesocket(listen_socket);
        WSACleanup();
        return 0;
    }

    if (listen(listen_socket, 1) == SOCKET_ERROR) {
        std::cout << "Error listening on socket.\n";
        closesocket(listen_socket);
        WSACleanup();
        return 0;
    }

    connect_socket = accept(listen_socket, NULL, NULL);
    if (connect_socket == INVALID_SOCKET) {
        std::cout << "accept() failed: " << WSAGetLastError() << '\n';
        closesocket(connect_socket);
        WSACleanup();
        return connect_socket;
    }
}


int main(int argc, char *argv[])
{


    // 获取传递的参数
    if (argc < 2)
    {
        return 1;
    }

    std::string name = argv[1];

    connect_socket=set_server();


    Session session(name,connect_socket,SERVER);
    exit(0);
}
