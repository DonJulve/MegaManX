#ifndef GAME_H
#define GAME_H

class Game
{
public:
    // Constructor: Initializes the engine with a title, width, and height
    Game(const char *title, int width, int height);

    // Registers components with the SceneLoader
    void registro();

    // Runs the game loop
    void run();
};

#endif