#ifndef CHILLPENGUINPENGUINCONTROLLER_H
#define CHILLPENGUINPENGUINCONTROLLER_H

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"

struct ChillPenguinPenguinParameters{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    HealthComponent* health;

    ChillPenguinPenguinParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb, HealthComponent* hc):
    transform(t), entityManager(em), animator(ac), rigidBody(rb), health(hc) {}
};

struct ChillPenguinPenguinComponent : public Component{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    HealthComponent* health;
    AudioManager* audioManager;
    std::shared_ptr<MegamanController> player;

    ChillPenguinPenguinComponent(ChillPenguinPenguinParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidBody(params.rigidBody),
    health(params.health) {}

    float speed = 200.0f;
    bool movingRight = false;
    bool blizzard = false;

    void checkHealth(){
        if (health->isDead()){
            audioManager->playSound("Ice_break");
            entityManager->deleteEntity(parent);
        }
    }

    void start() override{

        audioManager = AudioManager::getInstance();

        audioManager->loadSound("Ice_break", "sound_effects/chill penguin/Ice Break.wav");
        Collider* collider = entityManager->getComponent<Collider>(parent);
        if (collider) {
            std::function<void(Collider*, CollisionResult)> callbackCollisionEnter = [this](Collider* other, CollisionResult cd) {
                if (other->tag == "megamanBullet") health->takeDamage(35);
                if (other->tag == "megamanMediumBullet") health->takeDamage(50);
                if (other->tag == "megamanStrongBullet") health->takeDamage(100);
            };
            collider->onCollisionEnter = callbackCollisionEnter;
        }

        if (!movingRight){
            animator->spriteRenderer->flip = SDL_FLIP_NONE;
        }
        else {
            animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
        }
        rigidBody->velocityX = 0;
        rigidBody->velocityY = 0;

    }

    void update(float deltaTime) override{

      checkHealth();  

      if (blizzard){
          checkHealth();
          if (!movingRight){
              rigidBody->velocityX = -speed;
          }
          else {
              rigidBody->velocityX = speed;
          }
      }
      else {
        rigidBody->velocityX = 0;
      }
    }
};

class ChillPenguinPenguinComponentLoader{
    public:
        static ChillPenguinPenguinComponent fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("ChillPenguinPenguinComponent requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("ChillPenguinPenguinComponent requires a AnimatorComponent component");
            }

            Rigidbody* rigidBody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidBody) {
                throw std::runtime_error("ChillPenguinPenguinComponent requires a Rigidbody component");
            }

            HealthComponent* health = entityManager.getComponent<HealthComponent>(entityManager.getLast());
            if (!health) {
                throw std::runtime_error("ChillPenguinPenguinComponent requires a HealthComponent component");
            }

            ChillPenguinPenguinParameters params(
                transform,
                &entityManager,
                animator,
                rigidBody,
                health
            );

            return params;
        }

        static ChillPenguinPenguinComponent createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return ChillPenguinPenguinComponent(fromJSON(j, entityManager));
        }
};



#endif
