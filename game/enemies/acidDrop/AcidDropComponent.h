#pragma once

#pragma once

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"


struct AcidDropComponentParameters {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    
    AcidDropComponentParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb):
    transform(t), entityManager(em), animator(ac), rigidbody(rb) {}
};


struct AcidDropComponent : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;

    Collider* collider;

    AcidDropComponent(AcidDropComponentParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidbody(params.rigidbody) {}

    std::function<void(Collider*, CollisionResult)> callbackCollisionEnter = [this](Collider* other, CollisionResult cd) {
        if(other->getTag() == "Terrain"){
            animator->setBool("isLanding", true);
            rigidbody->velocityY = 0.0f;
        }
    };

    void start() override {
        collider = entityManager->getComponent<Collider>(parent);
        collider->onCollisionEnter = callbackCollisionEnter;

        animator->addFrameTrigger("land", 6, [this](){
            entityManager->deleteEntity(parent);
        });
    }

    void update(float deltaTime) override {

    }
};

class AcidDropComponentLoader {
public:
static AcidDropComponentParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
    Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
    if (!transform) {
        throw std::runtime_error("AcidDropComponent requires a Transform component");
    }

    AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
    if (!animator) {
        throw std::runtime_error("AcidDropComponent requires a AnimatorComponent component");
    }

    Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
    if (!rigidbody) {
        throw std::runtime_error("AcidDropComponent requires a Rigidbody component");
    }

    AcidDropComponentParameters params(
        transform,
        &entityManager,
        animator,
        rigidbody
    );
    
    return params;
}

    static AcidDropComponent createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
        return AcidDropComponent(fromJSON(j, entityManager));
    }
};