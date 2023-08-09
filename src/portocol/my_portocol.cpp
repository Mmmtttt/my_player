
#include "my_portocol.h"

using json = nlohmann::json;

int Send_Message(SOCKET socket, std::string& message){
    int length =message.length();
    int ret=send_all(socket, (const char *)&length,sizeof(length));
    ret=send_all(socket,message.c_str(),length);
    return ret;
}

int Recv_Message(SOCKET socket, std::string& message){
    int length =message.length();
    int ret=recv_all(socket, (char *)&length,sizeof(length));
    char * buff=new char[length];
    ret=recv_all(socket,buff,length);
    message.clear();
    message.assign(buff, length);
    delete []buff;
    return ret;
}

int Send_Filenames(SOCKET clientSocket){
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile("*", &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        json fileList;
        do {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                continue;
            }
            fileList.push_back(findData.cFileName);
        } while (FindNextFile(hFind, &findData) != 0);

        FindClose(hFind);

        // Serialize JSON and send
        std::string jsonStr = fileList.dump();
        // int length=jsonStr.length();
        // send(clientSocket, (const char *)&length,sizeof(length),0);
        // send(clientSocket, jsonStr.c_str(), length, 0);
        return Send_Message(clientSocket,jsonStr);
    }
    
}

int Receive_FileNames(SOCKET clientSocket) {
    
    // int length;
    // recv(clientSocket, (char *)&length, sizeof(length), 0);
    // char * buff=new char[length];
    // recv(clientSocket, (char *)&buff, length, 0);
    std::string message;
    int ret=Recv_Message(clientSocket,message);


    json fileList = json::parse(message);
    for (const auto& fileName : fileList) {
        std::cout << "|| " << fileName << std::endl;
    }

    return ret;
}

int Receive_FileName(SOCKET serverSocket,std::string& name){
    
    // int length;
    // recv(serverSocket, (char *)&length, sizeof(length), 0);
    // char * buff=new char[length];
    // recv(serverSocket, (char *)&buff, length, 0);
    //std::string message;
    int ret=Recv_Message(serverSocket,name);

    
    return ret;
}

int Send_FileName(SOCKET clientSocket,std::string& name){
    std::string fileName;
    std::cin>>fileName;
    //fileName+"\r";
    // int length=fileName.length();
    // send(clientSocket, (char *)&length, sizeof(length), 0);
    // send(clientSocket, fileName.c_str(), length, 0);
    int ret=Send_Message(clientSocket,fileName);
    name=fileName;
    return ret;
}

