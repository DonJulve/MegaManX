#pragma once

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "enemies/tombot/TombotController.h"


struct IglooControllerParameters {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    HealthComponent* healthComponent;

    IglooControllerParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb, HealthComponent* hc):
    transform(t), entityManager(em), animator(ac), rigidbody(rb), healthComponent(hc) {}
};

struct IglooController : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    HealthComponent* healthComponent;

    IglooController(IglooControllerParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidbody(params.rigidbody),
    healthComponent(params.healthComponent) {}

    std::shared_ptr<MegamanController> player;
    Collider* collider;
    bool muerto = false;

    float deltaX = 40.0f;
    float spawnTimer = 0.0f;
    float spawnCooldown = 4.0f;
    bool estaCerca = false;
    float velXInicial = 50.0f;

    float agroDistance = 200.0f;

    std::function<void(Collider*, CollisionResult)> onCollisionEnter = [this](Collider* other, CollisionResult result) {
    };

    void findPlayer(){
        if(!player) entityManager->findComponent<MegamanController>(player);
    }

    void spawnTombot(bool direction){
        if (!PrefabManager::getInstance().hasPrefab("tombot")) {
            PrefabManager::getInstance().loadPrefab("tombot");
        }
        Entity tombotEntity = PrefabManager::getInstance().instantiatePrefab("tombot", *entityManager);
        Transform* tombotTransform = entityManager->getComponent<Transform>(tombotEntity);
        TombotController* tombotController = entityManager->getComponent<TombotController>(tombotEntity);
        Rigidbody* tombotRigidbody = entityManager->getComponent<Rigidbody>(tombotEntity);
        if(tombotTransform){
            tombotTransform->posY = transform->posY;
            tombotTransform->posX = direction ? transform->posX + deltaX : transform->posX - deltaX;
        }
        if(tombotRigidbody){
            tombotRigidbody->velocityX = direction ? velXInicial : -velXInicial;
        }
        tombotController->start();
        tombotController->setDirection(direction);
    }

    void selectIgloo(int index){
        if(index == 1){
            animator->setBool("isIgloo1", true);
        }
        else if(index == 2){
            animator->setBool("isIgloo2", true);
        }
        else if(index == 3){
            animator->setBool("isIgloo3", true);
        }
    }

    void start() override {
        collider = entityManager->getComponent<Collider>(parent);
        if (!collider) {
            throw std::runtime_error("IglooController requires a Collider component");
        }

        collider->onCollisionEnter = onCollisionEnter;
    }
    void update(float deltaTime) override {
        if(!muerto){
            findPlayer();

            if(healthComponent->isDead() && !muerto){
                muerto = true;
                animator->setBool("isDestroyed", true);
                transform->posY += 27.0f;
                collider->scaleY = 20.0f;
            }

            Vector2D normal = {0.0f, 0.0f};
            if(player){
                normal.x = player->getTransform()->posX - transform->posX;
                normal.y = player->getTransform()->posY - transform->posY;

                estaCerca = (normal.x <= agroDistance && normal.x >= -agroDistance);
                if(estaCerca){
                    spawnTimer += deltaTime;
                    if(spawnTimer >= spawnCooldown){
                        spawnTimer = 0.0f;
                        bool direction = normal.x > 0.0f;
                        spawnTombot(direction);
                    }
                }
            }
        }
    }

};

class IglooControllerLoader {
    public:
        static IglooControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("IglooController requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("IglooController requires a AnimatorComponent component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("IglooController requires a Rigidbody component");
            }

            HealthComponent* healthComponent = entityManager.getComponent<HealthComponent>(entityManager.getLast());
            if (!healthComponent) {
                throw std::runtime_error("IglooController requires a HealthComponent component");
            }

            IglooControllerParameters params(
                transform, 
                &entityManager, 
                animator, 
                rigidbody, 
                healthComponent
            );

            return params;
        }
        static IglooController createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return IglooController(fromJSON(j, entityManager));
        }   
};