#ifndef RENDER_TEXT_MANAGER_H
#define RENDER_TEXT_MANAGER_H

#include <SDL2/SDL.h>
#include <string>
#include <unordered_map>
#include <memory>
#include "Components.h"

class RenderTextManager {
private:
    RenderTextManager();
    
    struct FontData {
        std::unordered_map<char, SDL_Texture*> letterTextures;
        int letterWidth;
        int letterHeight;
        
        FontData(int w, int h);
    };
    
    SDL_Renderer* renderer;
    std::unordered_map<std::string, std::unique_ptr<FontData>> fonts;
    static RenderTextManager* instance;

public:
    RenderTextManager(const RenderTextManager&) = delete;
    RenderTextManager& operator=(const RenderTextManager&) = delete;
    
    static RenderTextManager& getInstance();
    
    void initialize(SDL_Renderer* rend);
    bool loadFont(const std::string& fontName, const std::string& fontPath, int width, int height);
    void renderText(const std::string& fontName, const std::string& text, 
                    float x, float y, float xScale, float yScale, EntityManager* em);
    void cleanup();
    
};

#endif // RENDER_TEXT_MANAGER_H