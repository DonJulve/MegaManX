#ifndef CHILLPENGUINBULLETCONTROLLER_H
#define CHILLPENGUINBULLETCONTROLLER_H

#include <SDL2/SDL.h>
#include "RaycastManager.h"
#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "ChillPenguinPenguinComponent.h" 

struct ChillPenguinBulletParameters {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    ChillPenguinBulletParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb)
        : transform(t), entityManager(em), animator(ac), rigidBody(rb) {}
};

struct ChillPenguinBulletComponent : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    AudioManager* audioManager;

    ChillPenguinBulletComponent(ChillPenguinBulletParameters params)
        : transform(params.transform),
          entityManager(params.entityManager),
          animator(params.animator),
          rigidBody(params.rigidBody) {}

    float speed = 200.0f;
    bool movingRight = false;

    void start() override {
        Collider* collider = entityManager->getComponent<Collider>(parent);
        audioManager = AudioManager::getInstance();
        audioManager->loadSound("Ice_break", "sound_effects/chill penguin/Ice Break.wav");

        if (movingRight) {
            rigidBody->velocityX = speed;
        } else {
            rigidBody->velocityX = -speed;
        }
        rigidBody->velocityY = 0;
    }

    void update(float deltaTime) override {
        // Lanzar un rayo en la dirección de movimiento de la bala
        SDL_FPoint origin = { static_cast<float>(transform->posX), static_cast<float>(transform->posY) };
        SDL_FPoint direction = movingRight ? SDL_FPoint{ 1.0f, 0.0f } : SDL_FPoint{ -1.0f, 0.0f };
        Ray ray(origin, direction);

        // Realizar el raycast
        std::optional<RaycastHit> hit = RayCastManager::raycast(ray, entityManager, 10.0f, { "Penguin" });

        // Si el rayo intersecta con un pingüino
        if (hit.has_value()) {
            Entity hitEntity = hit.value().collider->parent;
            ChillPenguinPenguinComponent* penguinComponent = entityManager->getComponent<ChillPenguinPenguinComponent>(hitEntity);
            if (penguinComponent) {
                // Destruir la bala
                entityManager->deleteEntity(this->parent);
                audioManager->playSound("Ice_break");
            }
        }
    }
};

class ChillPenguinBulletComponentLoader {
public:
    static ChillPenguinBulletComponent fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
        Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
        if (!transform) {
            throw std::runtime_error("ChillPenguinBulletComponent requires a Transform component");
        }

        AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
        if (!animator) {
            throw std::runtime_error("ChillPenguinBulletComponent requires a AnimatorComponent component");
        }

        Rigidbody* rigidBody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
        if (!rigidBody) {
            throw std::runtime_error("ChillPenguinBulletComponent requires a Rigidbody component");
        }

        ChillPenguinBulletParameters params(
            transform,
            &entityManager,
            animator,
            rigidBody
        );

        return params;
    }

    static ChillPenguinBulletComponent createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
        return ChillPenguinBulletComponent(fromJSON(j, entityManager));
    }
};

#endif
