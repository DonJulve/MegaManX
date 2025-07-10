#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#pragma once

struct SelectorMenuComponentParameters {
    
    AnimatorComponent* animator;
    SelectorMenuComponentParameters( AnimatorComponent* ac) : animator(ac) {}
};



struct SelectorMenuComponent : public Component {

    AnimatorComponent* animator;

    SelectorMenuComponent(SelectorMenuComponentParameters params) : animator(params.animator){}

    void start() override
    {

    }

    void update(float deltaTime) override
    {
    }

};

class SelectorMenuComponentLoader {
    public:

        static SelectorMenuComponentParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            
            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("SelectorMenuComponent requires a AnimatorComponent component");
            }
            SelectorMenuComponentParameters params (animator);
    
            return params;
        }
    
        // Helper function to directly create a Camera component from JSON
        static SelectorMenuComponent createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
            return SelectorMenuComponent(fromJSON(j, entityManager));
        }
    };