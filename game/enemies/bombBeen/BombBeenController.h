#ifndef BOMBBEEN_H
#define BOMBBEEN_H

#include "Components.h"
#include "InputManager.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "effects/SmallExplosionComponent.h"
#include "BombBeenMineComponent.h"

struct BombBeenControllerParameters {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    HealthComponent* healt;

    BombBeenControllerParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb, HealthComponent* hc):
    transform(t), entityManager(em), animator(ac), rigidBody(rb), healt(hc) {}

};

struct BombBeenController : public Component{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    HealthComponent* health;

    ClassicIAComponent* iaComponent;
    std::shared_ptr<MegamanController> player;

    bool inputEnabled = true;
    bool mineCreated = false;

    int vida = 5;

    int minasLanzadas = 0;
    const int minasMaximas = 3;

    bool atacando = false;
    float shootCooldown = 1.0f;
    float shootTimer = 0.0f;

    float detectionDistance = 450.0f; //Distancia para detectar al megaman

    float agroDistnace = 150.0f;
    bool haDisparado = false;
    float coolDownAtaque = 10.0f;
    float coolDownTimer = 0.0f;

    BombBeenController(BombBeenControllerParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidBody(params.rigidBody),
    health(params.healt) {}

    void setup_ia(){
        Debug::Log("Setup IA");
        iaComponent->setInitialState("Idle");

        iaComponent->registerAction("Idle", [this](EntityManager* entityManager, float deltaTime){
            //Debug::Log("Action Idle");
            rigidBody->velocityX = 0;
        });

        iaComponent->registerAction("Fly", [this](EntityManager* entityManager, float deltaTime){
            //Debug::Log("Action Fly");
            rigidBody->velocityX = -150.0f;
        });

        iaComponent->registerAction("Shoot", [this](EntityManager* entityManager, float deltaTime){
            //Debug::Log("Action SHoot");
            rigidBody->velocityX = 0;
            shoot();
        });

        iaComponent->registerAction("Death", [this](EntityManager* entityManager, float deltaTime){
            //Debug::Log("Action Death");
            death();
        });

        iaComponent->registerCondition("Idle", "Fly", [this](EntityManager* entityManager, float deltaTime){
            //Debug::Log("Idle -> Fly");
            if(player){
                if(transform->posX - player->getTransform()->posX <= detectionDistance){
                    return true;
                }
            }
            return false;
        });

        iaComponent->registerCondition("Fly", "Shoot", [this](EntityManager* entityManager, float deltaTime){
            //Debug::Log("Fly -> Shoot");
            if(player){
                if(transform->posX - player->getTransform()->posX <= agroDistnace && !haDisparado){
                    return true;
                }
            }
            return false;
        });

        iaComponent->registerCondition("Fly", "Death", [this](EntityManager* entityManager, float deltaTime){
            //Debug::Log("Fly -> Death");
            if(health->isDead()){
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Shoot", "Fly", [this](EntityManager* entityManager, float deltaTime){
            //Debug::Log("Shoot -> Fly");
            return minasLanzadas >= minasMaximas;
        });

        iaComponent->registerCondition("Shoot", "Death", [this](EntityManager* entityManager, float deltaTime){
            //Debug::Log("Shoot -> Death");
            if(health->isDead()){
                return true;
            }
            return false;
        });
        Debug::Log("Setup IA Done");
    }

    void crearMina(float velocidadX){
        if (!PrefabManager::getInstance().hasPrefab("bombbeenmine")) {
            PrefabManager::getInstance().loadPrefab("bombbeenmine");
        }
        Entity entityMine = PrefabManager::getInstance().instantiatePrefab("bombbeenmine", *entityManager);
        Transform* mineTransform = entityManager->getComponent<Transform>(entityMine);
        BombBeenMineComponent* mineComponent = entityManager->getComponent<BombBeenMineComponent>(entityMine);
        if(mineTransform){
            mineTransform->posX = transform->posX;
            mineTransform->posY = transform->posY + 20;
            mineComponent->start();
            mineComponent->rigidBody->velocityX = velocidadX;
        }
        minasLanzadas++;
    }

    void shoot(){
        haDisparado = true;
        animator->setBool("isCharging", true);
        if(!animator->getBool("isShooting")){
            return;
        }
        atacando = true;
        findPlayer();
        if(!player){
            return;
        }
        bool derecha = player->getTransform()->posX > transform->posX;
        if(minasLanzadas == 0){
            crearMina(derecha ? 50 : -50);
            return;
        }
        if(minasLanzadas >= minasMaximas){
            atacando = false;
            minasLanzadas = 0;
            shootTimer = 0.0f;
            return;
        }
        //Lanzada minimo una mina y no se ha acabado
        if(shootTimer >= shootCooldown){
            crearMina(derecha ? 50 * (minasLanzadas+1) : -50 * (minasLanzadas+1));
            shootTimer = 0.0f;
        }
        return;

    }
    
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
            health->takeDamage(35);
        }
        if (other->tag == "megamanMediumBullet") {
            health->takeDamage(50);
        }
        if (other->tag == "megamanStrongBullet") {
            health->takeDamage(100);
        }
        Debug::Log("Vida: " + std::to_string(health->getCurrentHealth()));
        if(health->isDead()){
            Debug::Log("Muerto");
        }
    };

    void findPlayer()
    {
        if(!player) entityManager->findComponent<MegamanController>(player);
    }

    void start() override{
        Collider* collider = entityManager->getComponent<Collider>(parent);
        collider->onCollisionEnter = callbackCollisionEnter;
        iaComponent = entityManager->getComponent<ClassicIAComponent>(parent);
        if (!iaComponent) {
            throw std::runtime_error("BombBeen requires a ClassicIAComponent component");
        }

        animator->addFrameTrigger("charge", 3, [this](){
            animator->setBool("isCharging", false);
            animator->setBool("isShooting", true);
        });

        setup_ia();
    }

    void update(float deltaTime) override{
        findPlayer();
        if(atacando) shootTimer += deltaTime;
        if(haDisparado) coolDownTimer += deltaTime;
        if(coolDownTimer >= coolDownAtaque){
            haDisparado = false;
            coolDownTimer = 0.0f;
        }
    }
        

};

class BombBeenControllerLoader {
    public:
    
        static BombBeenControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("BombBeenController requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("BombBeenController requires a AnimatorComponent component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("BombBeenController requires a Rigidbody component");
            }

            HealthComponent* health = entityManager.getComponent<HealthComponent>(entityManager.getLast());
            if (!health) {
                throw std::runtime_error("BombBeenController requires a HealthComponent component");
            }

            BombBeenControllerParameters params(
                transform,
                &entityManager,
                animator,
                rigidbody,
                health
            );

            return params;
        }

        static BombBeenController createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return BombBeenController(fromJSON(j, entityManager));
        }
};


#endif