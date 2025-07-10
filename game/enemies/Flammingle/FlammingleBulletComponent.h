#ifndef FLAMMINGLEBULLETCONTROLLER_H
#define FLAMMINGLEBULLETCONTROLLER_H

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"

struct FlammingleBulletParameters{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    FlammingleBulletParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb):
    transform(t), entityManager(em), animator(ac), rigidBody(rb) {}
};

struct FlammingleBulletComponent : public Component{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    FlammingleBulletComponent(FlammingleBulletParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidBody(params.rigidBody) {}

    float speed = 180.0f;
    float time = 0.0f;

    Vector2D direction;

    void setDirection(Vector2D destino){
        //Normalizar la direccion
        direction.x = destino.x - transform->posX;
        direction.y = destino.y - transform->posY;
        float length = sqrt((direction.x * direction.x) + (direction.y * direction.y));
        rigidBody->velocityX = direction.x / length * -speed;
        rigidBody->velocityY = direction.y / length * speed;
        return;
    }

    void start() override{
        Collider* collider = entityManager->getComponent<Collider>(parent);

        rigidBody->velocityX = -speed;
        rigidBody->velocityY = speed;
    }

    void update(float deltaTime) override{
      time += deltaTime;
      if (time >= 0.3f) animator->setBool("isShooting", true);
    }
};

class FlammingleBulletComponentLoader{
    public:
        static FlammingleBulletComponent fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("FlammingleBulletComponent requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("FlammingleBulletComponent requires a AnimatorComponent component");
            }

            Rigidbody* rigidBody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidBody) {
                throw std::runtime_error("FlammingleBulletComponent requires a Rigidbody component");
            }

            FlammingleBulletParameters params(
                transform,
                &entityManager,
                animator,
                rigidBody
            );

            return params;
        }

        static FlammingleBulletComponent createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return FlammingleBulletComponent(fromJSON(j, entityManager));
        }
};



#endif
