#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include "Game.h"
#include "Engine.h"
#include <iostream>

#ifdef _WIN32
#define MAIN_FUNCTION SDL_main
#else
#define MAIN_FUNCTION main
#endif

int MAIN_FUNCTION(int argc, char* argv[]) {
    Game game("MegamanX", 800, 600);
    game.run();

    std::cout << "Game finished." << std::endl;

    return 0;
}