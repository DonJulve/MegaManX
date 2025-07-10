#ifndef MEGAMANBULLETCONTROLLER_H
#define MEGAMANBULLETCONTROLLER_H

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"

struct MegamanBulletParameters{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    MegamanBulletParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb):
    transform(t), entityManager(em), animator(ac), rigidBody(rb) {}
};

struct MegamanBulletComponent : public Component{
    // -------------------------------------------
    //  Bullet Properties
    // -------------------------------------------
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    AudioManager* audioManager;


    MegamanBulletComponent(MegamanBulletParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidBody(params.rigidBody) {}

    float speed = 500.0f;
    bool movingRight = false;

    void loadSoundEffects() {
        audioManager = AudioManager::getInstance();

        // Cargar pistas
        audioManager->loadSound("smallHit", "sound_effects/hits/Small Hit.wav");
    }
    // Quitamos la bala de la escena
    void removeBullet() {
        entityManager->deleteEntity(parent);
    }

    void start() override{
        // Direccion de la bala
        Collider* collider = entityManager->getComponent<Collider>(parent);
        if (movingRight) {
            rigidBody->velocityX = speed;
        } else {
            rigidBody->velocityX = -speed;
        }

        // Impacto de bala
        std::function<void(Collider*, CollisionResult)> callbackCollisionEnter = [this](Collider* other, CollisionResult cd) {
            if(other->getTag() != "Enemy" && other->getTag() != "Penguin" && other->getTag() != "Igloo") return;   
                rigidBody->velocityX = 0;
                Debug::Log("IMPACTO BALA");
                animator->setBool("hasImpacted", true);
                //audioManager->playSound("smallHit");
        };
        collider->onCollisionEnter = callbackCollisionEnter;

        animator->addFrameTrigger("impact", 2, [this](){
            removeBullet();
        });
    }

    void update(float deltaTime) override{
    }
};

class MegamanBulletComponentLoader{
    public:
        static MegamanBulletComponent fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("MegamanBulletComponent requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("MegamanBulletComponent requires a AnimatorComponent component");
            }

            Rigidbody* rigidBody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidBody) {
                throw std::runtime_error("MegamanBulletComponent requires a Rigidbody component");
            }

            MegamanBulletParameters params(
                transform,
                &entityManager,
                animator,
                rigidBody
            );

            return params;
        }

        static MegamanBulletComponent createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return MegamanBulletComponent(fromJSON(j, entityManager));
        }
};



#endif
