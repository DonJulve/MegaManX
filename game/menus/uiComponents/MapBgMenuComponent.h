#include "Components.h"
#pragma once

struct MapBgMenuComponentParameters
{

    MapBgMenuComponentParameters() {}
};

struct MapBgMenuComponent : public Component
{

    MapBgMenuComponent(MapBgMenuComponentParameters params) {}

    void start() override
    {
    }

    void update(float deltaTime) override
    {
    }
};

class MapBgMenuComponentLoader
{
public:
    static MapBgMenuComponentParameters fromJSON(const nlohmann::json &j, EntityManager &entityManager)
    {

        MapBgMenuComponentParameters params;

        return params;
    }

    // Helper function to directly create a Camera component from JSON
    static MapBgMenuComponent createFromJSON(nlohmann::json &j, EntityManager &entityManager)
    {
        return MapBgMenuComponent(fromJSON(j, entityManager));
    }
};