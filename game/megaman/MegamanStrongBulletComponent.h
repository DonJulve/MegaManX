#ifndef MEGAMANSTRONGBULLETCONTROLLER_H
#define MEGAMANSTRONGBULLETCONTROLLER_H

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"

struct MegamanStrongBulletParameters{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    MegamanStrongBulletParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb):
    transform(t), entityManager(em), animator(ac), rigidBody(rb) {}
};

struct MegamanStrongBulletComponent : public Component{
    // -------------------------------------------
    //  Bullet Properties
    // -------------------------------------------
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;


    MegamanStrongBulletComponent(MegamanStrongBulletParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidBody(params.rigidBody) {}

    float speed = 500.0f;
    bool movingRight = false;

    // Quitamos la bala de la escena
    void removeBullet() {
        entityManager->deleteEntity(parent);
    }

    void start() override{
        // Direccion de la bala
        Collider* collider = entityManager->getComponent<Collider>(parent);
        if (movingRight) {
            rigidBody->velocityX = speed;
            animator->spriteRenderer->flip = SDL_FLIP_NONE;
        } else {
            rigidBody->velocityX = -speed;
            animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
        }

        // Impacto de bala
        std::function<void(Collider*, CollisionResult)> callbackCollisionEnter = [this](Collider* other, CollisionResult cd) {
            if(other->getTag() != "Enemy" && other->getTag() != "Penguin" && other->getTag() == "Igloo") return;   
            Vector2D resultVector = cd.normal;
                rigidBody->velocityX = 0;
                Debug::Log("IMPACTO BALA");
                animator->setBool("hasImpacted", true);
        };
        collider->onCollisionEnter = callbackCollisionEnter;

        animator->addFrameTrigger("impact", 5, [this](){
            removeBullet();
        });
    }

    void update(float deltaTime) override{
    }
};

class MegamanStrongBulletComponentLoader{
    public:
        static MegamanStrongBulletComponent fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("MegamanStrongBulletComponent requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("MegamanStrongBulletComponent requires a AnimatorComponent component");
            }

            Rigidbody* rigidBody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidBody) {
                throw std::runtime_error("MegamanStrongBulletComponent requires a Rigidbody component");
            }

            MegamanStrongBulletParameters params(
                transform,
                &entityManager,
                animator,
                rigidBody
            );

            return params;
        }

        static MegamanStrongBulletComponent createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return MegamanStrongBulletComponent(fromJSON(j, entityManager));
        }
};



#endif
