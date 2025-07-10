#include "Components.h"
#pragma once

struct SpecMenuComponentParameters {

    SpecMenuComponentParameters() {}
};



struct SpecMenuComponent : public Component {

    SpecMenuComponent(SpecMenuComponentParameters params) {}

    void start() override
    {

    }

    void update(float deltaTime) override
    {
    }

};

class SpecMenuComponentLoader {
    public:

        static SpecMenuComponentParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {

            SpecMenuComponentParameters params;
    
            return params;
        }
    
        // Helper function to directly create a Camera component from JSON
        static SpecMenuComponent createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
            return SpecMenuComponent(fromJSON(j, entityManager));
        }
    };