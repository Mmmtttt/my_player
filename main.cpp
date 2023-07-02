#include "video.h"

std::chrono::_V2::system_clock::time_point start;
int64_t time_shaft = 0;
int64_t last_time = 0;
double speed = 2.0;
bool s_playing_pause = false;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Please provide a movie file" << std::endl;
        return -1;
    }

    Video video(argv[1]);
    

    video.play();

    return 0;
}
