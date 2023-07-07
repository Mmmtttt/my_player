#include "win_net.h"


int send_all(int socket, const char* buffer, size_t length) {
    size_t pos = 0;
    while (pos < length) {
        ssize_t ret = send(socket, buffer + pos, length - pos, 0);
        if (ret == -1) {
            // handle error
            return -1;
        }
        pos += ret;
    }
    return pos;
}

int recv_all(int socket, char* buffer, size_t length) {
    size_t pos = 0;
    while (pos < length) {
        ssize_t ret = recv(socket, buffer + pos, length - pos, 0);
        if (ret == -1) {
            // handle error
            return -1;
        } else if (ret == 0) {
            // the other side has closed the connection
            break;
        }
        pos += ret;
    }
    return pos;
}
