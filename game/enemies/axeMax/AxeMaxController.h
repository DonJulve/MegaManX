#ifndef AXEMAXCONTROLLER_H
#define AXEMAXCONTROLLER_H

#include "Components.h"
#include "InputManager.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "BaseComponent.h"
#include "effects/SmallExplosionComponent.h"
#include "BlockComponent.h"
#include "IAComponents.h"


struct AxeMaxControllerParameters {

    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    HealthComponent* healthComponent;


    AxeMaxControllerParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb, HealthComponent* hc):
    transform(t), entityManager(em), animator(ac), rigidbody(rb), healthComponent(hc){}
};


struct AxeMaxController : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    HealthComponent* healthComponent;

    ClassicIAComponent* iaComponent;
    std::shared_ptr<MegamanController> player;

    AxeMaxController(AxeMaxControllerParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidbody(params.rigidbody),
    healthComponent(params.healthComponent) {}

    Collider* collider;
    BaseComponent* baseComponent;
    bool haDadoGolpe = false;
    float tiempoTranscurrido = 0.0f;
    float tiempoRecarga = 2.0f;
    float agroDistance = 400.0f;
    float cooldownAttack = 4.0f;
    float tiempoAtaque = 0.0f;


    bool hayBloques = true;
    bool inputActive = true;
    bool primerAtaque = true;
    bool finAtque = false;


    void instanciarBase(){
        
        if (!PrefabManager::getInstance().hasPrefab("baseAxeMax")) {
            PrefabManager::getInstance().loadPrefab("baseAxeMax");
        }
        Entity baseEntity = PrefabManager::getInstance().instantiatePrefab("baseAxeMax", *entityManager);
        baseComponent = entityManager->getComponent<BaseComponent>(baseEntity);
        Transform* baseTransform = baseComponent->transform;
        if(baseTransform){
            baseTransform->posX = transform->posX - 78;
            baseTransform->posY = transform->posY + 30;
        }
    }

    void setup_ia()
    {
        //Debug::Log("Setting up IA");

        iaComponent->setInitialState("Charge");

        iaComponent->registerAction("Idle", [this](EntityManager* entityManager, float deltaTime){
            // Debug::Log("Action Idle");
            inputActive = true;
            animator->setBool("isAttacking", false);
            animator->setBool("isLaughing", false);
        });

        iaComponent->registerAction("Attack", [this](EntityManager* entityManager, float deltaTime){
            // Debug::Log("Action Attack");
            if(inputActive) attack();
        });

        iaComponent->registerAction("Charge", [this](EntityManager* entityManager, float deltaTime){
            // Debug::Log("Action Charge");
            if(inputActive) charge();
        });

        iaComponent->registerAction("Laugh", [this](EntityManager* entityManager, float deltaTime){
            // Debug::Log("Action Laugh");
            inputActive = true;
            laugh();
        });

        iaComponent->registerAction("Death", [this](EntityManager* entityManager, float deltaTime){
            // Debug::Log("Action Death");
            death();
        });


        iaComponent->registerCondition("Idle", "Attack", [this](EntityManager* entityManager, float deltaTime){
            // Debug::Log("Idle -> Attack");
            Vector2D normal = {0.0,0.0};
            if(player){
                normal.x = transform->posX - player->getTransform()->posX;
                normal.y = transform->posY - player->getTransform()->posY;
            }

            //We only check left side
            if(normal.x <= agroDistance && baseComponent->bloquesColocados()){
                return true;
            }
            if(!primerAtaque && baseComponent->hayBloques() && tiempoAtaque >= cooldownAttack){
                tiempoAtaque = 0.0f;
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Idle", "Charge", [this](EntityManager* entityManager, float deltaTime){
            // Debug::Log("Idle -> Charge");
            if(tiempoTranscurrido >= tiempoRecarga){
                tiempoTranscurrido = 0.0f;
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Laugh", "Charge", [this](EntityManager* entityManager, float deltaTime){
            // Debug::Log("Laugh -> Charge");
            if(tiempoTranscurrido >= tiempoRecarga){
                tiempoTranscurrido = 0.0f;
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Charge", "Idle", [this](EntityManager* entityManager, float deltaTime){
            // Debug::Log("Charge -> Idle");
            if(baseComponent->bloquesColocados()){
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Attack", "Idle", [this](EntityManager* entityManager, float deltaTime){
            return finAtque;
        });

        iaComponent->registerCondition("Idle", "Laugh", [this](EntityManager* entityManager, float deltaTime){
            return (haDadoGolpe && !baseComponent->hayBloques());
        });

        iaComponent->registerCondition("Idle", "Death", [this](EntityManager* entityManager, float deltaTime){
            //Debug::Log("Idle -> Death");
            if(healthComponent->isDead()){
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Attack", "Death", [this](EntityManager* entityManager, float deltaTime){
            //Debug::Log("Attack -> Death");
            if(healthComponent->isDead()){
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Charge", "Death", [this](EntityManager* entityManager, float deltaTime){
            //Debug::Log("Charge -> Death");
            if(healthComponent->isDead()){
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Laugh", "Death", [this](EntityManager* entityManager, float deltaTime){
            //Debug::Log("Laugh -> Death");
            if(healthComponent->isDead()){
                return true;
            }
            return false;
        });


        // Debug::Log("IA set up");
    }

    void findPlayer(){
        if(!player) entityManager->findComponent<MegamanController>(player);
    }
    
    void attack(){
        inputActive = false;
        finAtque = false;
        animator->setBool("isAttacking", true);
        primerAtaque = !primerAtaque;
    }

    void charge(){
        //Create both blocks(2)
        inputActive = false;
        baseComponent->crearBloque();
        baseComponent->crearBloque();
        hayBloques = true;
        haDadoGolpe = false;
    }

    void laugh(){
        animator->setBool("isLaughing", true);
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
        Debug::Log("Collision with " + other->tag);
        Debug::Log("Health: " + std::to_string(healthComponent->getCurrentHealth()));
        if(healthComponent->isDead()){
            Debug::Log("Dead");
        }
    };

    void start() override {
        instanciarBase();
        collider = entityManager->getComponent<Collider>(parent);

        collider->onCollisionEnter = callbackCollisionEnter;

        iaComponent = entityManager->getComponent<ClassicIAComponent>(parent);
        if (!iaComponent) {
            throw std::runtime_error("Axemax requires a ClassicIAComponent component");
        }

        animator->addFrameTrigger("attack", 3, [this](){
            baseComponent->attack();
        });

        animator->addFrameTrigger("attack", 7, [this](){
            finAtque = true;
            // Debug::Log("Fin ataque true");
        });

        animator->addFrameTrigger("attack", 8, [this](){
            animator->setBool("isAttacking", false);
            // Debug::Log("Fin animacion ataque");
        });

        setup_ia();
    }

    void update(float deltaTime) override {
        findPlayer();
        if(!baseComponent->bloquesLenos()){
            tiempoTranscurrido += deltaTime;
        }
        if(!primerAtaque) {
            tiempoAtaque += deltaTime;
        }
    }
};

class AxeMaxControllerLoader {
    public:
    
        static AxeMaxControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("AxeMaxController requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("AxeMaxController requires a AnimatorComponent component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("AxeMaxController requires a Rigidbody component");
            }

            HealthComponent* healthComponent = entityManager.getComponent<HealthComponent>(entityManager.getLast());
            if (!healthComponent) {
                throw std::runtime_error("AxeMaxController requires a HealthComponent component");
            }

            AxeMaxControllerParameters params(
                transform,
                &entityManager,
                animator,
                rigidbody,
                healthComponent
            );

            return params;
        }

        static AxeMaxController createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return AxeMaxController(fromJSON(j, entityManager));
        }
};

#endif
