#pragma once

#include "Components.h"
#include "InputManager.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "IAComponents.h"
#include "TombotBootsController.h"
#include "effects/SmallExplosionComponent.h"


struct TombotControllerParameters {
    
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    HealthComponent* healthComponent;

    TombotControllerParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb, HealthComponent* hc):
    transform(t), entityManager(em), animator(ac), rigidbody(rb), healthComponent(hc) {}
};

struct TombotController : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    HealthComponent* healthComponent;

    ClassicIAComponent* iaComponent;
    std::shared_ptr<MegamanController> player;

    TombotController(TombotControllerParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidbody(params.rigidbody),
    healthComponent(params.healthComponent) {}

    bool direccion = false; // false = left, true = right
    float agroDistance = 300.0f; // Distancia de detección del jugador
    float activationDistance = 500.0f; // Distancia a la que se activa el enemigo
    float speed = 100.0f; // Velocidad de movimiento
    float yInicial = 0.0f; // Posición inicial en Y
    float deltaY = 100.0f; // Diferencia de posición en Y

    float turboTimer = 0.0f; // Temporizador para el turbo
    float turboDuration = 2.0f; // Duración del turbo
    bool isTurboActive = false; // Indica si el turbo está activo

    int numVecesTurn = 2; //Para repetir la animacion varias veces
    bool turnEnd = false;

    Collider* collider;

    void setDirection(bool dir){
        direccion = dir;
        if(direccion){
            animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
        }
        else{
            animator->spriteRenderer->flip = SDL_FLIP_NONE;
        }
    }

    void setup_ia(){
        iaComponent->setInitialState("Idle");

        iaComponent->registerAction("Idle", [this](EntityManager* em, float deltaTime) {
            //Debug::Log("Idle");
            return;
        });

        iaComponent->registerAction("Start", [this](EntityManager* em, float deltaTime) {
            //Debug::Log("Start");
            rigidbody->velocityY = -75.0f;
        });

        iaComponent->registerAction("Turbo", [this](EntityManager* em, float deltaTime) {
            //Debug::Log("Turbo");
            if(direccion){
                rigidbody->velocityX = 75.0f;
            }
            else{
                rigidbody->velocityX = -75.0f;
            }
        });

        iaComponent->registerAction("Fly", [this](EntityManager* em, float deltaTime) {
            //Debug::Log("Fly");
            fly(deltaTime);
        });

        iaComponent->registerAction("Turn", [this](EntityManager* em, float deltaTime) {
            //Debug::Log("Turn");
            animator->setBool("isTurn", true);
            fly(deltaTime);
        });

        iaComponent->registerAction("Death", [this](EntityManager* em, float deltaTime) {
            //Debug::Log("Death");
            death();
        });

        iaComponent->registerCondition("Idle", "Start", [this](EntityManager* em, float deltaTime) {
            //Debug::Log("Condition Idle");
            //Mirar a ver si el enemigo esta cerca
            Vector2D normal = {0.0, 0.0};
            float distance;
            if(player){
                normal.x = abs(transform->posX - player->getTransform()->posX);
                normal.y = abs(transform->posY - player->getTransform()->posY);
                distance = sqrt(normal.x * normal.x + normal.y * normal.y);
                if ( distance <= activationDistance){
                    return true;
                }
            }

            return false;
        });

        iaComponent->registerCondition("Start", "Turbo", [this](EntityManager* em, float deltaTime) {
            //Debug::Log("Start -> Turbo");
            if(yInicial - transform->posY >= deltaY){
                rigidbody->velocityY = 0.0f;
                isTurboActive = true;
                return true; 
            }
            return false;
        });

        iaComponent->registerCondition("Turbo", "Fly", [this](EntityManager* em, float deltaTime) {
            //Debug::Log("Turbo -> Fly");
            if(isTurboActive){
                turboTimer += deltaTime;
                if(turboTimer >= turboDuration){
                    isTurboActive = false;
                    turboTimer = 0.0f;
                    attack();
                    rigidbody->velocityX = 0.0f;
                    rigidbody->velocityY = 0.0f;
                    animator->setBool("isTurbo", false);
                    animator->setBool("isFlying", true);
                    return true; // Cambia a Fly si se cumple la condición
                }
            }
            return false;
        });

        iaComponent->registerCondition("Fly", "Turn", [this](EntityManager* em, float deltaTime) {
            //Debug::Log("Fly -> Turn");
            Vector2D normal = {0.0f, 0.0f};
            if(player){
                normal.x = player->getTransform()->posX - transform->posX;
                normal.y = player->getTransform()->posY - transform->posY;
            }
            
            //Megaman a la izquierda y nosotros a la derecha
            if(normal.x >= -agroDistance && normal.x <= 0 && animator->spriteRenderer->flip == SDL_FLIP_HORIZONTAL){
                animator->setBool("isTurning", true);
                animator->setBool("isFlying", false);
                return true;
            }

            //Megaman a la derecha y nosotros a la izquierda
            if(normal.x <= agroDistance && normal.x >= 0 && animator->spriteRenderer->flip == SDL_FLIP_NONE){
                animator->setBool("isTurning", true);
                animator->setBool("isFlying", false);
                return true;
            }
            return false; // Cambia a Turn1 si se cumple la condición
        });


        iaComponent->registerCondition("Turn", "Fly", [this](EntityManager* em, float deltaTime) {
            //Debug::Log("Turn -> Fly");
            if(turnEnd){
                turnEnd = false;
                return true; // Cambia a Fly si se cumple la condición
            }
            return false;
        });

        iaComponent->registerCondition("Start", "Death", [this](EntityManager* em, float deltaTime) {
            //Debug::Log("Turbo -> Death");
            return healthComponent->isDead();
        });

        iaComponent->registerCondition("Turbo", "Death", [this](EntityManager* em, float deltaTime) {
            //Debug::Log("Turbo -> Death");
            return healthComponent->isDead();
        });

        iaComponent->registerCondition("Fly", "Death", [this](EntityManager* em, float deltaTime) {
            //Debug::Log("Fly -> Death");
            return healthComponent->isDead();
        });

        iaComponent->registerCondition("Turn", "Death", [this](EntityManager* em, float deltaTime) {
            //Debug::Log("Turn -> Death");
            return healthComponent->isDead();
        });

    }

    void attack(){
        if (!PrefabManager::getInstance().hasPrefab("tombotboots")) {
            PrefabManager::getInstance().loadPrefab("tombotboots");
        }
        Entity tombotBoots = PrefabManager::getInstance().instantiatePrefab("tombotboots", *entityManager);
        Transform* tombotBootsTransform = entityManager->getComponent<Transform>(tombotBoots);
        TombotBootsController* tombotBootsComponent = entityManager->getComponent<TombotBootsController>(tombotBoots);
        if(tombotBootsTransform){
            tombotBootsTransform->posX = transform->posX;
            tombotBootsTransform->posY = transform->posY+17;
            tombotBootsComponent->start();
        }
    }

    void fly(float deltaTime){
        Vector2D normal = {0.0f, 0.0f};
        if(player){
            normal.x = player->getTransform()->posX - transform->posX;
            normal.y = player->getTransform()->posY - transform->posY;
        }

        if(normal.x >= -agroDistance && normal.x <= agroDistance){
            float length = sqrt(normal.x * normal.x + normal.y * normal.y);
            if (length > 0) {
                normal.x /= length;
                normal.y /= length;
            }
            normal.x = normal.x * speed * deltaTime;
            normal.y = normal.y * speed * deltaTime * 0.2f;
            transform->posX += normal.x;
            transform->posY += normal.y;
        }
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
    };
    

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

    void findPlayer(){
        if(!player) entityManager->findComponent<MegamanController>(player);
    }


    void start() override {

        collider = entityManager->getComponent<Collider>(entityManager->getLast());
        if (!collider) {
            throw std::runtime_error("TombotController requires a Collider component");
        }

        iaComponent = entityManager->getComponent<ClassicIAComponent>(parent);
        if (!iaComponent) {
            throw std::runtime_error("Tombot requires a ClassicIAComponent component");
        }

        animator->addFrameTrigger("turn2", 2, [this](){
            numVecesTurn--;
            if(numVecesTurn <= 0){
                if(animator->spriteRenderer->flip == SDL_FLIP_HORIZONTAL){
                    animator->spriteRenderer->flip = SDL_FLIP_NONE;
                } else {
                    animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
                }
                animator->setBool("isTurning", false);
                animator->setBool("isFlying", true);
                turnEnd = true;
                animator->setBool("isTurning2", false);
                animator->setBool("isFlying", true);
                numVecesTurn = 3;
            }
            
        });

        animator->addFrameTrigger("turn1", 2, [this](){
            numVecesTurn--;
            if(numVecesTurn <= 0){
                animator->setBool("isTurning", false);
                animator->setBool("isTurning2", true);
                numVecesTurn = 3;
            }
        });

        collider->onCollisionEnter = callbackCollisionEnter;

        setup_ia();
        yInicial = transform->posY;
    }

    void update(float deltaTime) override {
        // Implementar lógica de actualización aquí
        findPlayer();
    }
};



class TombotControllerLoader {
    public:

        static TombotControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("TombotController requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("TombotController requires a AnimatorComponent component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("TombotController requires a Rigidbody component");
            }

            HealthComponent* healthComponent = entityManager.getComponent<HealthComponent>(entityManager.getLast());
            if (!healthComponent) {
                throw std::runtime_error("TombotController requires a HealthComponent component");
            }

            TombotControllerParameters params(
                transform, 
                &entityManager, 
                animator, 
                rigidbody, 
                healthComponent
            );

            return params;
        }
    
        static TombotController createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return TombotController(fromJSON(j, entityManager));
        }
};