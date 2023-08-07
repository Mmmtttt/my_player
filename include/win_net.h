#include <winsock2.h>
#include "control.h"
#define SEND(A) send(server_socket,(const char *)&A,sizeof(A),0)
#define RECV(A) recv(client_socket,(char *)&A,sizeof(A),0)

#define SEND_ALL(A) send_all(server_socket, (const char*)&A, sizeof(A))
#define RECV_ALL(A) recv_all(client_socket, (char*)&A, sizeof(A))

int send_all(int socket, const char* buffer, size_t length);

int recv_all(int socket, char* buffer, size_t length);

extern SOCKET client_socket;
// extern sockaddr_in clientService;

// extern SOCKET listen_socket;
// extern sockaddr_in serverService;
// extern SOCKET server_socket;