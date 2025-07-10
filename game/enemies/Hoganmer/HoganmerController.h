#pragma once

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "effects/SmallExplosionComponent.h"
#include "IAComponents.h"
#include "enemies/Hoganmer/HoganmerBall.h"
#include "HealthComponent.h"
#include "PrefabManager.h"

struct HoganmerControllerParameters {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    HealthComponent* healthComponent;

    HoganmerControllerParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb, HealthComponent* hc):
    transform(t), entityManager(em), animator(ac), rigidbody(rb), healthComponent(hc){}
};

struct HoganmerController : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    HealthComponent* healthComponent;
    
    ClassicIAComponent* iaComponent;
    std::shared_ptr<MegamanController> player;
    
    HoganmerController(HoganmerControllerParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidbody(params.rigidbody),
    healthComponent(params.healthComponent) {}
    
    static const int numeroArma = 6;
    Collider* collider;
    HoganmerBall* vecArmas[numeroArma];
    
    bool puedeRecibirGolpe = false;
    float agroDistance = 200.0f;
    float cooldownAttack = 2.0f;
    float attackTimer = 0.0f;
    bool canAttack = false;
    float velocityAttack = 200.0f;
    float velocityStep = velocityAttack / numeroArma;
    
    void setup_ia() {
        iaComponent->setInitialState("Idle");
        
        iaComponent->registerAction("Idle", [this](EntityManager* em, float deltaTime) {
            attackTimer += deltaTime;
            if (attackTimer >= cooldownAttack) {
                canAttack = true;
                attackTimer = 0.0f;
            }
        });

        iaComponent->registerAction("Attack", [this](EntityManager* em, float deltaTime) {
        });

        iaComponent->registerAction("Death", [this](EntityManager* em, float deltaTime) {
            death();
        });

        iaComponent->registerCondition("Idle", "Attack", [this](EntityManager* em, float deltaTime) {
            Vector2D normal = {0.0,0.0};
            if(player && canAttack){
                normal.x = player->getTransform()->posX - transform->posX;
                normal.y = player->getTransform()->posY - transform->posY;
                /*if(player->getTransform()->posY > transform->posY - 25 && player->getTransform()->posY < transform->posY + 25){
                    
                }*/

                if(normal.x < 0 && normal.x > -agroDistance){
                    animator->spriteRenderer->flip = SDL_FLIP_NONE;
                    animator->setBool("isCharging", true);
                    return true;
                } else if(normal.x > 0 && normal.x < agroDistance){
                    animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
                    animator->setBool("isCharging", true);
                    return true;
                }
            }
            return false;
        });

        iaComponent->registerCondition("Attack", "Idle", [this](EntityManager* em, float deltaTime) {
            return (
                !animator->getBool("isAttacking") &&
                !animator->getBool("isCharging") &&
                !animator->getBool("isCollecting")
            );
        });

        iaComponent->registerCondition("Attack", "Death", [this](EntityManager* em, float deltaTime) {
            // Condition to transition from Attack to Death
            return healthComponent->isDead();
        });

        iaComponent->registerCondition("Idle", "Death", [this](EntityManager* em, float deltaTime) {
            // Condition to transition from Idle to Death
            return healthComponent->isDead();
        });
        
    }

    void instanciarArma(){
        for(int i = 0; i < numeroArma; i++){
            if (!PrefabManager::getInstance().hasPrefab("hoganmerBall")) {
                PrefabManager::getInstance().loadPrefab("hoganmerBall");
            }
            Entity parte = PrefabManager::getInstance().instantiatePrefab("hoganmerBall", *entityManager);
            vecArmas[i] = entityManager->getComponent<HoganmerBall>(parte);
            vecArmas[i]->setPosition(transform->posX, transform->posY);
            vecArmas[i]->setVelocity(0, 0);
            vecArmas[i]->desaparecer();
            if(i != 0){
                vecArmas[i]->setChain();
            }
            else{
                vecArmas[i]->setCallback(collect);
            }
        }

    }

    void aparecerArma(){
        for(int i = 0; i < numeroArma; i++){
            vecArmas[i]->aparecer();
        }
    }

    void desaparecerArma(){
        for(int i = 0; i < numeroArma; i++){
            vecArmas[i]->desaparecer();
        }
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

        for(int i = 0; i < numeroArma; i++){
            entityManager->deleteEntity(vecArmas[i]->parent);
        }
        
        entityManager->deleteEntity(parent);
    }

    void findPlayer(){
        if(!player) entityManager->findComponent<MegamanController>(player);
    }
    
    std::function<void()> collect = [this]() {
        animator->setBool("isAttacking", false);
        puedeRecibirGolpe = false;
        animator->setBool("isCollecting", true);
    };

    std::function<void(Collider*, CollisionResult)> callbackCollisionEnter = [this](Collider* other, CollisionResult cd) {
        if(!puedeRecibirGolpe) return;
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

    void atacar(){
        Vector2D normal = {0.0,0.0};
        normal.x = player->getTransform()->posX - transform->posX;
        normal.y = player->getTransform()->posY - transform->posY;
        // normalizar
        float length = sqrt(normal.x * normal.x + normal.y * normal.y);
        if (length > 0) {
            normal.x /= length;
            normal.y /= length;
        }
        float velocidadX;
        float velocidadY;
        for(int i = 0; i < numeroArma; i++){
            float velocidadX = normal.x * (velocityAttack - (i * velocityStep));
            float velocidadY = normal.y * (velocityAttack - (i * velocityStep));
            vecArmas[i]->setVelocity(velocidadX, velocidadY);
            if(animator->spriteRenderer->flip == SDL_FLIP_HORIZONTAL){
                vecArmas[i]->setPosition(transform->posX+30, transform->posY);
            }
            else{
                vecArmas[i]->setPosition(transform->posX-30, transform->posY);
            }
            vecArmas[i]->aparecer();
        }
    }

    void start() override {
        collider = entityManager->getComponent<Collider>(parent);
        if (!collider) {
            throw std::runtime_error("HoganmerController requires a Collider component");
        }

        collider->onCollisionEnter = callbackCollisionEnter;

        iaComponent = entityManager->getComponent<ClassicIAComponent>(parent);
        if (!iaComponent) {
            throw std::runtime_error("HoganmerController requires a ClassicIAComponent component");
        }

        animator->addFrameTrigger("charge", 3, [this]() {
            animator->setBool("isCharging", false);
            animator->setBool("isAttacking", true);
            atacar();
            puedeRecibirGolpe = true;
            canAttack = false;
        });

        animator->addFrameTrigger("collect", 0, [this](){
            for(int i = 0; i < numeroArma; i++){
                vecArmas[i]->setPosition(transform->posX, transform->posY);
                vecArmas[i]->setVelocity(0, 0);
                vecArmas[i]->desaparecer();
            }
        });

        animator->addFrameTrigger("collect", 2, [this]() {
            animator->setBool("isCollecting", false);
            
        });

        setup_ia();
        instanciarArma();
    }

    void update(float deltaTime) override {
        findPlayer();
    }


};


class HoganmerControllerLoader {
public:
    static HoganmerControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
        Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
        if (!transform) {
            throw std::runtime_error("HoganmerController requires a Transform component");
        }

        AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
        if (!animator) {
            throw std::runtime_error("HoganmerController requires a AnimatorComponent component");
        }

        Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
        if (!rigidbody) {
            throw std::runtime_error("HoganmerController requires a Rigidbody component");
        }

        HealthComponent* healthComponent = entityManager.getComponent<HealthComponent>(entityManager.getLast());
        if (!healthComponent) {
            throw std::runtime_error("HoganmerController requires a HealthComponent component");
        }

        HoganmerControllerParameters params(
            transform,
            &entityManager,
            animator,
            rigidbody,
            healthComponent
        );

        return params;
    }

    static HoganmerController createFromJSON(const nlohmann::json& j, EntityManager& entityManager) {
        HoganmerControllerParameters params = fromJSON(j, entityManager);
        return HoganmerController(params);
    }

};

