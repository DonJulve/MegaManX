#include "Components.h"
#pragma once

struct MapMenuComponentParameters {

    MapMenuComponentParameters() {}
};



struct MapMenuComponent : public Component {

    MapMenuComponent(MapMenuComponentParameters params) {}

    void start() override
    {

    }

    void update(float deltaTime) override
    {
    }

};

class MapMenuComponentLoader {
    public:

        static MapMenuComponentParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {

            MapMenuComponentParameters params;
    
            return params;
        }
    
        // Helper function to directly create a Camera component from JSON
        static MapMenuComponent createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
            return MapMenuComponent(fromJSON(j, entityManager));
        }
    };