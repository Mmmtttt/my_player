#include <winsock2.h>
#include "control.h"


int send_all(int socket, const char* buffer, size_t length);

int recv_all(int socket, char* buffer, size_t length);