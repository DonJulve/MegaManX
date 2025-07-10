#include "Components.h"
#pragma once

struct FullBarMenuComponentParameters
{

    SpriteRenderer *renderFullBarMenu;
    FullBarMenuComponentParameters(SpriteRenderer *sp) : renderFullBarMenu(sp) {}
};

struct FullBarMenuComponent : public Component
{

    SpriteRenderer *renderFullBarMenu;

    FullBarMenuComponent(FullBarMenuComponentParameters params) : renderFullBarMenu(params.renderFullBarMenu) {}

    void actualizarBarraVida(int availabre, float vidaReal, float maxVidaMegaman)
    {
        int vida = (vidaReal * 28) / maxVidaMegaman;
        std::string ruta;
        if (availabre == 1)
        {
            ruta = "resources/sprites/pauseMenu/fullBar/fullBar_" + std::to_string(vida) + ".png";
        }
        else if (availabre == 2)
        {
            ruta = "resources/sprites/pauseMenu/fullBarDisable/fullBarDisable_" + std::to_string(vida) + ".png";
        }
        renderFullBarMenu->texture = TextureManager::getInstance()->getTexture(ruta);
    }
    void start() override
    {
    }

    void update(float deltaTime) override
    {
    }
};

class FullBarMenuComponentLoader
{
public:
    static FullBarMenuComponentParameters fromJSON(const nlohmann::json &j, EntityManager &entityManager)
    {

        SpriteRenderer *renderFullBarMenu = entityManager.getComponent<SpriteRenderer>(entityManager.getLast());
        if (!renderFullBarMenu)
        {
            throw std::runtime_error("FullBarMenuComponent requires a SpriteRenderer component");
        }
        FullBarMenuComponentParameters params(renderFullBarMenu);

        return params;
    }

    // Helper function to directly create a Camera component from JSON
    static FullBarMenuComponent createFromJSON(nlohmann::json &j, EntityManager &entityManager)
    {
        return FullBarMenuComponent(fromJSON(j, entityManager));
    }
};