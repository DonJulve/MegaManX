#pragma once

#include "Components.h"
#include "AnimatorComponent.h"

struct MegamanChargeParameters
{
    Transform *transform;
    EntityManager *entityManager;
    AnimatorComponent *animator;
    Rigidbody *rigidBody;

    MegamanChargeParameters(Transform *t, EntityManager *em, AnimatorComponent *ac, Rigidbody *rb) : transform(t), entityManager(em), animator(ac), rigidBody(rb) {}
};

struct MegamanChargeComponent : public Component
{
    Transform *transform;
    EntityManager *entityManager;
    AnimatorComponent *animator;
    Rigidbody *rigidBody;

    MegamanChargeComponent(MegamanChargeParameters params) : transform(params.transform),
                                                             entityManager(params.entityManager),
                                                             animator(params.animator),
                                                             rigidBody(params.rigidBody) {}

    void cambiarCarga()
    {
        animator->setBool("secondCharge", true);
    }

    void aparecer()
    {
        // Debug::Log("APARECIENDO CARGA");
        animator->setBool("secondCharge", false);
        // Debug::Log("APARECIENDO CARGA 1");
        animator->spriteRenderer->alpha = 255;
        // Debug::Log("APARECIENDO CARGA 2");
    }

    void desaparecer()
    {
        animator->spriteRenderer->alpha = 0;
        // Debug::Log("DESAPARECIENDO CARGA 1");
        animator->setBool("secondCharge", false);
    }

    void mover(float x, float y)
    {
        transform->posX = x;
        transform->posY = y;
    }

    void start() override
    {
        // Debug::Log("START CARGA");
    }

    void update(float deltaTime) override
    {
        // Implementar lógica de carga aquí
    }
};

class MegamanChargeLoader
{
public:
    static MegamanChargeParameters fromJSON(const nlohmann::json &j, EntityManager &entityManager)
    {
        Transform *transform = entityManager.getComponent<Transform>(entityManager.getLast());
        if (!transform)
        {
            throw std::runtime_error("MegamanChargeComponent requires a Transform component");
        }

        AnimatorComponent *animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
        if (!animator)
        {
            throw std::runtime_error("MegamanChargeComponent requires a AnimatorComponent component");
        }

        Rigidbody *rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
        if (!rigidbody)
        {
            throw std::runtime_error("MegamanChargeComponent requires a Rigidbody component");
        }

        MegamanChargeParameters params(
            transform,
            &entityManager,
            animator,
            rigidbody);

        return params;
    }

    static MegamanChargeComponent createFromJSON(nlohmann::json &j, EntityManager &entityManager)
    {
        return MegamanChargeComponent(fromJSON(j, entityManager));
    }
};