#include "Components.h"
#include "InputManager.h"
#include "Debug.h"
#pragma once

struct BarPowerUpsComponentParameters
{

    SpriteRenderer *renderBarPowerUp;
    EntityManager *entityManager;
    BarPowerUpsComponentParameters(SpriteRenderer *sp, EntityManager *em) : renderBarPowerUp(sp), entityManager(em) {}
};

struct BarPowerUpsComponent : public Component
{

    SpriteRenderer *renderBarPowerUp;
    EntityManager *entityManager;
    HealthComponent *healthComponentplayer;

    std::shared_ptr<MegamanController> player;

    bool active = true;
    int powerUp = 4;

    BarPowerUpsComponent(BarPowerUpsComponentParameters params) : renderBarPowerUp(params.renderBarPowerUp), entityManager(params.entityManager) {}

    void actualizarBarraPowerUp(float powerUpQueda, float powerUpTotal)
    {
        int vida = (powerUpQueda * 28) / powerUpTotal;
        std::string ruta;
        ruta = "resources/sprites/powerUpsbar/powerUp" + std::to_string(powerUp) + "/powerUp" + std::to_string(powerUp) + "_" + std::to_string(vida) + ".png";
        Debug::Log("Ruta de la barra de vida: " + ruta);
        renderBarPowerUp->texture = TextureManager::getInstance()->getTexture(ruta);
    }

    void cambiarPowerUp(int powerUpNuevo)
    {
        powerUp = powerUpNuevo;
        actualizarBarraPowerUp(0, 28);
    }

    void esconderPowerUp(bool activo)
    {
        if (activo)
        {
            renderBarPowerUp->alpha = 255;
        }
        else
        {
            renderBarPowerUp->alpha = 0;
        }
    }
    void start() override
    {
    }

    void update(float deltaTime) override
    {
        // TODO: coger el powerUp y actualizar la barra de vida
        // TODO: si no hay powerUp desactivar la barra de vida

        if (InputManager::isKeyPressed(SDL_SCANCODE_1) && active)
        {
            active = false;
            cambiarPowerUp(1);
        }

        if (InputManager::isKeyPressed(SDL_SCANCODE_2) && active)
        {
            active = false;
            cambiarPowerUp(4);
        }
        if (InputManager::isKeyPressed(SDL_SCANCODE_3) && active)
        {
            active = false;
            cambiarPowerUp(7);
        }
        if (InputManager::isKeyPressed(SDL_SCANCODE_4) && active)
        {
            active = false;
            cambiarPowerUp(8);
        }

        if (InputManager::isKeyPressed(SDL_SCANCODE_A) && active)
        {
            active = false;
            actualizarBarraPowerUp(1, 28);
        }

        if (InputManager::isKeyHeld(SDL_SCANCODE_1) || InputManager::isKeyHeld(SDL_SCANCODE_A) || InputManager::isKeyHeld(SDL_SCANCODE_2) || InputManager::isKeyHeld(SDL_SCANCODE_3) || InputManager::isKeyHeld(SDL_SCANCODE_4))
        {
            active = true;
        }
    }
};

class BarPowerUpsComponentLoader
{
public:
    static BarPowerUpsComponentParameters fromJSON(const nlohmann::json &j, EntityManager &entityManager)
    {

        SpriteRenderer *renderBarPowerUp = entityManager.getComponent<SpriteRenderer>(entityManager.getLast());
        if (!renderBarPowerUp)
        {
            throw std::runtime_error("BarPowerUpsComponent requires a SpriteRenderer component");
        }
        BarPowerUpsComponentParameters params(renderBarPowerUp, &entityManager);

        return params;
    }

    // Helper function to directly create a Camera component from JSON
    static BarPowerUpsComponent createFromJSON(nlohmann::json &j, EntityManager &entityManager)
    {
        return BarPowerUpsComponent(fromJSON(j, entityManager));
    }
};