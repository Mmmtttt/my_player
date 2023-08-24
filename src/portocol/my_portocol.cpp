
#include "my_portocol.h"

using json = nlohmann::json;

int Send_Message(SOCKET socket, std::string& message){
    int length =message.length();
    int ret=send_all(socket, (const char *)&length,sizeof(length));
    if(ret<=0){std::cout<<WSAGetLastError()<<std::endl;}
    ret=send_all(socket,message.c_str(),length);
    return ret;
}

int Recv_Message(SOCKET socket, std::string& message){
    int length =message.length();
    int ret=recv_all(socket, (char *)&length,sizeof(length));
    if(ret<=0){std::cout<<WSAGetLastError()<<std::endl;}
    char * buff=new char[length];
    ret=recv_all(socket,buff,length);
    message.clear();
    message.assign(buff, length);
    delete []buff;
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

    int ret=Send_Message(clientSocket,fileName);
    name=fileName;
    return ret;
}

void Send_side_data(SOCKET socket,AVPacketSideData* sidedata){
    send_all(socket,(const char*)&sidedata->type,sizeof(sidedata->type));
    send_all(socket,(const char*)&sidedata->size,sizeof(sidedata->size));
    send_all(socket,(const char*)sidedata->data,sidedata->size);
}

void Recv_side_data(SOCKET socket,AVPacket *pkt){
    enum AVPacketSideDataType type;
    int size;
    recv_all(socket,(char*)&type,sizeof(type));
    recv_all(socket,(char*)&size,sizeof(size));
    //uint8_t *dst_data = av_packet_new_side_data(pkt, type, size);

    int elems = pkt->side_data_elems;

    if ((unsigned)elems + 1 > INT_MAX / sizeof(*pkt->side_data))
        return ;
    if ((unsigned)size > INT_MAX - AV_INPUT_BUFFER_PADDING_SIZE)
        return ;

    pkt->side_data = (AVPacketSideData *)av_realloc(pkt->side_data,
                                (elems + 1) * sizeof(*pkt->side_data));
    if (!pkt->side_data)
        return ;

    pkt->side_data[elems].data = (uint8_t *)av_mallocz(size + AV_INPUT_BUFFER_PADDING_SIZE);
    if (!pkt->side_data[elems].data)
        return ;
    pkt->side_data[elems].size = size;
    pkt->side_data[elems].type = type;
    pkt->side_data_elems++;

    uint8_t *dst_data= pkt->side_data[elems].data;


    recv_all(socket,(char*)&dst_data,size);
}

void Send_side_datas(SOCKET socket,AVPacket *pkt){
    send_all(socket,(const char*)&pkt->side_data_elems,sizeof(pkt->side_data_elems));
    for (int i = 0; i < pkt->side_data_elems; i++) {
        Send_side_data(socket,&pkt->side_data[i]);
    }
}

void Recv_side_datas(SOCKET socket,AVPacket *pkt){
    int elems ;
    recv_all(socket,(char*)&elems,sizeof(elems));
    for (int i = 0; i < elems; i++) {
        Recv_side_data(socket,pkt);
    }
}






