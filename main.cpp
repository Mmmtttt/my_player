#include "video.h"
#include <winsock2.h>
#include <ws2tcpip.h>

std::chrono::_V2::system_clock::time_point start;
int64_t time_shaft = 0;
int64_t a_last_time = 0;
int64_t v_last_time = 0;
double speed = 1.0;
bool s_playing_pause = false;
bool s_playing_exit = false;
int64_t s_audio_play_time = 0;
int64_t s_video_play_time = 0;

SOCKET listen_socket;
sockaddr_in serverService;
SOCKET accept_socket;
SOCKET client_socket;
sockaddr_in clientService;


std::vector<std::pair<int,int64_t>> num_mapping_id_in_queue;

int main(int argc, char* argv[])
{
    std::string filename;

    if (argc < 2) {
        std::cin>>filename;
    }
    else{filename=argv[1];}
    Video video(filename);
    
    video.v_decoder->push_All_Packets(video.p_fmt_ctx);
    auto aq=&audio_packet_queue;
    auto vq=&video_packet_queue;
    video.play();


    return 0;
}
