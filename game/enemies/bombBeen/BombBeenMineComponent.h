#ifndef BOMB_BEEN_MINE_COMPONENT_H
#define BOMB_BEEN_MINE_COMPONENT_H

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "effects/BigExplosionComponent.h"

struct BombBeenMineParameters{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    BombBeenMineParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb):
    transform(t), entityManager(em), animator(ac), rigidBody(rb) {}
};

struct BombBeenMineComponent : public Component{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    BombBeenMineComponent(BombBeenMineParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidBody(params.rigidBody) {}

    float explosionTime = 2.3f;
    float timeCounter = 0.0f;
    
    void start() override{
    }

    void update(float deltaTime) override{
        if(rigidBody->velocityY == 0 && !animator->getBool("isLanded")){
            animator->setBool("isLanded", true);
            rigidBody->velocityX = 0;
            rigidBody->velocityY = 0;
        }
        if(animator->getBool("isLanded")){
            timeCounter += deltaTime;
            if(timeCounter >= explosionTime){
                if(!PrefabManager::getInstance().hasPrefab("bigExplosion")){
                    PrefabManager::getInstance().loadPrefab("bigExplosion");
                }
                Entity entityExplosion = PrefabManager::getInstance().instantiatePrefab("bigExplosion", *entityManager);
                Transform* explosionTransform = entityManager->getComponent<Transform>(entityExplosion);
                BigExplosionComponent* explosionComponent = entityManager->getComponent<BigExplosionComponent>(entityExplosion);
                if(explosionTransform){
                    explosionTransform->posX = transform->posX;
                    explosionTransform->posY = transform->posY;
                }
                explosionComponent->start();
                entityManager->deleteEntity(parent);
            }
        }
    }
};

class BombBeenMineLoader{
    public:
        static BombBeenMineComponent fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("BombBeenMineComponent requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("BombBeenMineComponent requires a AnimatorComponent component");
            }

            Rigidbody* rigidBody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidBody) {
                throw std::runtime_error("BombBeenMineComponent requires a Rigidbody component");
            }

            BombBeenMineParameters params(
                transform,
                &entityManager,
                animator,
                rigidBody
            );

            return params;
        }
        
        static BombBeenMineComponent createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return BombBeenMineComponent(fromJSON(j, entityManager));
        }
}; 

#endif