#ifndef CONNECT_H
#define CONNECT_H

#include "win_net.h"
#include <vector>
#include <string>
#include <thread>


class Server {
public:
    Server(int port);
    ~Server();
    void listenConnections();
    void handleConnection(Connection* connection);
private:
    int port;
    std::vector<Connection*> connections;
};

class Client {
public:
    Client(std::string serverIp, int serverPort);
    ~Client();
    void startConnection();
private:
    std::string serverIp;
    int serverPort;
};

class Connection {
public:
    Connection(Server* server, SOCKET socket);
    ~Connection();
    void processRequest();
private:
    Server* server;
    SOCKET socket;
};

class SocketWrapper {
public:
    SocketWrapper();
    ~SocketWrapper();
    int sendData(const char* buffer, int len);
    int receiveData(char* buffer, int len);
private:
    SOCKET socket;
};

#endif
