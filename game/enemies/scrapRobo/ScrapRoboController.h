#pragma once

#include "Components.h"
#include "InputManager.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "effects/SmallExplosionComponent.h"
#include "drops/DropController.h"
#include "./ScrapRoboBullet.h"
#include "IAComponents.h"

struct ScrapRoboControllerParameters
{

    Transform *transform;
    EntityManager *entityManager;
    AnimatorComponent *animator;
    Rigidbody *rigidbody;
    HealthComponent *healthComponent;

    ScrapRoboControllerParameters(Transform *t, EntityManager *em, AnimatorComponent *ac, Rigidbody *rb, HealthComponent *hc) : transform(t), entityManager(em), animator(ac), rigidbody(rb), healthComponent(hc) {}
};

struct ScrapRoboController : public Component
{
    Transform *transform;
    EntityManager *entityManager;
    AnimatorComponent *animator;
    Rigidbody *rigidbody;
    HealthComponent *healthComponent;

    ClassicIAComponent *iaComponent;
    std::shared_ptr<MegamanController> player;
    Collider *collider;

    bool readyToAttack = false;
    float agroDistance = 500.0f;
    float attackTimer = 0.0f;
    float attackCharge = 1.5f;
    bool isCharging = false;
    float attackCooldown = 4.0f;

    ScrapRoboController(ScrapRoboControllerParameters params) : transform(params.transform),
                                                                entityManager(params.entityManager),
                                                                animator(params.animator),
                                                                rigidbody(params.rigidbody),
                                                                healthComponent(params.healthComponent) {}

    void setup_ia()
    {
        iaComponent->setInitialState("Idle");

        iaComponent->registerAction("Idle", [this](EntityManager *entityManager, float deltaTime)
                                    {
                                        // Debug::Log("Action Idle");
                                    });

        iaComponent->registerAction("Attack", [this](EntityManager *entityManager, float deltaTime)
                                    {
                                        // Debug::Log("Action Attack");
                                    });

        iaComponent->registerAction("Death", [this](EntityManager *entityManager, float deltaTime)
                                    {
            // Debug::Log("Action Death");
            death(); });

        iaComponent->registerCondition("Idle", "Attack", [this](EntityManager *entityManager, float deltaTime)
                                       {
                                           // Debug::Log("Idle -> Attack");

                                           Vector2D normal = {0.0, 0.0};
                                           if (player && readyToAttack)
                                           {
                                               normal.x = player->getTransform()->posX - transform->posX;
                                               normal.y = player->getTransform()->posY - transform->posY;
                                               /*if(player->getTransform()->posY > transform->posY - 25 && player->getTransform()->posY < transform->posY + 25){

                                               }*/

                                               if (normal.x < 0 && normal.x > -agroDistance)
                                               {
                                                   animator->spriteRenderer->flip = SDL_FLIP_NONE;
                                                   isCharging = true;
                                                   readyToAttack = false;
                                                   animator->setBool("isAttacking", true);
                                                   return true;
                                               }
                                               else if (normal.x > 0 && normal.x < agroDistance)
                                               {
                                                   animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
                                                   isCharging = true;
                                                   readyToAttack = false;
                                                   animator->setBool("isAttacking", true);
                                                   return true;
                                               }
                                           }
                                           return false; });

        iaComponent->registerCondition("Attack", "Idle", [this](EntityManager *entityManager, float deltaTime)
                                       {
            // Debug::Log("Attack -> Idle");
            return (!isCharging); });

        iaComponent->registerCondition("Idle", "Death", [this](EntityManager *entityManager, float deltaTime)
                                       {
            // Debug::Log("Idle -> Death");
            return (healthComponent->isDead()); });

        iaComponent->registerCondition("Attack", "Death", [this](EntityManager *entityManager, float deltaTime)
                                       {
            // Debug::Log("Attack -> Death");
            return (healthComponent->isDead()); });
    }

    void findPlayer()
    {
        if (!player)
            entityManager->findComponent<MegamanController>(player);
    }

    void death()
    {
        // Debug::Log("MUERTE");
        if (!PrefabManager::getInstance().hasPrefab("smallExplosion"))
        {
            PrefabManager::getInstance().loadPrefab("smallExplosion");
        }
        Entity explosionEntity = PrefabManager::getInstance().instantiatePrefab("smallExplosion", *entityManager);
        Transform *explosionTransform = entityManager->getComponent<Transform>(explosionEntity);
        SmallExplosionComponent *explosionComponent = entityManager->getComponent<SmallExplosionComponent>(explosionEntity);
        if (explosionTransform)
        {
            explosionTransform->posX = transform->posX;
            explosionTransform->posY = transform->posY;
            explosionComponent->start();
        }

        // Scamos drop
        if (!PrefabManager::getInstance().hasPrefab("drop"))
        {
            PrefabManager::getInstance().loadPrefab("drop");
        }
        Entity dropEntity = PrefabManager::getInstance().instantiatePrefab("drop", *entityManager);
        Transform *dropTransform = entityManager->getComponent<Transform>(dropEntity);
        DropController *dropController = entityManager->getComponent<DropController>(dropEntity);
        if (dropTransform)
        {
            dropTransform->posX = transform->posX;
            dropTransform->posY = transform->posY;
        }
        dropController->start();

        entityManager->deleteEntity(parent);
    }

    void shootBullet()
    {
        if (!PrefabManager::getInstance().hasPrefab("scrapRoboBullet"))
        {
            PrefabManager::getInstance().loadPrefab("scrapRoboBullet");
        }
        Entity bulletEntity = PrefabManager::getInstance().instantiatePrefab("scrapRoboBullet", *entityManager);
        Transform *bulletTransform = entityManager->getComponent<Transform>(bulletEntity);
        ScrapRoboBullet *bulletComponent = entityManager->getComponent<ScrapRoboBullet>(bulletEntity);
        if (bulletTransform)
        {
            bulletTransform->posY = transform->posY - 14;
            bulletComponent->start();
            if (animator->spriteRenderer->flip == SDL_FLIP_NONE)
            {
                bulletTransform->posX = transform->posX - 20;
                bulletComponent->setVelocity(false);
            }
            else
            {
                bulletTransform->posX = transform->posX + 20;
                bulletComponent->setVelocity(true);
            }
        }
    }

    std::function<void(Collider *, CollisionResult)> callbackCollisionEnter = [this](Collider *other, CollisionResult cd)
    {
        if (other->tag == "megamanBullet")
        {
            healthComponent->takeDamage(35);
        }
        if (other->tag == "megamanMediumBullet")
        {
            healthComponent->takeDamage(50);
        }
        if (other->tag == "megamanStrongBullet")
        {
            healthComponent->takeDamage(100);
        }
        if (other->tag == "Terrain" && !animator->getBool("isLanded"))
        {
            animator->setBool("isLanded", true);
            animator->setBool("isCrawling", true);
            collider->scaleY = 37.0f;
        }
    };

    void start() override
    {

        iaComponent = entityManager->getComponent<ClassicIAComponent>(parent);
        if (!iaComponent)
        {
            throw std::runtime_error("Axemax requires a ClassicIAComponent component");
        }

        setup_ia();

        collider = entityManager->getComponent<Collider>(parent);

        collider->onCollisionEnter = callbackCollisionEnter;

        animator->addFrameTrigger("crawl", 2, [this]()
                                  { animator->setBool("isCrawling", false); });

        animator->addFrameTrigger("crawl", 1, [this]()
                                  {
            if(animator->spriteRenderer->flip == SDL_FLIP_NONE){
                transform->posX -= 20.0f;
            }
            else{
                transform->posX += 20.0f;
            } });
    }

    void update(float deltaTime) override
    {
        findPlayer();

        if (!readyToAttack)
        {
            attackCooldown -= deltaTime;
            if (attackCooldown <= 0.0f)
            {
                readyToAttack = true;
                attackCooldown = 6.0f;
            }
        }

        if (isCharging)
        {
            attackTimer += deltaTime;
            if (attackTimer >= attackCharge)
            {
                attackTimer = 0.0f;
                isCharging = false;
                shootBullet();
                animator->setBool("isAttacking", false);
            }
        }
    }
};

class ScrapRoboControllerLoader
{
public:
    static ScrapRoboControllerParameters fromJSON(const nlohmann::json &j, EntityManager &entityManager)
    {
        Transform *transform = entityManager.getComponent<Transform>(entityManager.getLast());
        if (!transform)
        {
            throw std::runtime_error("ScrapRoboController requires a Transform component");
        }

        AnimatorComponent *animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
        if (!animator)
        {
            throw std::runtime_error("ScrapRoboController requires a AnimatorComponent component");
        }

        Rigidbody *rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
        if (!rigidbody)
        {
            throw std::runtime_error("ScrapRoboController requires a Rigidbody component");
        }

        HealthComponent *healthComponent = entityManager.getComponent<HealthComponent>(entityManager.getLast());
        if (!healthComponent)
        {
            throw std::runtime_error("ScrapRoboController requires a HealthComponent component");
        }

        ScrapRoboControllerParameters params(
            transform,
            &entityManager,
            animator,
            rigidbody,
            healthComponent);

        return params;
    }

    static ScrapRoboController createFromJSON(nlohmann::json &j, EntityManager &entityManager)
    {
        return ScrapRoboController(fromJSON(j, entityManager));
    }
};