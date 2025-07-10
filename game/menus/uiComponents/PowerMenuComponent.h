#include "Components.h"
#include "menus/uiComponents/FullBarMenuComponent.h"
#pragma once

struct PowerMenuComponentParameters
{

    SpriteRenderer *renderPowerMenu;
    EntityManager *entityManager;
    Transform *transform;
    PowerMenuComponentParameters(SpriteRenderer *sp, EntityManager *m, Transform *t) : renderPowerMenu(sp), entityManager(m), transform(t) {}
};

struct PowerMenuComponent : public Component
{

    SpriteRenderer *renderPowerMenu;
    EntityManager *entityManager;
    FullBarMenuComponent *barMenuComponent;
    Transform *barTransform;
    Transform *transform;

    float movX = 0.064f;
    float movY = 0.035f;

    PowerMenuComponent(PowerMenuComponentParameters params) : renderPowerMenu(params.renderPowerMenu), entityManager(params.entityManager), transform(params.transform) {}

    void actualizarPower(int power, int availablePower, int levelPower, int maxLevelPower)
    {
        std::string ruta;
        if (availablePower == 0)
        {
            return;
        }
        if (availablePower == 1)
        {
            ruta = "resources/sprites/pauseMenu/powerUpsActivated/powerUpsActivated_" + std::to_string(power) + ".png";
            renderPowerMenu->texture = TextureManager::getInstance()->getTexture(ruta);
        }
        else if (availablePower == 2)
        {
            ruta = "resources/sprites/pauseMenu/powerUpsDisabled/powerUpsDisabled_" + std::to_string(power) + ".png";
            renderPowerMenu->texture = TextureManager::getInstance()->getTexture(ruta);
        }
        if (!PrefabManager::getInstance().hasPrefab("lifeBarMenu"))
        {
            PrefabManager::getInstance().loadPrefab("lifeBarMenu");
        }
        Entity entity = PrefabManager::getInstance().instantiatePrefab("lifeBarMenu", *entityManager);
        barMenuComponent = entityManager->getComponent<FullBarMenuComponent>(entity);
        barTransform = entityManager->getComponent<Transform>(entity);
        barTransform->posX = transform->posX + movX;
        barTransform->posY = transform->posY + movY;
        barMenuComponent->actualizarBarraVida(availablePower, levelPower, maxLevelPower);
        barMenuComponent->start();
    }
    void start() override
    {
    }

    void update(float deltaTime) override
    {
    }
};

class PowerMenuComponentLoader
{
public:
    static PowerMenuComponentParameters fromJSON(const nlohmann::json &j, EntityManager &entityManager)
    {

        SpriteRenderer *renderPowerMenu = entityManager.getComponent<SpriteRenderer>(entityManager.getLast());
        if (!renderPowerMenu)
        {
            throw std::runtime_error("PowerMenuComponent requires a SpriteRenderer component");
        }

        Transform *transform = entityManager.getComponent<Transform>(entityManager.getLast());
        if (!transform)
        {
            throw std::runtime_error("PowerMenuComponent requires a Transform component");
        }
        PowerMenuComponentParameters params(renderPowerMenu, &entityManager, transform);

        return params;
    }

    // Helper function to directly create a Camera component from JSON
    static PowerMenuComponent createFromJSON(nlohmann::json &j, EntityManager &entityManager)
    {
        return PowerMenuComponent(fromJSON(j, entityManager));
    }
};