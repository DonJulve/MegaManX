#include "Components.h"
#include "RenderTextManager.h"
#include "Debug.h"
#include "InputManager.h"
#include "AnimatorComponent.h"
#pragma once

struct BulletMainMenuComponentParameters {
    
    AnimatorComponent* animator;
    BulletMainMenuComponentParameters( AnimatorComponent* ac) : animator(ac) {}
};



struct BulletMainMenuComponent : public Component {

    AnimatorComponent* animator;

    BulletMainMenuComponent(BulletMainMenuComponentParameters params) : animator(params.animator){}

    void start() override
    {

    }

    void update(float deltaTime) override
    {
    }

    void render(EntityManager& entityManager, SDL_Renderer* renderer) override 
    {
    };
};

class BulletMainMenuComponentLoader {
    public:

        static BulletMainMenuComponentParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            
            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("MegamanBulletComponent requires a AnimatorComponent component");
            }
            BulletMainMenuComponentParameters params (animator);
    
            return params;
        }
    
        // Helper function to directly create a Camera component from JSON
        static BulletMainMenuComponent createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
            return BulletMainMenuComponent(fromJSON(j, entityManager));
        }
    };