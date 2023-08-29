#include "player.h"
#include "my_portocol.h"
#include <QCoreApplication>

SOCKET connect_socket;

SOCKET set_connect(){
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    connect_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connect_socket == INVALID_SOCKET) {
        std::cout << "Error at socket: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 0;
    }

    SOCKADDR_IN clientService;
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr("127.0.0.1");
    clientService.sin_port = htons(12345);

    if (connect(connect_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
        std::cout << "Failed to connect.\n";
        closesocket(connect_socket);
        WSACleanup();
        return 0;
    }
    return connect_socket;
}

SOCKET  set_server(){
    SOCKADDR_IN serverService;
    serverService.sin_family = AF_INET;
    serverService.sin_addr.s_addr = INADDR_ANY;
    serverService.sin_port = htons(12345);

    if (bind(connect_socket, (SOCKADDR*)&serverService, sizeof(serverService)) == SOCKET_ERROR) {
        std::cout << "bind() failed.\n";
        closesocket(connect_socket);
        WSACleanup();
        return 0;
    }

    if (listen(connect_socket, 1) == SOCKET_ERROR) {
        std::cout << "Error listening on socket.\n";
        closesocket(connect_socket);
        WSACleanup();
        return 0;
    }

    SOCKET accept_socket = accept(connect_socket, NULL, NULL);
    if (accept_socket == INVALID_SOCKET) {
        std::cout << "accept() failed: " << WSAGetLastError() << '\n';
        closesocket(connect_socket);
        WSACleanup();
        return accept_socket;
    }
}

int main(int argc, char *argv[])
{
    QApplication  a(argc, argv);



    // 获取传递的参数
    QStringList arguments = a.arguments();
    if (arguments.size() < 2)
    {
        qDebug() << "No argument provided.";
        return 1;
    }

    std::string name = arguments.at(1).toStdString(); // 第一个参数是程序名，第二个参数是传递的字符串

    PLAYER_TYPE type = arguments.at(2)=="LOCAL"?LOCAL:REMOTE;

    if(arguments.at(2)=="SERVER"){
        //type=SERVER_;
        // connect_socket=set_server();
        // Player player(NULL,name,SERVER_,connect_socket);
        // emit player.actionSignal();
    }


    if(type==REMOTE){
        connect_socket=set_connect();
        int ret =Send_Message(connect_socket,name);
        if(ret<=0){std::cout<<"connect failed"<<std::endl;return 0;}
        Player player(NULL,name,REMOTE,connect_socket);
        player.show();

        emit player.actionSignal();
    }
    else{
        Player player(NULL,name,LOCAL,-1);
        player.show();

        emit player.actionSignal();
    }


    // 在这里可以进行你希望在单独进程中执行的操作
    // ...
    //QObject::connect(&player, &Player::exitSignal, &a, &exit); // 连接退出信号

    return a.exec();
}
