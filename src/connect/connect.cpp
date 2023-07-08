#include "connect.h"
#include "transport_session.h"

Server::Server(int port) : port(port) {
}

Server::~Server() {
    for (Connection* connection : connections) {
        delete connection;
    }
    connections.clear();
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
}

Client::~Client() {
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

    Session session("1.mp4",connect_socket,CLIENT);
}

Connection::Connection(SOCKET socket) : socket(socket) {
}

Connection::~Connection() {
    closesocket(socket);
}

void Connection::processRequest() {
    Session session("1.mp4",socket,SERVER);
}
