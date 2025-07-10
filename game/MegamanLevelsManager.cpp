#include "MegamanLevelsManager.h"


MegamanLevelsManager* MegamanLevelsManager::instance = nullptr;

MegamanLevelsManagerParameters::MegamanLevelsManagerParameters(EntityManager* em) : entityManager(em) {}

MegamanLevelsManager::MegamanLevelsManager(MegamanLevelsManagerParameters params) : entityManager(params.entityManager) {
    level_to_load = "";
    level_to_remove = "";
    loading_level = false;
    counter = 0.0f;

    if (instance != nullptr) {
        throw std::runtime_error("MegamanLevelsManager is a singleton and cannot be instantiated more than once");
    }

    instance = this;
}

MegamanLevelsManager::~MegamanLevelsManager() {
    instance = nullptr;
}

MegamanLevelsManager* MegamanLevelsManager::getInstance() {
    if (instance == nullptr) {
        throw std::runtime_error("MegamanLevelsManager not yet created");
    }
    return instance;
}

void MegamanLevelsManager::start() {
    spriteRenderer = entityManager->getComponent<SpriteRenderer>(parent);
    if (!spriteRenderer) {
        throw std::runtime_error("MegamanLevelsManager requires a SpriteRenderer component");
    }
    Debug::Log("MegamanLevelsManager started");
}

void MegamanLevelsManager::update(float deltaTime) {
    if (loading_level) {
        if (spriteRenderer->alpha < 255) {
            counter += fadeSpeed * deltaTime;
            spriteRenderer->alpha = static_cast<int>(counter);
            if (spriteRenderer->alpha >= 255) {
                counter = 255.0f;
                spriteRenderer->alpha = 255;
                entityManager->getSceneManager()->loadSceneAsync(level_to_load);
                entityManager->getSceneManager()->removeScene(level_to_remove);
            }
        }
        else {
            bool isLoading = entityManager->getSceneManager()->isSceneLoading(level_to_load);
            bool isLoaded = entityManager->getSceneManager()->isSceneLoaded(level_to_load);
            if (!isLoading && isLoaded) {
                loading_level = false;
            }
        }
    }
    else if (spriteRenderer->alpha > 0) {
        counter -= fadeSpeed * deltaTime;
        spriteRenderer->alpha = static_cast<int>(counter);
        if (spriteRenderer->alpha <= 0) {
            counter = 0.0f;
            spriteRenderer->alpha = 0;
        }
    }
}

void MegamanLevelsManager::loadLevel(std::string levelLoad, std::string levelRemove, float fadeSpeed) {
    if (loading_level || spriteRenderer->alpha > 0) {
        return;
    }
    level_to_load = levelLoad;
    level_to_remove = levelRemove;
    loading_level = true;
    this->fadeSpeed = fadeSpeed;
}

MegamanLevelsManagerParameters MegamanLevelsManagerLoader::fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
    MegamanLevelsManagerParameters params(&entityManager);
    return params;
}

MegamanLevelsManager MegamanLevelsManagerLoader::createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
    return MegamanLevelsManager(fromJSON(j, entityManager));
}