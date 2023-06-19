#include "player.h"
#include "video.h"
#include "decoder.h"
#include "renderer.h"
#include <SDL.h>

Player::Player()
    : video(nullptr), decoder(nullptr), renderer(nullptr),
      playingExit(false), playingPause(false)
{
    initSDL();
}

Player::~Player()
{
    cleanup();
}

bool Player::open(const std::string& filename)
{
    video = new Video();
    decoder = new Decoder();
    renderer = new Renderer();

    if (!video->open(filename)) {
        cleanup();
        return false;
    }

    decoder->open(video);
    renderer->open(video);
    videoStreamIndex = decoder->getVideoStreamIndex();

    return true;
}

void Player::play()
{
    SDL_Event sdlEvent;

    while (!playingExit) {
        handleEvents();

        if (!playingPause) {
            if (decoder->decodeNextFrame()) {
                renderer->renderFrame();
            } else {
                playingExit = true;
            }
        }
        SDL_Delay(40);
    }
}

void Player::pause()
{
    playingPause = !playingPause;
}

void Player::stop()
{
    playingExit = true;
}

void Player::handleEvents()
{
    SDL_Event sdlEvent;

    while (SDL_PollEvent(&sdlEvent)) {
        if (sdlEvent.type == SDL_QUIT) {
            playingExit = true;
        } else if (sdlEvent.type == SDL_KEYDOWN && sdlEvent.key.keysym.sym == SDLK_SPACE) {
            pause();
        }
    }
}

bool Player::initSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        return false;
    }

    return true;
}

void Player::cleanup()
{
    delete renderer;
    delete decoder;
    delete video;

    SDL_Quit();
}
