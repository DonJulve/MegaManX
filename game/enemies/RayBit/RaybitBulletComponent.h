#ifndef RAYBITBULLETCOMPONENT_H
#define RAYBITBULLETCOMPONENT_H

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"

struct RaybitBulletParameters{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    RaybitBulletParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb):
    transform(t), entityManager(em), animator(ac), rigidBody(rb) {}
};

struct RaybitBulletComponent : public Component{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    RaybitBulletComponent(RaybitBulletParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidBody(params.rigidBody) {}

    float speed = 200.0f;
    bool isMoving = false;

    void start() override{
        Collider* collider = entityManager->getComponent<Collider>(parent);

        animator->addFrameTrigger("charging", 4, [this](){
            animator->setBool("isShooting", true);
            rigidBody->velocityX = -speed;
        });

    }

    void update(float deltaTime) override{
    }
};

class RaybitBulletComponentLoader{
    public:
        static RaybitBulletComponent fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("RaybitBulletComponent requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("RaybitBulletComponent requires a AnimatorComponent component");
            }

            Rigidbody* rigidBody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidBody) {
                throw std::runtime_error("RaybitBulletComponent requires a Rigidbody component");
            }

            RaybitBulletParameters params(
                transform,
                &entityManager,
                animator,
                rigidBody
            );

            return params;
        }

        static RaybitBulletComponent createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return RaybitBulletComponent(fromJSON(j, entityManager));
        }
};



#endif
