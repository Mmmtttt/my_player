#ifndef MY_PORTCOL
#define MY_PORTCOL

#include "json.hpp"
#include <winsock2.h>
#include <windows.h>
#include "win_net.h"

using json = nlohmann::json;

int Send_Message(SOCKET socket, std::string& message);

int Recv_Message(SOCKET socket, std::string& message);

int Send_Filenames(SOCKET clientSocket);

int Receive_FileNames(SOCKET clientSocket);

int Receive_FileName(SOCKET serverSocket,std::string& name);

int Send_FileName(SOCKET clientSocket,std::string& name);



#endif