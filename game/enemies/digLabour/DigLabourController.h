#pragma once


#include "Components.h"
#include "InputManager.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "effects/SmallExplosionComponent.h"
#include "drops/DropController.h"
#include "IAComponents.h"
#include "DigLabourPickAxe.h"
#include <cmath>

struct DigLabourControllerParameters {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    HealthComponent* healthComponent;


    DigLabourControllerParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb, HealthComponent* hc):
    transform(t), entityManager(em), animator(ac), rigidbody(rb), healthComponent(hc) {}
};

//Dame solo es esquelo

struct DigLabourController : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    HealthComponent* healthComponent;

    ClassicIAComponent* iaComponent;
    std::shared_ptr<MegamanController> player;
    Collider* collider;

    bool haDadoGolpe = false;

    float agroDistance = 500.0f;

    float laughTimer = 0.0f;
    float laughCooldown = 1.5f;
    bool isLaughing = false;

    Vector2D velocityPickAxe = {0.0f, 0.0f};

    float attackTimer = 0.0f;
    float attackCooldown = 4.0f;
    bool puedoAtacar = false;


    DigLabourController(DigLabourControllerParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidbody(params.rigidbody),
    healthComponent(params.healthComponent) {}


    void setup_ia(){
        iaComponent->setInitialState("Idle");

        iaComponent->registerAction("Idle", [this](EntityManager* em, float deltaTime) {
            attackTimer += deltaTime;
            if(attackTimer >= attackCooldown){
                attackTimer = 0.0f;
                puedoAtacar = true;
            }
        });

        iaComponent->registerAction("Attack", [this](EntityManager* em, float deltaTime) {
        });
           
        iaComponent->registerAction("Laugh", [this](EntityManager* em, float deltaTime) {
            laughTimer += deltaTime;
            if(laughTimer >= laughCooldown){
                laughTimer = 0.0f;
                animator->setBool("isLaughing", false);
            }
        });

        iaComponent->registerAction("Death", [this](EntityManager* em, float deltaTime) {
            death();
        });

        iaComponent->registerCondition("Idle", "Attack", [this](EntityManager* em, float deltaTime) {
            // Debug::Log("Idle -> Attack");

            Vector2D normal = {0.0,0.0};
            if(player && puedoAtacar){
                normal.x = player->getTransform()->posX - transform->posX;
                normal.y = player->getTransform()->posY - transform->posY;
                /*if(player->getTransform()->posY > transform->posY - 25 && player->getTransform()->posY < transform->posY + 25){
                    
                }*/

                if(normal.x < 0 && normal.x > -agroDistance){
                    animator->spriteRenderer->flip = SDL_FLIP_NONE;
                    animator->setBool("isAttacking", true);
                    return true;
                } else if(normal.x > 0 && normal.x < agroDistance){
                    animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
                    animator->setBool("isAttacking", true);
                    return true;
                }
            }
            return false;
        });

        iaComponent->registerCondition("Idle", "Laugh", [this](EntityManager* em, float deltaTime) {
            if(haDadoGolpe){
                animator->setBool("isLaughing", true);
                haDadoGolpe = false;
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Attack", "Idle", [this](EntityManager* em, float deltaTime) {
            return !animator->getBool("isAttacking");
        });

        iaComponent->registerCondition("Laugh", "Idle", [this](EntityManager* em, float deltaTime) {
            return !animator->getBool("isLaughing");
        });

        iaComponent->registerCondition("Idle", "Death", [this](EntityManager* em, float deltaTime) {
            return healthComponent->isDead();
        });

        iaComponent->registerCondition("Attack", "Death", [this](EntityManager* em, float deltaTime) {
            return healthComponent->isDead();
        });

        iaComponent->registerCondition("Laugh", "Death", [this](EntityManager* em, float deltaTime) {
            return healthComponent->isDead();
        });
    }

    void findPlayer(){
        if(!player) entityManager->findComponent<MegamanController>(player);
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

    void generarPickAxe(){
        if (!PrefabManager::getInstance().hasPrefab("digLabourPickAxe")) {
            PrefabManager::getInstance().loadPrefab("digLabourPickAxe");
        }
        Entity axeEntity = PrefabManager::getInstance().instantiatePrefab("digLabourPickAxe", *entityManager);
        Transform* axeTransform = entityManager->getComponent<Transform>(axeEntity);
        DigLabourPickAxe* axeComponent = entityManager->getComponent<DigLabourPickAxe>(axeEntity);
        if(axeTransform){
            Vector2D posIni = {0.0,0.0};
            posIni.y = transform->posY -15;
            axeComponent->start();
            if(animator->spriteRenderer->flip == SDL_FLIP_NONE){
                posIni.x = transform->posX - 10;
            } else {
                posIni.x = transform->posX + 10;
            }
            axeComponent->setPosInicial(posIni);
            axeComponent->setCallBackParent(setHaDadoGolpe);
        }
    }
    

    std::function<void()> setHaDadoGolpe = [this]() {
        haDadoGolpe = true;
    };

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
    };

    void start() override{
        collider = entityManager->getComponent<Collider>(parent);

        collider->onCollisionEnter = callbackCollisionEnter;
        iaComponent = entityManager->getComponent<ClassicIAComponent>(parent);
        if(!iaComponent) {
            throw std::runtime_error("DigLabourController requires a ClassicIAComponent component");
        }

        setup_ia();

        animator->addFrameTrigger("attack", 4, [this](){
            animator->setBool("isAttacking", false);
            puedoAtacar = false;
            generarPickAxe();
        });

    }

    void update(float deltaTime) override {
        findPlayer();
    }

};


class DigLabourControllerLoader {
    public:
        static DigLabourControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("DigLabourController requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("DigLabourController requires a AnimatorComponent component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("DigLabourController requires a Rigidbody component");
            }

            HealthComponent* healthComponent = entityManager.getComponent<HealthComponent>(entityManager.getLast());
            if (!healthComponent) {
                throw std::runtime_error("DigLabourController requires a HealthComponent component");
            }

            DigLabourControllerParameters params(
                transform,
                &entityManager,
                animator,
                rigidbody,
                healthComponent
            );
            
            return params;
        }

        static DigLabourController createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return DigLabourController(fromJSON(j, entityManager));
        }
};