#include "player.h"
#include <QCoreApplication>


SOCKET set_connect(std::string IP){
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET connect_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connect_socket == INVALID_SOCKET) {
        std::cout << "Error at socket: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 0;
    }

    SOCKADDR_IN clientService;
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(IP.c_str());
    clientService.sin_port = htons(12346);

    if (connect(connect_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
        std::cout << "Failed to connect.\n";
        closesocket(connect_socket);
        WSACleanup();
        return 0;
    }
    return connect_socket;
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


    if(type==REMOTE){
        SOCKET connect_socket=set_connect(arguments.at(3).toStdString());
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
