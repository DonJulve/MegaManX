#ifndef MEGAMAN_LEVELS_MANAGER_H
#define MEGAMAN_LEVELS_MANAGER_H

#include <string>
#include <stdexcept>
#include "Components.h"
#include "EntityManager.h"

struct MegamanLevelsManagerParameters {
    EntityManager* entityManager;

    MegamanLevelsManagerParameters(EntityManager* em);
};

class MegamanLevelsManager : public Component {
private:
    EntityManager* entityManager;
    SpriteRenderer* spriteRenderer;
    std::string level_to_load;
    std::string level_to_remove;
    bool loading_level;
    float counter; // For fade timing
    float fadeSpeed;
    static MegamanLevelsManager* instance;

public:
    MegamanLevelsManager(MegamanLevelsManagerParameters params);
    ~MegamanLevelsManager();

    static MegamanLevelsManager* getInstance();

    void start() override;
    void update(float deltaTime) override;
    void loadLevel(std::string levelLoad, std::string levelRemove, float fadeSpeed = 255.0f / 0.1f);
};

class MegamanLevelsManagerLoader {
public:
    static MegamanLevelsManagerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager);
    static MegamanLevelsManager createFromJSON(nlohmann::json& j, EntityManager& entityManager);
};

#endif // MEGAMAN_LEVELS_MANAGER_H