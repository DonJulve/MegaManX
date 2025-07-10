#ifndef SMALL_EXPLOSION_COMPONENT_H
#define SMALL_EXPLOSION_COMPONENT_H

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "AudioManager.h"

struct SmallExplosionParameters{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;

    SmallExplosionParameters(Transform* t, EntityManager* em, AnimatorComponent* ac):
    transform(t), entityManager(em), animator(ac) {}
};

struct SmallExplosionComponent : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    int positionX;
    int positionY;
    AudioManager* audioManager;

    //Contructor del componente
    SmallExplosionComponent(SmallExplosionParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator) {
        Debug::Log("CONSTRUCTOR DE SMALL EXPLOSION");
    }

    void start() override {
        audioManager = AudioManager::getInstance();
        audioManager->loadSound("smallExplosion", "sound_effects/58 - MMX - Enemy Die (3).wav");

        Debug::Log("SMALL EXPLOSION START");
        animator->addFrameTrigger("smallExplosion", 7, [this](){
            //Borrar la explosion
            entityManager->deleteEntity(parent);
        });

        animator->addFrameTrigger("smallExplosion", 1, [this](){
            //Reproducir el sonido de la explosion
            audioManager->playSound("smallExplosion");
        }); 
    }

    void update(float deltaTime) override {
    }


};

class SmallExplosionLoader {
    public:
    
        static SmallExplosionParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("SmallExplosionComponent requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("SmallExplosionComponent requires a AnimatorComponent component");
            }

            SmallExplosionParameters params(
                transform,
                &entityManager,
                animator
            );

            return params;
        }

        static SmallExplosionComponent createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return SmallExplosionComponent(fromJSON(j, entityManager));
        }
};

#endif // SMALL_EXPLOSION_COMPONENT_H