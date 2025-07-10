#pragma once

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include <random>

struct DropControllerParameters {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    DropControllerParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb):
    transform(t), entityManager(em), animator(ac), rigidBody(rb) {}
};

struct DropController : public Component{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    Collider* collider;
    AudioManager* audioManager;
    
    std::shared_ptr<MegamanController> player;

    std::random_device rd;
    std::mt19937 generator;
    std::uniform_real_distribution<float> dis;

    DropController(DropControllerParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidBody(params.rigidBody) {
        generator = std::mt19937(rd());
    }

    bool isDropped = false;
    float dropTime = 0.0f;
    float dropTimeLimit = 7.0f;
    float startBlink = 2.0f;
    bool isBlinking = false;
    float blinkTime = 0.0f;
    float blinkTimeLimit = 0.09f;
    bool blink = false;

    bool smallLife = false;
    float smallLifeValue = 0.4f;

    bool bigLife = false;
    float bigLifeValue = 0.27f;

    bool smallAmmo = false;
    float smallAmmoValue = 0.f;

    bool bigAmmo = false;
    float bigAmmoValue = 0.0f;

    bool heart = false;
    float heartValue = 0.0f;

    bool extraLife = false;
    float extraLifeValue = 0.05f;

    std::function<void(Collider*, CollisionResult)> onCollisionEnter = [this](Collider* other, CollisionResult result) {
        if(other->getTag() == "Player"){
            audioManager->playSound("drop");
            findPlayer();
            if(!player) return;
            if(smallLife){
                player->addHealth(30.0f);

            } else if(bigLife){
                player->addHealth(75.0f);
            } else if(smallAmmo){

            } else if(bigAmmo){

            } else if(heart){

            } else if(extraLife){

            }
            entityManager->deleteEntity(parent);
        }
    };
    
    void findPlayer(){
        if(!player) entityManager->findComponent<MegamanController>(player);
    }

    void start() override {
        audioManager = AudioManager::getInstance();
        audioManager->loadSound("drop", "sound_effects/megaman/LifeGain.wav");
        float random = dis(generator);
        collider = entityManager->getComponent<Collider>(parent);
        collider->onCollisionEnter = onCollisionEnter;
        Debug::Log("Random: " + std::to_string(random));
        if (random < smallLifeValue) {
            smallLife = true;
            animator->setBool("isSmallLife", true);
        } else if (random < smallLifeValue + bigLifeValue) {
            bigLife = true;
            animator->setBool("isBigLife", true);
        } else if (random < smallLifeValue + bigLifeValue + smallAmmoValue) {
            smallAmmo = true;
            animator->setBool("isSmallAmmo", true);
        } else if (random < smallLifeValue + bigLifeValue + smallAmmoValue + bigAmmoValue) {
            bigAmmo = true;
            animator->setBool("isBigAmmo", true);
        } else if (random < smallLifeValue + bigLifeValue + smallAmmoValue + bigAmmoValue + heartValue) {
            heart = true;
            animator->setBool("isHeart", true);
        } else if (random < smallLifeValue + bigLifeValue + smallAmmoValue + bigAmmoValue + heartValue + extraLifeValue) {
            extraLife = true;
            animator->setBool("isExtraLife", true);
        }
        else {
            //Si no es ninguna destruimos entidad
            entityManager->deleteEntity(parent);
        }
        isDropped = true;
    }

    void update(float deltaTime) override {
        if(isDropped){
            dropTime += deltaTime;
            if(dropTime >= dropTimeLimit){
                entityManager->deleteEntity(parent);
            }
        }
    }
};

class DropControllerLoader {
    public:
    
        static DropControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("DropController requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("DropController requires a AnimatorComponent component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("DropController requires a Rigidbody component");
            }

            DropControllerParameters params(
                transform,
                &entityManager,
                animator,
                rigidbody
            );

            return params;
        }

        static DropController createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return DropController(fromJSON(j, entityManager));
        }
};