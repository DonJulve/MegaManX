#pragma once

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"


struct AcidDropGeneratorParameters {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    
    AcidDropGeneratorParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb):
    transform(t), entityManager(em), animator(ac), rigidbody(rb) {}
};


struct AcidDropGenerator : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;

    AcidDropGenerator(AcidDropGeneratorParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidbody(params.rigidbody) {}

    Collider* collider;
    float tiempoRecarga = 2.0f;
    float tiempoTranscurrido = 0.0f;

    void instanciarAcido(){
        if (!PrefabManager::getInstance().hasPrefab("acidDrop")) {
            PrefabManager::getInstance().loadPrefab("acidDrop");
        }
        Entity acidEntity = PrefabManager::getInstance().instantiatePrefab("acidDrop", *entityManager);
        Transform* acidTransform = entityManager->getComponent<Transform>(acidEntity);
        if(acidTransform){
            acidTransform->posY = transform->posY + 15;
        }
    }

    void start() override {

        animator->addFrameTrigger("attack", 3, [this](){
            instanciarAcido();
            animator->setBool("isAttacking", false);
        });

    }

    void update(float deltaTime) override {
        tiempoTranscurrido += deltaTime;
        if(tiempoTranscurrido >= tiempoRecarga){
            animator->setBool("isAttacking", true);
            tiempoTranscurrido = 0.0f;
        }
    }
};

class AcidDropGeneratorLoader {
public:
    static AcidDropGeneratorParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
        Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
        if (!transform) {
            throw std::runtime_error("AcidDropGenerator requires a Transform component");
        }

        AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
        if (!animator) {
            throw std::runtime_error("AcidDropGenerator requires a AnimatorComponent component");
        }

        Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
        if (!rigidbody) {
            throw std::runtime_error("AcidDropGenerator requires a Rigidbody component");
        }

        AcidDropGeneratorParameters params(
            transform,
            &entityManager,
            animator,
            rigidbody
        );
        
        return params;
    }

    static AcidDropGenerator createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
        return AcidDropGenerator(fromJSON(j, entityManager));
    }
};