#include "Components.h"

SpriteRendererParameters::SpriteRendererParameters(SDL_Texture* tex, AnchorPoint center, Transform* t, int alpha)
    : texture(tex), centerMode(center), transform(t), alpha(alpha) {}

SpriteRenderer::SpriteRenderer(SpriteRendererParameters srp)
    : texture(srp.texture), centerMode(srp.centerMode), transform(srp.transform), alpha(srp.alpha) {
    if (!texture) {
        width = height = 0;
    } else {
        SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
    }
    worldBounds = {0, 0, width, height};
    flip = SDL_FLIP_NONE;
}

SpriteRenderer::SpriteRenderer()
    : texture(nullptr), centerMode(AnchorPoint::CENTER) {}

void SpriteRenderer::start() {}

void SpriteRenderer::update(float deltaTime) {
    if (transform) {
        updateBounds();
    }
}

void SpriteRenderer::updateBounds() {
    worldBounds.w = static_cast<int>(width * transform->sizeX);
    worldBounds.h = static_cast<int>(height * transform->sizeY);
    worldBounds.x = static_cast<int>(transform->posX);
    worldBounds.y = static_cast<int>(transform->posY);
    if (centerMode == AnchorPoint::CENTER) {
        worldBounds.x -= worldBounds.w / 2;
        worldBounds.y -= worldBounds.h / 2;
    }
}

TextureManager* TextureManager::instance = nullptr;

TextureManager::TextureManager(SDL_Renderer* r) : renderer(r) {}

TextureManager::~TextureManager() {
    for (auto& [path, texture] : textureCache) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
}

TextureManager* TextureManager::getInstance(SDL_Renderer* renderer) {
    if (!instance) {
        instance = new TextureManager(renderer);
    }
    return instance;
}

TextureManager* TextureManager::getInstance() {
    if (!instance) {
        throw std::runtime_error("TextureManager no inicializado");
    }
    return instance;
}

SDL_Texture* TextureManager::loadBMP(std::string& filePath) {
    std::string fullPath = filePath;
    SDL_Surface* tempSurface = SDL_LoadBMP(fullPath.c_str());
    if (!tempSurface) {
        Debug::Log("Error al cargar textura: " + std::string(SDL_GetError()));
        throw std::runtime_error("Error al cargar textura: " + std::string(SDL_GetError()));
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    if (!texture) {
        Debug::Log("Error al crear textura: " + std::string(SDL_GetError()));
        throw std::runtime_error("Error al cargar textura: " + std::string(SDL_GetError()));
        return nullptr;
    }
    return texture;
}

SDL_Texture* TextureManager::loadPNG(std::string& filePath) {
    std::string fullPath = filePath;
    SDL_Texture* texture = IMG_LoadTexture(renderer, fullPath.c_str());
    if (!texture) {
        Debug::Log("Error al cargar textura: " + std::string(IMG_GetError()));
        throw std::runtime_error("Error al cargar textura: " + std::string(IMG_GetError()));
        return nullptr;
    }
    return texture;
}

SDL_Texture* TextureManager::getTexture(std::string& path) {
    auto it = textureCache.find(path);
    if (it != textureCache.end()) {
        return it->second;
    }
    SDL_Texture* texture = nullptr;
    std::string extension = path.substr(path.find_last_of(".") + 1);
    if (extension == "bmp") {
        texture = loadBMP(path);
    } else if (extension == "png") {
        texture = loadPNG(path);
    }
    if (texture) {
        textureCache[path] = texture;
    }
    return texture;
}

TextureManager* SpriteRendererLoader::textureManager = nullptr;

void SpriteRendererLoader::initialize(SDL_Renderer* renderer) {
    if (!textureManager) {
        textureManager = TextureManager::getInstance(renderer);
    }
}

void SpriteRendererLoader::cleanup() {
    delete textureManager;
    textureManager = nullptr;
}

SpriteRendererParameters SpriteRendererLoader::fromJSON(const nlohmann::json& j, const EntityManager& entityManager) {
    if (!textureManager) {
        throw std::runtime_error("TextureManager no inicializado");
    }
    std::string texturePath = "resources/" + j.value("texturePath", "");
    if (texturePath.empty()) {
        throw std::runtime_error("Ruta de textura no especificada en JSON");
    }
    SDL_Texture* texture = textureManager->getTexture(texturePath);
    if (!texture) {
        throw std::runtime_error("No se pudo cargar la textura: " + texturePath);
    }
    Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
    if (!transform) {
        throw std::runtime_error("SpriteRenderer requiere un componente Transform");
    }
    AnchorPoint centerMode = j.value("anchor", AnchorPoint::CENTER);

    int alpha = j.value("alpha", 255);
    if (alpha < 0 || alpha > 255) {
        throw std::runtime_error("Valor de alpha fuera de rango (0-255)");
    }

    return SpriteRendererParameters(texture, centerMode, transform, alpha);
}

SpriteRenderer SpriteRendererLoader::createFromJSON(const nlohmann::json& j, const EntityManager& entityManager) {
    return SpriteRenderer(fromJSON(j, entityManager));
}