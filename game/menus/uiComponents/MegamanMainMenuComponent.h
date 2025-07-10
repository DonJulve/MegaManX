#include "Components.h"
#include "RenderTextManager.h"
#include "Debug.h"
#include "InputManager.h"
#include "AnimatorComponent.h"
#pragma once

struct MegamanMainMenuComponentParameters {
    
    AnimatorComponent* animator;
    MegamanMainMenuComponentParameters( AnimatorComponent* ac) : animator(ac) {}
};



struct MegamanMainMenuComponent : public Component {

    AnimatorComponent* animator;

    MegamanMainMenuComponent(MegamanMainMenuComponentParameters params) : animator(params.animator){}

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

class MegamanMainMenuComponentLoader {
    public:

        static MegamanMainMenuComponentParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            
            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("MegamanBulletComponent requires a AnimatorComponent component");
            }
            MegamanMainMenuComponentParameters params (animator);
    
            return params;
        }
    
        // Helper function to directly create a Camera component from JSON
        static MegamanMainMenuComponent createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
            return MegamanMainMenuComponent(fromJSON(j, entityManager));
        }
    };