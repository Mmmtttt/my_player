#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include "video.h"
#include "decoder.h"
#include "renderer.h"

class Player {
public:
    Player();
    ~Player();

    bool open(const std::string& filename);
    void play();
    void pause();
    void stop();
    void handleEvents();

private:
    bool initSDL();
    void cleanup();

    Video* video;
    Decoder* decoder;
    Renderer* renderer;
    bool playingExit;
    bool playingPause;
    int videoStreamIndex;
};

#endif
