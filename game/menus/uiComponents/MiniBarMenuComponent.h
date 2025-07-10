#include "Components.h"
#pragma once

struct MiniBarMenuComponentParameters {

    SpriteRenderer* renderMiniBarMenu;
    MiniBarMenuComponentParameters(SpriteRenderer* sp ) : renderMiniBarMenu(sp){}
};



struct MiniBarMenuComponent : public Component {

    SpriteRenderer* renderMiniBarMenu;

    MiniBarMenuComponent(MiniBarMenuComponentParameters params) : renderMiniBarMenu(params.renderMiniBarMenu) {}

    void actualizarBarraVida(float vidaReal, float maxVidaMegaman){
        int vida = (vidaReal * 16) / maxVidaMegaman;
        std::string ruta;
        ruta = "resources/sprites/pauseMenu/miniBar/miniBar_" + std::to_string(vida) + ".png";
        renderMiniBarMenu->texture = TextureManager::getInstance()->getTexture(ruta);
    }
    void start() override
    {

    }

    void update(float deltaTime) override
    {
    }

};

class MiniBarMenuComponentLoader {
    public:

        static MiniBarMenuComponentParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {

            SpriteRenderer* renderMiniBarMenu = entityManager.getComponent<SpriteRenderer>(entityManager.getLast());
            if (!renderMiniBarMenu) {
                throw std::runtime_error("MiniBarMenuComponent requires a SpriteRenderer component");
            }
            MiniBarMenuComponentParameters params(renderMiniBarMenu);
    
            return params;
        }
    
        // Helper function to directly create a Camera component from JSON
        static MiniBarMenuComponent createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
            return MiniBarMenuComponent(fromJSON(j, entityManager));
        }
    };