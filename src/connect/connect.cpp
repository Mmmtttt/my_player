#include "connect.h"
#include "transport_session.h"
#include "my_portocol.h"

Server::Server(int port) : port(port) {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != NO_ERROR) {
        throw std::runtime_error("Initialization error\n");
    }
}

Server::~Server() {
    for (Connection* connection : connections) {
        delete connection;
    }
    connections.clear();
    WSACleanup();
}

void Server::listenConnections() {
    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket == INVALID_SOCKET) {
        std::cout << "Error at socket: " << WSAGetLastError() << "\n";
        WSACleanup();
        return;
    }

    SOCKADDR_IN serverService;
    serverService.sin_family = AF_INET;
    serverService.sin_addr.s_addr = INADDR_ANY;
    serverService.sin_port = htons(port);

    if (bind(listen_socket, (SOCKADDR*)&serverService, sizeof(serverService)) == SOCKET_ERROR) {
        std::cout << "bind() failed.\n";
        closesocket(listen_socket);
        WSACleanup();
        return;
    }

    if (listen(listen_socket, 1) == SOCKET_ERROR) {
        std::cout << "Error listening on socket.\n";
        closesocket(listen_socket);
        WSACleanup();
        return;
    }

    while (true) {
        SOCKET accept_socket = accept(listen_socket, NULL, NULL);
        if (accept_socket == INVALID_SOCKET) {
            std::cout << "accept() failed: " << WSAGetLastError() << '\n';
            closesocket(listen_socket);
            WSACleanup();
            return;
        }

        std::cout << "Client connected.\n";

        Connection* connection = new Connection(accept_socket);
        connections.push_back(connection);

        std::thread t(&Connection::processRequest, connection);
        t.detach();
    }
}



Client::Client(std::string serverIp, int serverPort) : serverIp(serverIp), serverPort(serverPort) {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != NO_ERROR) {
        throw std::runtime_error("Initialization error\n");
    }
    
}

Client::~Client() {
    WSACleanup();
}

void Client::startConnection() {
    SOCKET connect_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connect_socket == INVALID_SOCKET) {
        std::cout << "Error at socket: " << WSAGetLastError() << "\n";
        WSACleanup();
        return;
    }

    SOCKADDR_IN clientService;
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(serverIp.c_str());
    clientService.sin_port = htons(serverPort);

    if (connect(connect_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
        std::cout << "Failed to connect.\n";
        closesocket(connect_socket);
        WSACleanup();
        return;
    }

    while(true){
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        int ret=Receive_FileNames(connect_socket);
        std::string name;
        ret *=Send_FileName(connect_socket,name);
        if(ret<=0){std::cout<<"connect failed"<<std::endl;return;}

        try{Session session(name,connect_socket,CLIENT);}
        catch(const std::exception& e){std::cout<<e.what()<<std::endl;}
    }
    closesocket(connect_socket);
}

Connection::Connection(SOCKET socket) : socket(socket) {
}

Connection::~Connection() {
    closesocket(socket);
    std::cout<<"connection destoryed"<<std::endl;
}

void Connection::processRequest() {
    while(true){
        int ret=Send_Filenames(socket);
        std::string name;
        ret *=Receive_FileName(socket,name);
        if(ret<=0){std::cout<<"connect failed"<<std::endl;return;}
        if(name=="q")break;
        try{Session session(name,socket,SERVER);}
        catch(const std::exception& e){std::cout<<e.what()<<std::endl;}
    }
    
}