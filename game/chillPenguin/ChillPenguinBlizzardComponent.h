#ifndef CHILLPENGUINBLIZZARDCONTROLLER_H
#define CHILLPENGUINBLIZZARDCONTROLLER_H

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"

struct ChillPenguinBlizzardParameters{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    ChillPenguinBlizzardParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb):
    transform(t), entityManager(em), animator(ac), rigidBody(rb) {}
};

struct ChillPenguinBlizzardComponent : public Component{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    ChillPenguinBlizzardComponent(ChillPenguinBlizzardParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidBody(params.rigidBody) {}

    void start() override{
        Collider* collider = entityManager->getComponent<Collider>(parent);

        rigidBody->velocityX = 0;
        rigidBody->velocityY = 0;

    }

    void update(float deltaTime) override{
    }
};

class ChillPenguinBlizzardComponentLoader{
    public:
        static ChillPenguinBlizzardComponent fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("ChillPenguinBlizzardComponent requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("ChillPenguinBlizzardComponent requires a AnimatorComponent component");
            }

            Rigidbody* rigidBody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidBody) {
                throw std::runtime_error("ChillPenguinBlizzardComponent requires a Rigidbody component");
            }

            ChillPenguinBlizzardParameters params(
                transform,
                &entityManager,
                animator,
                rigidBody
            );

            return params;
        }

        static ChillPenguinBlizzardComponent createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return ChillPenguinBlizzardComponent(fromJSON(j, entityManager));
        }
};



#endif
