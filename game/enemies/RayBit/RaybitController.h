#ifndef RAYBITCONTROLLER_H
#define RAYBITCONTROLLER_H

#include "Components.h"
#include "InputManager.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "effects/SmallExplosionComponent.h"
#include "RaybitBulletComponent.h"
#include "IAComponents.h"
#include "drops/DropController.h"

struct RaybitControllerParameters {

    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    HealthComponent* health;

    RaybitControllerParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb, HealthComponent* hc):
    transform(t), entityManager(em), animator(ac), rigidBody(rb), health(hc) {}
};

struct RaybitController : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    HealthComponent* health;

    std::shared_ptr<MegamanController> player;
    ClassicIAComponent* iaComponent;

    float agroDistance = 250.0f;
    float speed = 100.0f;
    float jumpForce = 300.0f;
    bool inputActive = true; 
    bool direction = false; //TODO AUX

    bool hadShoot = true;

    bool suelo = true;

    float shootCooldown = 3.0f;
    float shootTimer = 0.0f;



    //Contructor del controller
    RaybitController(RaybitControllerParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidBody(params.rigidBody),
    health(params.health) {}

    std::function<void(Collider*, CollisionResult)> callbackCollisionEnter = [this](Collider* other, CollisionResult cd) {
        Debug::Log("Colision con " + other->tag);
        if (other->tag == "megamanBullet") {
            health->takeDamage(35);
        }
        if (other->tag == "megamanMediumBullet") {
            health->takeDamage(50);
        }
        if (other->tag == "megamanStrongBullet") {
            health->takeDamage(100);
        }
        Debug::Log("Vida" + std::to_string(health->getCurrentHealth()));
        if(health->isDead()){
            Debug::Log("MUERTE");
        }
    };


    //Funcion de inicio
    void start() override {
        Collider* collider = entityManager->getComponent<Collider>(parent);

        collider->onCollisionEnter = callbackCollisionEnter;
        iaComponent = entityManager->getComponent<ClassicIAComponent>(parent);
        if (!iaComponent) {
            throw std::runtime_error("RaybitController requires a ClassicIAComponent component");
        }


        animator->addFrameTrigger("shoot", 2, [this](){
            if (!PrefabManager::getInstance().hasPrefab("raybitBullet")) {
                PrefabManager::getInstance().loadPrefab("raybitBullet");
            }
            Entity bulletEntity = PrefabManager::getInstance().instantiatePrefab("raybitBullet", *entityManager);
            Transform* bulletTransform = entityManager->getComponent<Transform>(bulletEntity);
            RaybitBulletComponent* bulletComponent = entityManager->getComponent<RaybitBulletComponent>(bulletEntity);
            if(bulletTransform){
                bulletTransform->posX = transform->posX;
                bulletTransform->posY = transform->posY;
                if(direction){
                    bulletTransform->posX = this->transform->posX + 36;
                    bulletComponent->speed *= -1;
                } else {
                    bulletTransform->posX = this->transform->posX - 36;
                }
                bulletTransform->posY = this->transform->posY - 29;
                bulletComponent->start();
            }
        });
        animator->addFrameTrigger("shoot", 3, [this](){
            animator->setBool("isShooting", false);
        });
        animator->addFrameTrigger("startingJump", 2, [this](){
            animator->setBool("startingJump", false);
            animator->setBool("isJumping", true);
            if(direction){
                rigidBody->velocityX = speed;
                rigidBody->velocityY = -jumpForce;
            } else {
                rigidBody->velocityX = -speed;
                rigidBody->velocityY = -jumpForce;
            }
        });


        setup_ia();
    }

    void setup_ia()
    {
        iaComponent->setInitialState("Idle");

        iaComponent->registerAction("Idle", [this](EntityManager* entityManager, float deltaTime){
            inputActive = true;
        });


        iaComponent->registerAction("JumpRight", [this](EntityManager* entityManager, float deltaTime){
            if(inputActive) jumpRight();
        });

        iaComponent->registerAction("JumpLeft", [this](EntityManager* entityManager, float deltaTime){
            if(inputActive) jumpLeft();
        });

        iaComponent->registerAction("Shoot", [this](EntityManager* entityManager, float deltaTime){
            if(inputActive) shoot();
        });

        iaComponent->registerAction("Death", [this](EntityManager* entityManager, float deltaTime){
            death();
        });

        iaComponent->registerCondition("Idle", "JumpRight", [this](EntityManager* entityManager, float deltaTime){
            //Calculate normal between this entity and the player
            Vector2D normal = {0.0,0.0};
            if(player){
                normal.x = player->getTransform()->posX - transform->posX;
                normal.y = player->getTransform()->posY - transform->posY;
            }
            if(!suelo){
                return false;
            }

            //Check if the player is in the right side of the entity
            if(normal.x >= agroDistance){
                return true;
            }
            
            if(normal.x > 0 && hadShoot){
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Idle", "JumpLeft", [this](EntityManager* entityManager, float deltaTime){
            //Calculate normal between this entity and the player
            Vector2D normal = {0.0,0.0};
            if(player){
                normal.x = player->getTransform()->posX - transform->posX;
                normal.y = player->getTransform()->posY - transform->posY;
            }

            if(!suelo){
                return false;
            }
            //Check if the player is in the left side of the entity
            if(normal.x <= -agroDistance){
                return true;
            }
            
            if(normal.x <= 0 && hadShoot){
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Idle", "Shoot", [this](EntityManager* entityManager, float deltaTime){
            //Calculate normal between this entity and the player
            Vector2D normal = {0.0,0.0};
            if(player && !hadShoot){
                normal.x = player->getTransform()->posX - transform->posX;
                normal.y = player->getTransform()->posY - transform->posY;
                /*if(player->getTransform()->posY > transform->posY - 25 && player->getTransform()->posY < transform->posY + 25){
                    
                }*/

                if(normal.x < 0 && normal.x > -agroDistance && animator->spriteRenderer->flip == SDL_FLIP_NONE){
                    return true;
                } else if(normal.x > 0 && normal.x < agroDistance && animator->spriteRenderer->flip == SDL_FLIP_HORIZONTAL){
                    return true;
                }
            }
            return false;
        });

        iaComponent->registerCondition("JumpRight", "Idle", [this](EntityManager* entityManager, float deltaTime){
            if(rigidBody->velocityY > 0 && animator->getBool("isJumping")){
                animator->setBool("isFalling", true);
                animator->setBool("isJumping", false);
            }
            //Suelo
            else if(rigidBody->velocityY == 0 && animator->getBool("isFalling")){
                animator->setBool("isFalling", false);
                rigidBody->velocityX = 0;
                suelo = true;
                return true;
            }

            return false;
        });

        iaComponent->registerCondition("JumpLeft", "Idle", [this](EntityManager* entityManager, float deltaTime){
            if(rigidBody->velocityY > 0 && animator->getBool("isJumping")){
                animator->setBool("isFalling", true);
                animator->setBool("isJumping", false);
            }
            //Suelo
            else if(rigidBody->velocityY == 0 && animator->getBool("isFalling")){
                animator->setBool("isFalling", false);
                rigidBody->velocityX = 0;
                suelo = true;
                return true;
            }

            return false;
        });

        iaComponent->registerCondition("Shoot", "Idle", [this](EntityManager* entityManager, float deltaTime){
            if(!animator->getBool("isShooting")){
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Idle", "Death", [this](EntityManager* entityManager, float deltaTime){
            if(health->isDead()){
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("JumpRight", "Death", [this](EntityManager* entityManager, float deltaTime){
            if(health->isDead()){
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("JumpLeft", "Death", [this](EntityManager* entityManager, float deltaTime){
            if(health->isDead()){
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Shoot", "Death", [this](EntityManager* entityManager, float deltaTime){
            if(health->isDead()){
                return true;
            }
            return false;
        });
    }

    void findPlayer()
    {
        if(!player) entityManager->findComponent<MegamanController>(player);
    }

    void jumpLeft(){
        suelo = false;
        inputActive = false;
        direction = false; //TODO AUX

        animator->setBool("startingJump", true);
        animator->spriteRenderer->flip = SDL_FLIP_NONE;
    }

    void jumpRight(){
        suelo = false;
        inputActive = false;
        direction = true; //TODO AUX
        
        animator->setBool("startingJump", true);
        animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
    }

    void shoot(){
        hadShoot = true;
        inputActive = false;
        animator->setBool("isShooting", true);
    }

    void death()
    {
        Debug::Log("MUERTE");
        inputActive = false;
        if (!PrefabManager::getInstance().hasPrefab("smallExplosion")) {
            PrefabManager::getInstance().loadPrefab("smallExplosion");
        }
        Entity explosionEntity = PrefabManager::getInstance().instantiatePrefab("smallExplosion", *entityManager);
        Transform* explosionTransform = entityManager->getComponent<Transform>(explosionEntity);
        SmallExplosionComponent* explosionComponent = entityManager->getComponent<SmallExplosionComponent>(explosionEntity);
        if(explosionTransform){
            explosionTransform->posX = transform->posX;
            explosionTransform->posY = transform->posY;
        }
        //Borrar la explosion
        explosionComponent->start();

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

    //Funcion de actualizacion
    void update(float deltaTime) override {
        findPlayer();
        if(hadShoot) shootTimer += deltaTime;
        if(shootTimer >= shootCooldown){
            shootTimer = 0.0f;
            hadShoot = false;
        }
    }
};

class RaybitControllerLoader {
    public:
    
        static RaybitControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("RaybitController requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("RaybitController requires a AnimatorComponent component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("RaybitController requires a Rigidbody component");
            }

            HealthComponent* health = entityManager.getComponent<HealthComponent>(entityManager.getLast());
            if (!health) {
                throw std::runtime_error("RaybitController requires a HealthComponent component");
            }

            RaybitControllerParameters params(
                transform,
                &entityManager,
                animator,
                rigidbody,
                health
            );

            return params;
        }

        static RaybitController createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return RaybitController(fromJSON(j, entityManager));
        }
};

#endif
