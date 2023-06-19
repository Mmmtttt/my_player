#include "video.h"



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
