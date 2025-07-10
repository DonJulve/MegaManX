#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#pragma once

struct BossMenuComponentParameters
{

    AnimatorComponent *animator;
    EntityManager *entityManager;
    BossMenuComponentParameters(AnimatorComponent *ac, EntityManager *em) : animator(ac), entityManager(em) {}
};

struct BossMenuComponent : public Component
{

    AnimatorComponent *animator;
    EntityManager *entityManager;

    float counter = 0.0f;

    static int BossSeleccionado;
    std::shared_ptr<FlammeMenuComponent> flammeMenu;

    SpriteRenderer *renderFlammeMenu;

    std::string nombreBoss[8] = {
        "launch octopus",
        "chill penguin",
        "armored armadillo",
        "flame mammoth",
        "storm eagle",
        "boomer kuwanger",
        "spark mandrill",
        "Sting chameleon"};

    std::string levelBoss[8] = {
        "",
        "levels/penguin",
        "",
        "levels/mammoth",
        "",
        "",
        "",
        ""};

    BossMenuComponent(BossMenuComponentParameters params) : animator(params.animator), entityManager(params.entityManager)
    {
    }

    void start() override
    {
        RenderTextManager::getInstance().loadFont("unselectedFont", "sprites/letters/lettersUnselected", 7, 7);
        entityManager->findComponent<FlammeMenuComponent>(flammeMenu);
        renderFlammeMenu = entityManager->getComponent<SpriteRenderer>(flammeMenu->parent);
        if (BossSeleccionado == 3)
        {
            renderFlammeMenu->alpha = 255;
            animator->spriteRenderer->alpha = 0;
        }
        else if (BossSeleccionado == 1)
        {
            renderFlammeMenu->alpha = 0;
            animator->spriteRenderer->alpha = 255;
        }
    }

    void update(float deltaTime) override
    {
        counter += deltaTime;

        if (counter > 3.0)
        {
            MegamanLevelsManager::getInstance()->loadLevel(levelBoss[BossSeleccionado], entityManager->getSceneManager()->getSceneNameByEntityManager(entityManager));
        }
    }
    void render(EntityManager &entityManager, SDL_Renderer *renderer) override
    {
        RenderTextManager::getInstance().renderText("unselectedFont", nombreBoss[BossSeleccionado], 0.3f, 0.75f, 4, 4, &entityManager);
    };
};
int BossMenuComponent::BossSeleccionado;

class BossMenuComponentLoader
{
public:
    static BossMenuComponentParameters fromJSON(const nlohmann::json &j, EntityManager &entityManager)
    {

        AnimatorComponent *animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
        if (!animator)
        {
            throw std::runtime_error("BossMenuComponent requires a AnimatorComponent component");
        }
        BossMenuComponentParameters params(animator, &entityManager);

        return params;
    }

    // Helper function to directly create a Camera component from JSON
    static BossMenuComponent createFromJSON(nlohmann::json &j, EntityManager &entityManager)
    {
        return BossMenuComponent(fromJSON(j, entityManager));
    }
};