#ifndef BLOCKCOMPONENT_H
#define BLOCKCOMPONENT_H

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"

struct BlockComponentParameters {

    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    HealthComponent* health;

    BlockComponentParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb, HealthComponent* hc):
    transform(t), entityManager(em), animator(ac), rigidbody(rb), health(hc) {}
};

struct BlockComponent : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    HealthComponent* health;

    Collider* collider;

    int position;
    bool enBase = true;
    int altura;

    std::function<void(int)> onDeath;

    BlockComponent(BlockComponentParameters params):
        transform(params.transform),
        entityManager(params.entityManager),
        animator(params.animator),
        rigidbody(params.rigidbody),
        health(params.health) {}

    void changeToSpike(){
        collider->setTag("Enemy");
        animator->setBool("hasSpikes", true);
    }

    void setOnDeath(std::function<void(int)> callback) {
        onDeath = callback;
    }

    void setPosition(int _pos){
        position = _pos;
    }

    void removeBlock() {
        if(enBase){
            onDeath(position);
        }
        entityManager->deleteEntity(parent);
    }
    void asignarAltura(int _altura){
        altura = _altura;
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

        if (health->isDead()) {
            removeBlock();
        }
    };

    void start() override {
        collider = entityManager->getComponent<Collider>(parent);
        collider->onCollisionEnter = callbackCollisionEnter;
    }

    void update(float deltaTime) override {
        //El caso del bloque qeu sube
        if(transform->posY <= altura && rigidbody->velocityY < 0){
            rigidbody->velocityY = 0;
            transform->posY = altura;
        }
        //El caso del bloque que baja
        else if(transform->posY >= altura && rigidbody->velocityY > 0){
            rigidbody->velocityY = 0;
            transform->posY = altura;
        }
    }
};

class BlockComponentLoader {
    public:
    
        static BlockComponentParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("BlockComponent requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("BlockComponent requires an AnimatorComponent component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("BlockComponent requires a Rigidbody component");
            }

            HealthComponent* health = entityManager.getComponent<HealthComponent>(entityManager.getLast());
            if (!health) {
                throw std::runtime_error("BlockComponent requires a HealthComponent component");
            }

            BlockComponentParameters params(
                transform,
                &entityManager,
                animator,
                rigidbody,
                health
            );

            return params;
        }

        static BlockComponent createFromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            return BlockComponent(fromJSON(j, entityManager));
        }
};


#endif