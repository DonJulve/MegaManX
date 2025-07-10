#ifndef SNOWSHOOTERCONTROLLER_H
#define SNOWSHOOTERCONTROLLER_H

#include "Components.h"
#include "InputManager.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "effects/SmallExplosionComponent.h"
#include "SnowShooterBulletComponent.h"

struct SnowShooterControllerParameters {

    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    HealthComponent* health;

    SnowShooterControllerParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb, HealthComponent* hc):
    transform(t), entityManager(em), animator(ac), rigidBody(rb), health(hc) {}
};

struct SnowShooterController : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    HealthComponent* health;

    ClassicIAComponent* iaComponent;

    std::shared_ptr<MegamanController> player;

    bool inputActive = false; 
    float agroDistanceX = 200.0f;
    float agroDistanceY = 50.0f;
    bool suelo = false;

    bool hasShoot = false;
    float shootTime = 0.0f;
    float shootCooldown = 5.0f;

    Vector2D posMegaman;


    //Contructor del controller
    SnowShooterController(SnowShooterControllerParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidBody(params.rigidBody),
    health(params.health) {}

    void findPlayer(){
        if(!player) entityManager->findComponent<MegamanController>(player);
    }

    void setup_ia(){
        iaComponent->setInitialState("Idle");

        iaComponent->registerAction("Idle", [this](EntityManager* entityManager, float deltaTime){
            inputActive = true;
        });

        iaComponent->registerAction("Shoot", [this](EntityManager* entityManager, float deltaTime){
            if(inputActive) shoot();
        });

        iaComponent->registerAction("Death", [this](EntityManager* entityManager, float deltaTime){

            death();
        });

        iaComponent->registerCondition("Idle", "Shoot", [this](EntityManager* entityManager, float deltaTime){
            Vector2D normal = {0.0,0.0};

            if(player){
                normal.x = transform->posX - player->getTransform()->posX;
                normal.y = transform->posY - player->getTransform()->posY;
            }

            if(hasShoot){
                return false;
            }

            if(normal.y <= agroDistanceY && normal.y >= -agroDistanceY){
                return false;
            }

            if(normal.x <= agroDistanceX && normal.x >= 0){ //Megaman a la izquierda
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
            return health->isDead();
        });

        iaComponent->registerCondition("Shoot", "Death", [this](EntityManager* entityManager, float deltaTime){
            return health->isDead();
        });
    }

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
        if(other->tag == "Terrain"){
            suelo = true;
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
        //Disparooooo
        animator->addFrameTrigger("shoot", 6, [this](){
            if (!PrefabManager::getInstance().hasPrefab("snowshooterBullet")) {
                PrefabManager::getInstance().loadPrefab("snowshooterBullet");
            }
            Entity bulletEntity = PrefabManager::getInstance().instantiatePrefab("snowshooterBullet", *entityManager);
            Transform* bulletTransform = entityManager->getComponent<Transform>(bulletEntity);
            SnowShooterBulletComponent* bulletComponent = entityManager->getComponent<SnowShooterBulletComponent>(bulletEntity);
            if(bulletTransform){
                bulletTransform->posX = transform->posX;
                bulletTransform->posY = transform->posY;

                bulletTransform->posX = this->transform->posX - 20;
                bulletTransform->posY = this->transform->posY - 13;

                bulletComponent->start();
                animator->setBool("isShooting", false);
                bulletComponent->setDirection(posMegaman);
            }
        });

        animator->addFrameTrigger("shoot", 2, [this](){
            findPlayer();
            if(player){
                posMegaman.x = player->getTransform()->posX;
                posMegaman.y = player->getTransform()->posY;
            }
        });
        setup_ia();
    }

    void shoot(){
        inputActive = false;
        hasShoot = true;
        animator->setBool("isShooting", true);
    }

    void death(){
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

        if(hasShoot){
            shootTime += deltaTime;
            if(shootTime >= shootCooldown){
                hasShoot = false;
                shootTime = 0.0f;
            }
        }
    }
};

class SnowShooterControllerLoader {
    public:
    
        static SnowShooterControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("SnowShooterController requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("SnowShooterController requires a AnimatorComponent component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("SnowShooterController requires a Rigidbody component");
            }

            HealthComponent* health = entityManager.getComponent<HealthComponent>(entityManager.getLast());
            if (!health) {
                throw std::runtime_error("SnowShooterController requires a HealthComponent component");
            }

            SnowShooterControllerParameters params(
                transform,
                &entityManager,
                animator,
                rigidbody,
                health
            );

            return params;
        }

        static SnowShooterController createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return SnowShooterController(fromJSON(j, entityManager));
        }
};

#endif
