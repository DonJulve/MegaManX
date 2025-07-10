#ifndef BIG_EXPLOSION_COMPONENT_H
#define BIG_EXPLOSION_COMPONENT_H

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "AudioManager.h"

struct BigExplosionParameters{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;

    BigExplosionParameters(Transform* t, EntityManager* em, AnimatorComponent* ac):
    transform(t), entityManager(em), animator(ac) {}
};

struct BigExplosionComponent : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    int positionX;
    int positionY;
    
    AudioManager* audioManager;

    Collider* collider;

    //Contructor del componente
    BigExplosionComponent(BigExplosionParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator) {}

    void start() override {
        
        audioManager = AudioManager::getInstance();
        audioManager->loadSound("smallExplosion", "sound_effects/58 - MMX - Enemy Die (3).wav");
        collider = entityManager->getComponent<Collider>(parent);
        animator->addFrameTrigger("bigExplosion", 10, [this](){
            //Borrar la explosion
            entityManager->deleteEntity(parent);
        });

        animator->addFrameTrigger("bigExplosion", 5, [this](){
            //Reproducir sonido de explosion
            collider->scaleX = 0;
            collider->scaleY = 0;
        });

        animator->addFrameTrigger("bigExplosion", 1, [this](){
            //Reproducir el sonido de la explosion
            audioManager->playSound("smallExplosion");
        }); 
    }

    void update(float deltaTime) override {

    }
};

class BigExplosionLoader {
    public:
    
        static BigExplosionParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("BigExplosionComponent requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("BigExplosionComponent requires a AnimatorComponent component");
            }

            BigExplosionParameters params(
                transform,
                &entityManager,
                animator
            );

            return params;
        }

        static BigExplosionComponent createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return BigExplosionComponent(fromJSON(j, entityManager));
        }
};


#endif