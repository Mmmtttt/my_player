#ifndef CONNECT_H
#define CONNECT_H

#include "win_net.h"
#include <vector>
#include <string>
#include <thread>

class Connection;
class Server {
public:
    Server(int port);
    ~Server();
    void listenConnections();
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
    Connection(SOCKET socket);
    ~Connection();
    void processRequest();
private:
    SOCKET socket;
};

// class SocketWrapper {
// public:
//     SocketWrapper();
//     ~SocketWrapper();
//     int sendData(const char* buffer, int len);
//     int receiveData(char* buffer, int len);
// private:
//     SOCKET socket;
// };

#endif
