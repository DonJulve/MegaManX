#pragma once

#include "Components.h"
#include "AnimatorComponent.h"
#include "RaycastManager.h"
#include "Debug.h"

struct  FireParameters {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    FireParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb):
    transform(t), entityManager(em), animator(ac), rigidBody(rb) {}
};

struct FireComponent : public Component{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    Collider* collider;

    FireComponent(FireParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidBody(params.rigidBody) {}

    std::function<void(Collider*, CollisionResult)> callbackCollisionEnter = [this](Collider* other, CollisionResult cd) {
        if (other->tag == "Terrain"){
            animator->setBool("extinct", true);
            rigidBody->velocityX = 0;
            rigidBody->velocityY = 0;
            rigidBody->kinematic = true;
        }
        if(other->tag == "Lava"){
            entityManager->deleteEntity(parent);
        }
    };

    void setVelocity(float x, float y) {
        rigidBody->velocityX = x;
        rigidBody->velocityY = y;
    }

    void start() override {
        collider = entityManager->getComponent<Collider>(parent);

        animator->addFrameTrigger("extinct", 6, [this](){
            // Destruir el fuego
            entityManager->deleteEntity(parent);
        });

        collider->onCollisionEnter = callbackCollisionEnter;

    }

    void update(float deltaTime) override {
    }

};

class FireLoader {
    public:
        static FireParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("FireComponent requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("FireComponent requires a AnimatorComponent component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("FireComponent requires a Rigidbody component");
            }

            FireParameters params(
                transform,
                &entityManager,
                animator,
                rigidbody
            );

            return params;
        }

        static FireComponent createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return FireComponent(fromJSON(j, entityManager));
        }

    };
