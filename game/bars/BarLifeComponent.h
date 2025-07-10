#include "Components.h"
#pragma once

struct BarLifeComponentParameters
{

    SpriteRenderer *renderBarLife;
    EntityManager *entityManager;
    BarLifeComponentParameters(SpriteRenderer *sp, EntityManager *em) : renderBarLife(sp), entityManager(em) {}
};

struct BarLifeComponent : public Component
{

    SpriteRenderer *renderBarLife;
    EntityManager *entityManager;
    HealthComponent *healthComponentplayer;

    BarLifeComponent(BarLifeComponentParameters params) : renderBarLife(params.renderBarLife), entityManager(params.entityManager) {}

    void actualizarBarraVida(float vidaReal, float maxVidaMegaman)
    {
        int vida = (vidaReal * 16) / maxVidaMegaman;
        std::string ruta;
        if(vida < 0)
        {
            vida = 0;
        }
        ruta = "resources/sprites/barlife/barlife_" + std::to_string(vida) + ".png";
        renderBarLife->texture = TextureManager::getInstance()->getTexture(ruta);
    }

    bool findPlayer()
    {
        std::shared_ptr<MegamanController> mc;

        Entity t = entityManager->findComponent<MegamanController>(mc);

        // Initialize Mega Man's transform
        healthComponentplayer = entityManager->getComponent<HealthComponent>(t);
        if (!healthComponentplayer || t==0) {
            
            std::shared_ptr<MegamanArmorController> mAc;

            t = entityManager->findComponent<MegamanArmorController>(mAc);

            // Initialize Mega Man's transform
            healthComponentplayer = entityManager->getComponent<HealthComponent>(t);

            if (!healthComponentplayer || t==0) {
                return false;
            }
            return true;
        }

        return true;
    }	

    void start() override
    {
        if (findPlayer())
        {
            actualizarBarraVida(healthComponentplayer->getCurrentHealth(), healthComponentplayer->getMaxHealth());
        }
    }

    void update(float deltaTime) override
    {
        if (findPlayer())
        {
            actualizarBarraVida(healthComponentplayer->getCurrentHealth(), healthComponentplayer->getMaxHealth());
        }
    }
};

class BarLifeComponentLoader
{
public:
    static BarLifeComponentParameters fromJSON(const nlohmann::json &j, EntityManager &entityManager)
    {

        SpriteRenderer *renderBarLife = entityManager.getComponent<SpriteRenderer>(entityManager.getLast());
        if (!renderBarLife)
        {
            throw std::runtime_error("BarLifeComponent requires a SpriteRenderer component");
        }
        BarLifeComponentParameters params(renderBarLife, &entityManager);

        return params;
    }

    // Helper function to directly create a Camera component from JSON
    static BarLifeComponent createFromJSON(nlohmann::json &j, EntityManager &entityManager)
    {
        return BarLifeComponent(fromJSON(j, entityManager));
    }
};