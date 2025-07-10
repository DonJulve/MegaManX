#pragma once

#include "Components.h"

struct ScrapRoboBulletParameters
{
    Transform *transform;
    EntityManager *entityManager;
    Rigidbody *rigidbody;

    ScrapRoboBulletParameters(Transform *t, EntityManager *em, Rigidbody *rb) : transform(t), entityManager(em), rigidbody(rb) {}
};

struct ScrapRoboBullet : public Component
{
    Transform *transform;
    EntityManager *entityManager;
    Rigidbody *rigidbody;

    Collider *collider;

    const float speed = 250.0f;

    ScrapRoboBullet(ScrapRoboBulletParameters params) : transform(params.transform),
                                                        entityManager(params.entityManager),
                                                        rigidbody(params.rigidbody) {}

    void setVelocity(bool direction)
    {
        if (direction)
        {
            rigidbody->velocityX = speed;
        }
        else
        {
            rigidbody->velocityX = -speed;
        }
    }

    void start() override
    {
        collider = entityManager->getComponent<Collider>(parent);
        if (!collider)
        {
            throw std::runtime_error("ScrapRoboBullet requires a Collider component");
        }
    }

    void update(float deltaTime) override
    {
        // Implement bullet behavior here
    }
};

class ScrapRoboBulletLoader
{
public:
    static ScrapRoboBulletParameters fromJSON(const nlohmann::json &j, EntityManager &entityManager)
    {
        Transform *transform = entityManager.getComponent<Transform>(entityManager.getLast());
        if (!transform)
        {
            throw std::runtime_error("ScrapRoboBullet requires a Transform component");
        }

        Rigidbody *rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
        if (!rigidbody)
        {
            throw std::runtime_error("ScrapRoboBullet requires a Rigidbody component");
        }

        ScrapRoboBulletParameters params(
            transform,
            &entityManager,
            rigidbody);

        return params;
    }

    static ScrapRoboBullet createFromJSON(const nlohmann::json &j, EntityManager &entityManager)
    {
        ScrapRoboBulletParameters params = fromJSON(j, entityManager);
        return ScrapRoboBullet(params);
    }
};
