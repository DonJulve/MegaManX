#pragma once

#include "Components.h"
#include "Debug.h"
#include "AnimatorComponent.h"

struct PipeturnControllerParameters
{
    Transform *transform;
    AnimatorComponent *animator;
    EntityManager *entityManager;
    Rigidbody *rigidbody;

    PipeturnControllerParameters(Transform *t, EntityManager *em, AnimatorComponent *ac, Rigidbody *rb) : transform(t), entityManager(em), animator(ac), rigidbody(rb) {}
};

struct PipeturnController : public Component
{
    Transform *transform;
    EntityManager *entityManager;
    AnimatorComponent *animator;
    Rigidbody *rigidbody;
    ClassicIAComponent *iaComponent;
    Collider *collider;

    PipeturnController(PipeturnControllerParameters params) : transform(params.transform),
                                                              entityManager(params.entityManager),
                                                              animator(params.animator),
                                                              rigidbody(params.rigidbody) {}

    float darseLaVuelta = 2.0f;
    bool arriba = true;
    float posicionInicial = 0;

    void start() override
    {
        collider = entityManager->getComponent<Collider>(parent);
        rigidbody->velocityX = 100.0f;
        posicionInicial = transform->posY;
        animator->addFrameTrigger("rotate", 2, [this]()
                                  {
            animator->spriteRenderer->alpha = 0;  
            transform->posY = posicionInicial; });
        animator->addFrameTrigger("rotate", 3, [this]()
                                  {
            animator->spriteRenderer->alpha = 255;
            transform->posY += 50;
            animator->spriteRenderer->flip = SDL_FLIP_VERTICAL; });

        animator->addFrameTrigger("rotate", 6, [this]()
                                  {
            animator->spriteRenderer->flip = SDL_FLIP_NONE;
            transform->posY -= 80; });
    }

    void update(float deltaTime) override
    {
        darseLaVuelta -= deltaTime;
        if (darseLaVuelta <= 0.0f)
        {
            rigidbody->velocityX *= -1;
            darseLaVuelta = 2.0f;
        }
    }
};

class PipeturnControllerLoader
{
public:
    static PipeturnControllerParameters fromJSON(const nlohmann::json &j, EntityManager &entityManager)
    {
        Transform *transform = entityManager.getComponent<Transform>(entityManager.getLast());
        if (!transform)
        {
            throw std::runtime_error("PipeturnController requires a Transform component");
        }

        AnimatorComponent *animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
        if (!animator)
        {
            throw std::runtime_error("PipeturnController requires a AnimatorComponent component");
        }

        Rigidbody *rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
        if (!rigidbody)
        {
            throw std::runtime_error("PipeturnController requires a Rigidbody component");
        }

        return PipeturnControllerParameters(transform, &entityManager, animator, rigidbody);
    }

    static PipeturnController createFromJSON(const nlohmann::json &j, EntityManager &entityManager)
    {
        PipeturnControllerParameters params = fromJSON(j, entityManager);
        return PipeturnController(params);
    }
};