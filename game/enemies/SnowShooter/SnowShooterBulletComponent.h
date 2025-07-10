#ifndef SNOWSHOOTERBULLETCONTROLLER_H
#define SNOWSHOOTERBULLETCONTROLLER_H

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"

struct SnowShooterBulletParameters{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    SnowShooterBulletParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb):
    transform(t), entityManager(em), animator(ac), rigidBody(rb) {}
};

struct SnowShooterBulletComponent : public Component{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    SnowShooterBulletComponent(SnowShooterBulletParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidBody(params.rigidBody) {}

    float speed = 200.0f;

    Vector2D direction;

    void setDirection(Vector2D destino){
        //Normalizar la direccion
        direction.x = destino.x - transform->posX;
        direction.y = destino.y - transform->posY;
        float length = sqrt((direction.x * direction.x) + (direction.y * direction.y));
        rigidBody->velocityX = direction.x / length * speed;
        rigidBody->velocityY = direction.y / length * speed;
        return;
    }

    void start() override{
        Collider* collider = entityManager->getComponent<Collider>(parent);

        rigidBody->velocityX = -speed;
        rigidBody->velocityY = 0;

    }

    void update(float deltaTime) override{
    }
};

class SnowShooterBulletComponentLoader{
    public:
        static SnowShooterBulletComponent fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("SnowShooterBulletComponent requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("SnowShooterBulletComponent requires a AnimatorComponent component");
            }

            Rigidbody* rigidBody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidBody) {
                throw std::runtime_error("SnowShooterBulletComponent requires a Rigidbody component");
            }

            SnowShooterBulletParameters params(
                transform,
                &entityManager,
                animator,
                rigidBody
            );

            return params;
        }

        static SnowShooterBulletComponent createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return SnowShooterBulletComponent(fromJSON(j, entityManager));
        }
};



#endif
