#include "RenderTextManager.h"
#include <filesystem>

RenderTextManager* RenderTextManager::instance = nullptr;

// FontData constructor
RenderTextManager::FontData::FontData(int w, int h) 
    : letterWidth(w), letterHeight(h) {
    Debug::Log("FontData constructed with width: " + std::to_string(w) + ", height: " + std::to_string(h));
}

// Private constructor
RenderTextManager::RenderTextManager() 
    : renderer(nullptr) {
    Debug::Log("RenderTextManager instance created");
}

RenderTextManager& RenderTextManager::getInstance() {
    if (!instance) {
        instance = new RenderTextManager();
    }
    return *instance;
}

void RenderTextManager::initialize(SDL_Renderer* rend) {
    renderer = rend;
    Debug::Log("RenderTextManager initialized with renderer: " + std::to_string(reinterpret_cast<uintptr_t>(rend)));
}

bool RenderTextManager::loadFont(const std::string& fontName, const std::string& fontPath, int width, int height) {
    if (!renderer) {
        Debug::Log("loadFont failed: renderer is null");
        return false;
    }
    
    Debug::Log("Loading font: " + fontName + " from path: " + fontPath + " with size " + std::to_string(width) + "x" + std::to_string(height));
    auto fontData = std::make_unique<FontData>(width, height);
    TextureManager* textureManager = TextureManager::getInstance();

    std::string resPath = "resources/" + fontPath;
    Debug::Log("Resource path set to: " + resPath);
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(resPath)) {
            std::string filename = entry.path().stem().string();
            std::string extension = entry.path().extension().string();
            Debug::Log("Checking file: " + entry.path().string());
            
            if (filename.length() == 1) {
                char c = filename[0];
                std::string fullPath = entry.path().string();
                Debug::Log("Found valid character file: " + fullPath + " for char: " + c);
                
                SDL_Texture* texture = textureManager->getTexture(fullPath);
                if (!texture) {
                    printf("Failed to load texture for %s\n", fullPath.c_str());
                    Debug::Log("Texture load failed for: " + fullPath);
                    continue;
                }
                Debug::Log("Texture loaded successfully for char: " + std::string(1, c));
                
                fontData->letterTextures[c] = texture;
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        printf("Error reading font directory %s: %s\n", resPath.c_str(), e.what());
        Debug::Log("Filesystem error: " + std::string(e.what()) + " for directory: " + resPath);
        return false;
    }
    
    if (!fontData->letterTextures.empty()) {
        fonts[fontName] = std::move(fontData);
        Debug::Log("Font " + fontName + " loaded with " + std::to_string(fonts[fontName]->letterTextures.size()) + " characters");
        return true;
    }
    Debug::Log("Font " + fontName + " failed to load: no valid characters found");
    return false;
}

void RenderTextManager::renderText(const std::string& fontName, const std::string& text, 
                                   float x, float y, float xScale, float yScale, EntityManager* em) {
    
    auto it = fonts.find(fontName);
    if (it == fonts.end()) {
        Debug::Log("Font not found: " + fontName);
        return;
    }

    Camera* camera = nullptr;
    for (Entity entity : em->getEntities()) {
        camera = em->getComponent<Camera>(entity);
        if (camera) break;
    }

    if (!camera) return;
    
    FontData* font = it->second.get();

    //int windowWidth, windowHeight;
    //SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);
    
    // Convert normalized coordinates (0.0 to 1.0) to pixel coordinates
    int pixelX = static_cast<int>(x * camera->windowWidth) + camera->excessWidth / 2;
    int pixelY = static_cast<int>(y * camera->windowHeight) + camera->excessHeight / 2;
    int currentX = pixelX;
    
    // Scale dimensions relative to window size
    int scaledWidth = static_cast<int>(font->letterWidth * xScale * (camera->windowWidth / 800.0f));
    int scaledHeight = static_cast<int>(font->letterHeight * yScale * (camera->windowHeight / 600.0f));
    
    for (char c : text) {
        auto texIt = font->letterTextures.find(c);
        if (texIt != font->letterTextures.end()) {
            SDL_Rect dstRect = {currentX, pixelY, scaledWidth, scaledHeight};
            SDL_RenderCopy(renderer, texIt->second, nullptr, &dstRect);
            currentX += scaledWidth;
        } else {
            currentX += scaledWidth;
        }
    }
}

void RenderTextManager::cleanup() {
    Debug::Log("Cleaning up RenderTextManager: clearing " + std::to_string(fonts.size()) + " fonts");
    fonts.clear();
    delete instance;
    instance = nullptr;
    Debug::Log("RenderTextManager instance deleted");
}