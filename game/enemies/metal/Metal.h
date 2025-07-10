#pragma once

#include "AnimatorComponent.h"
#include "Components.h"
#include "effects/SmallExplosionComponent.h"
#include "megaman/MegamanController.h"

struct MetalControllerParameters {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    HealthComponent* healthComponent;

    MetalControllerParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb, HealthComponent* hc):
    transform(t), entityManager(em), animator(ac), rigidbody(rb), healthComponent(hc){}
};

struct MetalController : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    HealthComponent* healthComponent;

    MetalController(MetalControllerParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidbody(params.rigidbody),
    healthComponent(params.healthComponent) {}

    Collider* collider;
    std::shared_ptr<MegamanController> player;

    float agroDistance = 300.0f;

    void death(){
        if (!PrefabManager::getInstance().hasPrefab("smallExplosion")) {
            PrefabManager::getInstance().loadPrefab("smallExplosion");
        }
        Entity entityExplosion = PrefabManager::getInstance().instantiatePrefab("smallExplosion", *entityManager);
        Transform* explosionTransform = entityManager->getComponent<Transform>(entityExplosion);
        SmallExplosionComponent* explosionComponent = entityManager->getComponent<SmallExplosionComponent>(entityExplosion);
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
        Debug::Log("Vida: " + std::to_string(healthComponent->getCurrentHealth()));
        if(healthComponent->isDead()){
            Debug::Log("Muerto");
        }
    };

    void findPlayer(){
        if(!player) entityManager->findComponent<MegamanController>(player);
    }

    void start() override {
        collider = entityManager->getComponent<Collider>(parent);
        collider->onCollisionEnter = callbackCollisionEnter;
        animator->addFrameTrigger("esconderse", 3, [this](){
            animator->setBool("desaparecer", false);
        });

        animator->addFrameTrigger("salir", 3, [this](){
            findPlayer();
            //Miramos en que lado esta el jugador
            Debug::Log("V1");
            if(player->getTransform()->posX > transform->posX){
                rigidbody->velocityX = 100.0f;
                animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
            }else{
                rigidbody->velocityX = -100.0f;
                animator->spriteRenderer->flip = SDL_FLIP_NONE;
            }
            animator->setBool("atacar", true);
        });
        
        animator->addFrameTrigger("esconderse", 0, [this](){
            Debug::Log("V0");
            rigidbody->velocityX = 0.0f;
        });
    }

    void update(float deltaTime) override {
        if(healthComponent->isDead()){
            death();
            return;
        }
        findPlayer();
        //Si cerca miramos -> nos mira nos quedamos quietos, no mira, atacamos
        if(!player) return;

        if(player->getTransform()->posX > transform->posX){
                animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
        }else{
            animator->spriteRenderer->flip = SDL_FLIP_NONE;
        }

        if(std::abs(player->getTransform()->posX - transform->posX) > agroDistance){
            animator->setBool("atacar", false);
            animator->setBool("desaparecer", true);
            animator->setBool("aparecer", false);
            Debug::Log("Fuera de rango");
            return;  
        } 
        
        if(player->getAnimator()->spriteRenderer->flip == animator->spriteRenderer->flip){
            Debug::Log("Me esta mirando");
            //Esconderse
            animator->setBool("atacar", false);
            animator->setBool("desaparecer", true);
            animator->setBool("aparecer", false);
            // rigidbody->velocityX = 0.0f;
            return;
        }
        Debug::Log("Slagoooo");
        animator->setBool("aparecer", true);
    }

};

class MetalControllerLoader {
    public:

        static MetalControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("MetalController requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("MetalController requires a AnimatorComponent component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("MetalController requires a Rigidbody component");
            }

            HealthComponent* healthComponent = entityManager.getComponent<HealthComponent>(entityManager.getLast());
            if (!healthComponent) {
                throw std::runtime_error("MetalController requires a HealthComponent component");
            }

            return MetalControllerParameters(transform, &entityManager, animator, rigidbody, healthComponent);
        }

        static MetalController createFromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            MetalControllerParameters params = fromJSON(j, entityManager);
            return MetalController(params);
        }
};