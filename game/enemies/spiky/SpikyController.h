#ifndef SPIKYCONTROLLER_H
#define SPIKYCONTROLLER_H

#include "Components.h"
#include "InputManager.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "effects/SmallExplosionComponent.h"

struct SpikyControllerParameters {

    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    HealthComponent* health;

    SpikyControllerParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb, HealthComponent* hc):
    transform(t), entityManager(em), animator(ac), rigidBody(rb), health(hc) {}
};

struct SpikyController : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    HealthComponent* health;

    float speed = 100.0f;
    int direction = 1; //1 = right, -1 = left
    bool estaCerca = false;

    std::shared_ptr<MegamanController> player;
    float activationDistance = 500.0f; // Distancia a la que se activa el enemigo

    void findPlayer() {
        if(!player) entityManager->findComponent<MegamanController>(player);
    }

    //Contructor del controller
    SpikyController(SpikyControllerParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidBody(params.rigidBody),
    health(params.health) {}

    //Funcion de inicio
    void start() override {

        //----------------------------------------------------------------------------------------
        // Definir colisiones
        //----------------------------------------------------------------------------------------
        Collider* collider = entityManager->getComponent<Collider>(parent);
        if (collider) {
            //Definir callback para onCollisionEnter
            std::function<void(Collider*, CollisionResult)> callbackCollisionEnter = [this](Collider* other, CollisionResult cd) {
                //Choque con una pared (cambiar de dirección)
                if ((other->tag == "Terrain" || other->tag == "Enemy") && (abs(cd.normal.x) > abs(cd.normal.y))) {     
                    direction *= -1;
                    animator->spriteRenderer->flip = (direction==1) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
                }

                //Balas del Megaman
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

                //Muerte
                if(health->isDead())  animator->setBool("die", true);
            };

            //Asignar callback
            collider->onCollisionEnter = callbackCollisionEnter;
        }

        //----------------------------------------------------------------------------------------
        // Callbacks animator
        //----------------------------------------------------------------------------------------
        animator->addFrameTrigger("die", 3, [this](){
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
        });


    }

    //Funcion de actualizacion
    void update(float deltaTime) override {
        findPlayer();
        if(player){
            Vector2D normal = {0.0, 0.0};
            float distance = 0.0f;
            normal.x = abs(transform->posX - player->getTransform()->posX);
            normal.y = abs(transform->posY - player->getTransform()->posY);
            distance = sqrt(normal.x * normal.x + normal.y * normal.y);
            if ( distance <= activationDistance){
                estaCerca = true;
            }
        }
        if(estaCerca){
            rigidBody->velocityX = speed * direction;
        }
    }
};


class SpikyControllerLoader {
    public:
    
        static SpikyControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("SpikyController requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("SpikyController requires a AnimatorComponent component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("SpikyController requires a Rigidbody component");
            }

            HealthComponent* health = entityManager.getComponent<HealthComponent>(entityManager.getLast());
            if (!health)  {
                throw std::runtime_error("SpikyController requires a Health component");
            }

            SpikyControllerParameters params(
                transform,
                &entityManager,
                animator,
                rigidbody,
                health
            );

            return params;
        }

        static SpikyController createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return SpikyController(fromJSON(j, entityManager));
        }
};

#endif
