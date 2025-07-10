#ifndef MEGAMANMEDIUMBULLETCONTROLLER_H
#define MEGAMANMEDIUMBULLETCONTROLLER_H

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"

struct MegamanMediumBulletParameters{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    MegamanMediumBulletParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb):
    transform(t), entityManager(em), animator(ac), rigidBody(rb) {}
};

struct MegamanMediumBulletComponent : public Component{
    // -------------------------------------------
    //  Bullet Properties
    // -------------------------------------------
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;


    MegamanMediumBulletComponent(MegamanMediumBulletParameters params):
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
        // TODO CAMBIAR LA DIRECCION DEL SPRITE
        Collider* collider = entityManager->getComponent<Collider>(parent);
        if (movingRight) {
            rigidBody->velocityX = speed;
            animator->spriteRenderer->flip = SDL_FLIP_NONE;
        }
        else {
            rigidBody->velocityX = -speed;
            animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
        }

        // Impacto de bala
        std::function<void(Collider*, CollisionResult)> callbackCollisionEnter = [this](Collider* other, CollisionResult cd) {
            if(other->getTag() != "Enemy" && other->getTag() != "Penguin" && other->getTag() == "Igloo") return;   
            Vector2D resultVector = cd.normal;
                rigidBody->velocityX = 0;
                Debug::Log("IMPACTO BALA MEDIA");
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

class MegamanMediumBulletComponentLoader{
    public:
        static MegamanMediumBulletComponent fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("MegamanMediumBulletComponent requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("MegamanMediumBulletComponent requires a AnimatorComponent component");
            }

            Rigidbody* rigidBody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidBody) {
                throw std::runtime_error("MegamanMediumBulletComponent requires a Rigidbody component");
            }

            MegamanMediumBulletParameters params(
                transform,
                &entityManager,
                animator,
                rigidBody
            );

            return params;
        }

        static MegamanMediumBulletComponent createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return MegamanMediumBulletComponent(fromJSON(j, entityManager));
        }
};



#endif
