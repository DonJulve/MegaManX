#include "Components.h"
#include "RenderTextManager.h"
#include "Debug.h"
#include "InputManager.h"
#include "AnimatorComponent.h"
#pragma once

struct FlammeMenuComponentParameters
{

    AnimatorComponent *animator;
    FlammeMenuComponentParameters(AnimatorComponent *ac) : animator(ac) {}
};

struct FlammeMenuComponent : public Component
{

    AnimatorComponent *animator;

    FlammeMenuComponent(FlammeMenuComponentParameters params) : animator(params.animator) {}

    void start() override
    {
    }

    void update(float deltaTime) override
    {
    }

    void render(EntityManager &entityManager, SDL_Renderer *renderer) override {
    };
};

class FlammeMenuComponentLoader
{
public:
    static FlammeMenuComponentParameters fromJSON(const nlohmann::json &j, EntityManager &entityManager)
    {

        AnimatorComponent *animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
        if (!animator)
        {
            throw std::runtime_error("MegamanBulletComponent requires a AnimatorComponent component");
        }
        FlammeMenuComponentParameters params(animator);

        return params;
    }

    // Helper function to directly create a Camera component from JSON
    static FlammeMenuComponent createFromJSON(nlohmann::json &j, EntityManager &entityManager)
    {
        return FlammeMenuComponent(fromJSON(j, entityManager));
    }
};