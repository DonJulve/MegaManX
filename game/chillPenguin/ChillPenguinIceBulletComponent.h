#ifndef CHILLPENGUINICEBULLETCONTROLLER_H
#define CHILLPENGUINICEBULLETCONTROLLER_H

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "ChillPenguinPenguinComponent.h"

struct ChillPenguinIceBulletParameters{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    ChillPenguinIceBulletParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb):
    transform(t), entityManager(em), animator(ac), rigidBody(rb) {}
};

struct ChillPenguinIceBulletComponent : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;


    ChillPenguinIceBulletComponent(ChillPenguinIceBulletParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidBody(params.rigidBody) {}

    float speed = 100.0f;
    bool movingRight = true; // Por defecto, se mueve hacia la derecha

    void start() override {
        Collider* collider = entityManager->getComponent<Collider>(parent);

        // Usamos un índice para determinar si esta bala debe crear un pingüino
        static int penguinCreationCounter = 0;

        animator->addFrameTrigger("shoot", 4, [this]() {
            // Solo creamos pingüinos en las primeras 2 balas
            if (penguinCreationCounter < 2) {
                if (!PrefabManager::getInstance().hasPrefab("chillpenguinPenguin")) {
                    PrefabManager::getInstance().loadPrefab("chillpenguinPenguin");
                }
                Entity bulletEntity = PrefabManager::getInstance().instantiatePrefab("chillpenguinPenguin", *entityManager);
                Transform* bulletTransform = entityManager->getComponent<Transform>(bulletEntity);
                ChillPenguinPenguinComponent* bulletComponent = entityManager->getComponent<ChillPenguinPenguinComponent>(bulletEntity);
                if (bulletTransform) {
                    int separation = 40; // Separación base
                    bulletComponent->movingRight = movingRight;
                    if (movingRight) {
                        bulletTransform->posX = transform->posX + (separation * (penguinCreationCounter + 1));
                    } else {
                        bulletTransform->posX = transform->posX - (separation * (penguinCreationCounter + 1));
                    }
                    bulletTransform->posY = transform->posY;
                    bulletComponent->start();
                }
                penguinCreationCounter++; // Incrementamos el contador
                animator->addFrameTrigger("shoot", 5, [this]() {
                    penguinCreationCounter = 0;
                });
            }
        });

        rigidBody->velocityX = 0;
        rigidBody->velocityY = 0;
    }

    void update(float deltaTime) override {
    }
};
class ChillPenguinIceBulletComponentLoader{
    public:
        static ChillPenguinIceBulletComponent fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("ChillPenguinIceBulletComponent requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("ChillPenguinIceBulletComponent requires a AnimatorComponent component");
            }

            Rigidbody* rigidBody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidBody) {
                throw std::runtime_error("ChillPenguinIceBulletComponent requires a Rigidbody component");
            }

            ChillPenguinIceBulletParameters params(
                transform,
                &entityManager,
                animator,
                rigidBody
            );

            return params;
        }

        static ChillPenguinIceBulletComponent createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return ChillPenguinIceBulletComponent(fromJSON(j, entityManager));
        }
};



#endif
