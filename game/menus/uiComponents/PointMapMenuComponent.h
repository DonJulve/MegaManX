#include "Components.h"
#include "AnimatorComponent.h"
#pragma once

struct PointMapMenuComponentParameters {
    
    AnimatorComponent* animator;
    PointMapMenuComponentParameters( AnimatorComponent* ac) : animator(ac) {}
};



struct PointMapMenuComponent : public Component {

    AnimatorComponent* animator;

    PointMapMenuComponent(PointMapMenuComponentParameters params) : animator(params.animator){}

    void start() override
    {

    }

    void update(float deltaTime) override
    {
    }

};

class PointMapMenuComponentLoader {
    public:

        static PointMapMenuComponentParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            
            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("PointMapMenuComponent requires a AnimatorComponent component");
            }
            PointMapMenuComponentParameters params (animator);
    
            return params;
        }
    
        // Helper function to directly create a Camera component from JSON
        static PointMapMenuComponent createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
            return PointMapMenuComponent(fromJSON(j, entityManager));
        }
    };