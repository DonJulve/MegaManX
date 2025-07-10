#pragma once

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"

struct HoganmerBallParameters {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;

    HoganmerBallParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb):
    transform(t), entityManager(em), animator(ac), rigidbody(rb){}
};

struct HoganmerBall : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;

    HoganmerBall(HoganmerBallParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidbody(params.rigidbody) {}

    Collider* collider;
    float attackCooldown = 2.0f;
    float attackTimer = 0.0f;
    bool vuelta = false;
    std::function<void()> parentCallback;
    bool attack = false;

    void setVelocity(float x, float y) {
        vuelta = false;
        rigidbody->velocityX = x;
        rigidbody->velocityY = y;
    }

    void setChain(){
        animator->setBool("isChain", true);
        collider->tag = "NoDamage";
    }

    void setPosition(float x, float y) {
        transform->posX = x;
        transform->posY = y;
    }

    void aparecer(){
        animator->spriteRenderer->alpha = 255;
        attack = true;
    }

    void desaparecer(){
        animator->spriteRenderer->alpha = 0;
        attack = false;
    }

    void setCallback(std::function<void()> callback) {
        parentCallback = callback;
    }

    void start() override {
        collider = entityManager->getComponent<Collider>(parent);
        if (!collider) {
            throw std::runtime_error("HoganmerBall requires a Collider component");
        }

    }

    void update(float deltaTime) override {
        if(!attack) return;
        attackTimer += deltaTime;
        if(attackTimer >= attackCooldown && !vuelta){
            attackTimer = 0.0f;
            vuelta = true;
            rigidbody->velocityX = -rigidbody->velocityX;
            rigidbody->velocityY = -rigidbody->velocityY;
        }
        else if(attackTimer >= attackCooldown && vuelta){
            attackTimer = 0.0f;
            vuelta = false;
            if(parentCallback) {
                parentCallback();
            }
        }
    }


};


class HoganmerBallLoader {
public:
    static HoganmerBallParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
        Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
        if (!transform) {
            throw std::runtime_error("HoganmerBall requires a Transform component");
        }

        AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
        if (!animator) {
            throw std::runtime_error("HoganmerBall requires a AnimatorComponent component");
        }

        Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
        if (!rigidbody) {
            throw std::runtime_error("HoganmerBall requires a Rigidbody component");
        }

        HoganmerBallParameters params(
            transform,
            &entityManager,
            animator,
            rigidbody
        );

        return params;
    }

    static HoganmerBall createFromJSON(const nlohmann::json& j, EntityManager& entityManager) {
        HoganmerBallParameters params = fromJSON(j, entityManager);
        return HoganmerBall(params);
    }

};

