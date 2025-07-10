#pragma once

#include "Components.h"
#include "InputManager.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "effects/SmallExplosionComponent.h"
#include "drops/DropController.h"


struct ScrapControllerParameters {

    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    HealthComponent* healthComponent;


    ScrapControllerParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb, HealthComponent* hc):
    transform(t), entityManager(em), animator(ac), rigidbody(rb), healthComponent(hc){}
};

struct ScrapController : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    HealthComponent* healthComponent;

    ClassicIAComponent* iaComponent;
    std::shared_ptr<MegamanController> player;
    Collider* collider;

    ScrapController(ScrapControllerParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidbody(params.rigidbody),
    healthComponent(params.healthComponent) {}


    void setup_ia(){
        iaComponent->setInitialState("Idle");

        iaComponent->registerAction("Idle", [this](EntityManager* entityManager, float deltaTime){
            // Debug::Log("Action Idle");
            
        });


        iaComponent->registerAction("Death", [this](EntityManager* entityManager, float deltaTime){
            // Debug::Log("Action Death");
            death();
        });

        iaComponent->registerCondition("Idle", "Death", [this](EntityManager* entityManager, float deltaTime){
            // Debug::Log("Idle -> Death");
            return (healthComponent->isDead());
        });

    }

    void death(){
        // Debug::Log("MUERTE");
        if (!PrefabManager::getInstance().hasPrefab("smallExplosion")) {
            PrefabManager::getInstance().loadPrefab("smallExplosion");
        }
        Entity explosionEntity = PrefabManager::getInstance().instantiatePrefab("smallExplosion", *entityManager);
        Transform* explosionTransform = entityManager->getComponent<Transform>(explosionEntity);
        SmallExplosionComponent* explosionComponent = entityManager->getComponent<SmallExplosionComponent>(explosionEntity);
        if(explosionTransform){
            explosionTransform->posX = transform->posX;
            explosionTransform->posY = transform->posY;
            explosionComponent->start();
        }

        //Scamos drop
        if (!PrefabManager::getInstance().hasPrefab("drop")) {
            PrefabManager::getInstance().loadPrefab("drop");
        }
        Entity dropEntity = PrefabManager::getInstance().instantiatePrefab("drop", *entityManager);
        Transform* dropTransform = entityManager->getComponent<Transform>(dropEntity);
        DropController* dropController = entityManager->getComponent<DropController>(dropEntity);
        if(dropTransform){
            dropTransform->posX = transform->posX;
            dropTransform->posY = transform->posY;
        }
        dropController->start();
        
        entityManager->deleteEntity(parent);
    }


    std::function<void(Collider*, CollisionResult)> callbackCollisionEnter = [this](Collider* other, CollisionResult cd) {
        if (other->tag == "megamanBullet") {
            healthComponent->takeDamage(35);
        }
        if (other->tag == "megamanMediumBullet") {
            healthComponent->takeDamage(50);
        }
        if (other->tag == "megamanStrongBullet") {
            healthComponent->takeDamage(100);
        }
        if (other->tag == "Terrain" && !animator->getBool("isLanded")) {
            animator->setBool("isLanded", true);
            animator->setBool("isCrawling", true);
            collider->scaleY = 37.0f;
        }
    };

    void start() override{

        iaComponent = entityManager->getComponent<ClassicIAComponent>(parent);
        if (!iaComponent) {
            throw std::runtime_error("Axemax requires a ClassicIAComponent component");
        }

        setup_ia();

        collider = entityManager->getComponent<Collider>(parent);

        collider->onCollisionEnter = callbackCollisionEnter;
    }

    void update(float deltaTime) override {

    }
};

class ScrapControllerLoader {
    public:

        static ScrapControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("ScrapController requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("ScrapController requires a AnimatorComponent component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("ScrapController requires a Rigidbody component");
            }

            HealthComponent* healthComponent = entityManager.getComponent<HealthComponent>(entityManager.getLast());
            if (!healthComponent) {
                throw std::runtime_error("ScrapController requires a HealthComponent component");
            }

            ScrapControllerParameters params(
                transform,
                &entityManager,
                animator,
                rigidbody,
                healthComponent
            );

            return params;

        }

        static ScrapController createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return ScrapController(fromJSON(j, entityManager));
        }

};
