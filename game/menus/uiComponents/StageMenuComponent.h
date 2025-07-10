#include "Components.h"
#pragma once

struct StageMenuComponentParameters {

    StageMenuComponentParameters() {}
};



struct StageMenuComponent : public Component {

    StageMenuComponent(StageMenuComponentParameters params) {}

    void start() override
    {

    }

    void update(float deltaTime) override
    {
    }

};

class StageMenuComponentLoader {
    public:

        static StageMenuComponentParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {

            StageMenuComponentParameters params;
    
            return params;
        }
    
        // Helper function to directly create a Camera component from JSON
        static StageMenuComponent createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
            return StageMenuComponent(fromJSON(j, entityManager));
        }
    };