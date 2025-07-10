#pragma once

#include "Components.h"
#include "AnimatorComponent.h"
#include "HealthComponent.h"
#include "EntityManager.h"
#include "InputManager.h"
#include "RaycastManager.h"
#include "Debug.h"
#include "effects/SmallExplosionComponent.h"
#include "AudioManager.h"
#include <random>
#include "../MegamanLevelsManager.h"

struct FlamemammothControllerParameters
{
    Transform *transform;
    EntityManager *entityManager;
    AnimatorComponent *animator;
    Rigidbody *rigidBody;
    HealthComponent *healthComponent;

    FlamemammothControllerParameters(Transform *t, EntityManager *em, AnimatorComponent *ac, Rigidbody *rb, HealthComponent *hc)
        : transform(t), entityManager(em), animator(ac), rigidBody(rb), healthComponent(hc) {}
};

struct FlamemammothController : public Component
{
    Transform *transform;
    EntityManager *entityManager;
    AnimatorComponent *animator;
    Rigidbody *rigidBody;
    HealthComponent *healthComponent;

    Collider *collider;
    ClassicIAComponent *iaComponent;
    std::shared_ptr<MegamanController> player;

    bool waitStart = true;
    float startTimer = 3.0f;

    float jumpForceY = -800.0f;
    float jumpForceX = 400.0f;
    bool isJumping = false;
    bool suelo = true;
    bool input = true;

    float lavaForceY = -200.0f;
    float lavaForceX = 400.0f;

    float fireForceY = 0.0f;
    float fireForceX = 200.0f;

    float distMelee = 200.0f;
    float distRange = 300.0f + distMelee;

    float saltoMelee = 0.5f;
    float lavaMelee = 0.2f;
    float fuegoMelee = 0.3f;

    float saltoRange = 0.3f;
    float lavaRange = 0.2f;
    float fuegoRange = 0.5f;

    float inputCooldown = 0.5f;
    float inputTimer = 0.0f;
    bool tieneAccion = false;

    bool saltar = false;
    bool dispararLava = false;
    bool dispararFuego = false;

    std::random_device rd;
    std::mt19937 generator;
    std::uniform_real_distribution<float> dis;

    FlamemammothController(FlamemammothControllerParameters params) : transform(params.transform),
                                                                      entityManager(params.entityManager),
                                                                      animator(params.animator),
                                                                      rigidBody(params.rigidBody),
                                                                      healthComponent(params.healthComponent)
    {
        generator = std::mt19937(rd());
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
    };

    void jump()
    {
        animator->setBool("isJumping", true);
        rigidBody->velocityX = jumpForceX;
        rigidBody->velocityY = jumpForceY;
    }

    void manageIsFalling()
    {
        if (isJumping)
        {
            if (rigidBody->velocityY > 0)
            {
                animator->setBool("isFalling", true);
                animator->setBool("isJumping", false);
            }
            rayCastFall();
        }
    }

    void findPlayer()
    {
        if (!player)
            entityManager->findComponent<MegamanController>(player);
    }

    void crearFuego()
    {
        if (!PrefabManager::getInstance().hasPrefab("flamemammoth_fire"))
        {
            PrefabManager::getInstance().loadPrefab("flamemammoth_fire");
        }
        Entity fireEntity = PrefabManager::getInstance().instantiatePrefab("flamemammoth_fire", *entityManager);
        Transform *fireTransform = entityManager->getComponent<Transform>(fireEntity);
        Rigidbody *fireRigidbody = entityManager->getComponent<Rigidbody>(fireEntity);
        FireComponent *fireComponent = entityManager->getComponent<FireComponent>(fireEntity);
        if (fireTransform && fireRigidbody)
        {
            fireTransform->posX = transform->posX;
            fireTransform->posY = transform->posY;
            if (animator->spriteRenderer->flip == SDL_FLIP_HORIZONTAL)
            {
                fireTransform->posX += 50;
                fireComponent->setVelocity(lavaForceX, lavaForceY);
            }
            else
            {
                fireTransform->posX -= 50;
                fireComponent->setVelocity(-lavaForceX, lavaForceY);
            }
        }
    }

    void crearLava()
    {
        if (!PrefabManager::getInstance().hasPrefab("flamemammoth_lava"))
        {
            PrefabManager::getInstance().loadPrefab("flamemammoth_lava");
        }
        Entity lavaEntity = PrefabManager::getInstance().instantiatePrefab("flamemammoth_lava", *entityManager);
        Transform *lavaTransform = entityManager->getComponent<Transform>(lavaEntity);
        Rigidbody *lavaRigidbody = entityManager->getComponent<Rigidbody>(lavaEntity);
        LavaComponent *lavaComponent = entityManager->getComponent<LavaComponent>(lavaEntity);
        if (lavaTransform && lavaRigidbody)
        {
            lavaTransform->posY = transform->posY;
            if (animator->spriteRenderer->flip == SDL_FLIP_HORIZONTAL)
            {
                lavaTransform->posX = transform->posX + 50;
                lavaComponent->setVelocity(lavaForceX, lavaForceY);
            }
            else
            {
                lavaTransform->posX = transform->posX - 50;
                lavaComponent->setVelocity(-lavaForceX, lavaForceY);
            }
        }
    }

    void rayCastFall()
    {
        SDL_FPoint origin = {static_cast<float>(transform->posX), static_cast<float>(transform->posY)};
        SDL_FPoint direction = SDL_FPoint{0.0f, 1.0f};

        Ray ray(origin, direction);

        // Realizar el raycast
        std::optional<RaycastHit> hit = RayCastManager::raycast(ray, entityManager, 90.0f, {"Terrain"});

        if (hit.has_value())
        {
            suelo = true;
            Debug::Log("Hay suelo");
            animator->setBool("isFalling", false);
        }
    }

    void setupia()
    {
        iaComponent->setInitialState("Idle");

        iaComponent->registerAction("Idle", [this](EntityManager *entityManager, float deltaTime)
                                    {
                                        Debug::Log("Idle");
                                        if (waitStart)
                                            return;
                                        if (inputTimer >= inputCooldown)
                                        {
                                            inputTimer = 0.0f;
                                            tieneAccion = true;
                                        }
                                        else
                                        {
                                            inputTimer += deltaTime;
                                            return;
                                        }

                                        Vector2D normal = {0.0, 0.0};
                                        if (player)
                                        {
                                            normal.x = player->getTransform()->posX - transform->posX;
                                            normal.y = player->getTransform()->posY - transform->posY;
                                            if (std::abs(normal.x) < distMelee)
                                            {
                                                // Estamos rango a melee
                                                float random = dis(generator);
                                                if (random < saltoMelee)
                                                {
                                                    // Hacer el salto
                                                    saltar = true;
                                                }
                                                else if (random < lavaMelee + saltoMelee)
                                                {
                                                    // Disparar lava
                                                    dispararLava = true;
                                                }
                                                else if (random < fuegoMelee + lavaMelee + saltoMelee)
                                                {
                                                    // Disparar fuego
                                                    dispararFuego = true;
                                                }
                                            }
                                            else if (std::abs(normal.x) < distRange)
                                            {
                                                // Estamos rango a distancia
                                                float random = dis(generator);
                                                if (random < saltoRange)
                                                {
                                                    // Hacer el salto
                                                    saltar = true;
                                                }
                                                else if (random < lavaRange + saltoRange)
                                                {
                                                    // Disparar lava
                                                    dispararLava = true;
                                                }
                                                else if (random < fuegoRange + lavaRange + saltoRange)
                                                {
                                                    // Disparar fuego
                                                    dispararFuego = true;
                                                }
                                            }
                                        } });

        iaComponent->registerAction("Jump", [this](EntityManager *entityManager, float deltaTime)
                                    {
            Debug::Log("Jump");
            manageIsFalling(); });

        iaComponent->registerAction("ShootLava", [this](EntityManager *entityManager, float deltaTime)
                                    { Debug::Log("ShootLava"); });

        iaComponent->registerAction("ShootFire", [this](EntityManager *entityManager, float deltaTime)
                                    { Debug::Log("ShootFire"); });

        iaComponent->registerAction("Death", [this](EntityManager *entityManager, float deltaTime)
                                    {
            Debug::Log("Death");
            death(); });

        iaComponent->registerCondition("Idle", "Jump", [this](EntityManager *entityManager, float deltaTime)
                                       {
            Debug::Log("Idle -> Jump");
            if(!tieneAccion) return false;
            if(saltar){
                //Calcular hacia donde saltar
                float distJugador = 0.0f;
                if(player){
                    distJugador = player->getTransform()->posX - transform->posX;
                    if(distJugador >= 0){
                        animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
                        jumpForceX = distJugador;
                        jump();
                    }
                    else{
                        animator->spriteRenderer->flip = SDL_FLIP_NONE;
                        jumpForceX = distJugador;
                        jump();
                    }
                }
                return true;
            }
            return saltar; });

        iaComponent->registerCondition("Jump", "Idle", [this](EntityManager *entityManager, float deltaTime)
                                       {
            Debug::Log("Jump -> Idle");
            if(input){
                tieneAccion = false;
                saltar = false;
                inputTimer = 0.0f;
                inputCooldown = 0.5f + dis(generator);
            }
            return input; });

        iaComponent->registerCondition("Idle", "ShootLava", [this](EntityManager *entityManager, float deltaTime)
                                       {
            Debug::Log("Idle -> ShootLava");
            if(!tieneAccion) return false;
            if(dispararLava){
                float distJugador = 0.0f;
                if(player){
                    distJugador = player->getTransform()->posX - transform->posX;
                    if(distJugador >= 0){
                        animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;                   
                    }
                    else{
                        animator->spriteRenderer->flip = SDL_FLIP_NONE;
                    }
                    animator->setBool("isShootingLava", true);
                    input = false;
                }
                return true;
            }
            return dispararLava; });

        iaComponent->registerCondition("Idle", "ShootFire", [this](EntityManager *entityManager, float deltaTime)
                                       {
            Debug::Log("Idle -> ShootFire");
            if(!tieneAccion) return false;
            if(dispararFuego){
                float distJugador = 0.0f;
                if(player){
                    distJugador = player->getTransform()->posX - transform->posX;
                    if(distJugador >= 0){
                        animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
                        fireForceX = distJugador + 50;
                    }
                    else{
                        animator->spriteRenderer->flip = SDL_FLIP_NONE;
                        fireForceX = distJugador - 50;
                    }
                    animator->setBool("isShootingFire", true);
                    input = false;
                }
                return true;
            }
            return dispararFuego; });

        iaComponent->registerCondition("ShootFire", "Idle", [this](EntityManager *entityManager, float deltaTime)
                                       {
            Debug::Log("ShootFire -> Idle");
            if(input){
                tieneAccion = false;
                dispararFuego = false;
                inputTimer = 0.0f;
                inputCooldown = 0.5f + dis(generator);
            }
            return input; });

        iaComponent->registerCondition("ShootLava", "Idle", [this](EntityManager *entityManager, float deltaTime)
                                       {
            Debug::Log("ShootLava -> Idle");
            if(input){
                tieneAccion = false;
                dispararLava = false;
                inputTimer = 0.0f;
                inputCooldown = 0.5f + dis(generator);
            }
            return input; });

        iaComponent->registerCondition("Idle", "Death", [this](EntityManager *entityManager, float deltaTime)
                                       { return healthComponent->isDead(); });

        iaComponent->registerCondition("Jump", "Death", [this](EntityManager *entityManager, float deltaTime)
                                       { return healthComponent->isDead(); });

        iaComponent->registerCondition("ShootLava", "Death", [this](EntityManager *entityManager, float deltaTime)
                                       { return healthComponent->isDead(); });

        iaComponent->registerCondition("ShootFire", "Death", [this](EntityManager *entityManager, float deltaTime)
                                       { return healthComponent->isDead(); });
    }

    void death()
    {
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
        }
        MegamanLevelsManager::getInstance()->loadLevel("levelSelectorMenu", entityManager->getSceneManager()->getSceneNameByEntityManager(entityManager), 255.0f / 1.5f);
        // Borrar la explosion
        entityManager->deleteEntity(parent);
        AudioManager::getInstance()->stopMusic();
    }

    void start() override
    {
        collider = entityManager->getComponent<Collider>(parent);
        if (!collider)
        {
            throw std::runtime_error("FlamemammothController requires a Collider component");
        }
        collider->onCollisionEnter = callbackCollisionEnter;

        iaComponent = entityManager->getComponent<ClassicIAComponent>(parent);
        if (!iaComponent)
        {
            throw std::runtime_error("FlamemammothController requires a ClassicIAComponent component");
        }

        setupia();
        AudioManager::getInstance()->stopMusic();
        AudioManager::getInstance()->loadMusic("boss", "music/20. Boss Battle.mp3");
        AudioManager::getInstance()->playMusic("boss", -1, 0);

        animator->addFrameTrigger("jump", 0, [this]()
                                  {
            isJumping = true;
            input = false; });

        animator->addFrameTrigger("idle", 0, [this]()
                                  { input = true; });

        animator->addFrameTrigger("shootLava", 5, [this]()
                                  { crearLava(); });

        animator->addFrameTrigger("shootLava", 7, [this]()
                                  {
            input = true;
            animator->setBool("isShootingLava", false); });

        animator->addFrameTrigger("shootFire", 2, [this]()
                                  { crearFuego(); });

        animator->addFrameTrigger("shootFire", 4, [this]()
                                  {
            input = true;
            animator->setBool("isShootingFire", false); });

        animator->addFrameTrigger("start", 4, [this]()
                                  {
            waitStart = true;
            tieneAccion = false; });
    }

    void update(float deltaTime) override
    {
        findPlayer();
        if (waitStart)
        {
            if (startTimer > 0.0f)
            {
                startTimer -= deltaTime;
            }
            else
            {
                animator->setBool("isStarting", false);
                waitStart = false;
                tieneAccion = true;
            }
        }
    }

    // TODO; ES PARA DEBUG
    void render(EntityManager &entityManager, SDL_Renderer *renderer) override
    {
        Camera *camera = nullptr;
        for (Entity entity : entityManager.getEntities())
        {
            camera = entityManager.getComponent<Camera>(entity);
            if (camera)
                break; // Assuming there is only one camera in the scene
        }

        SDL_FPoint origin = {transform->posX, transform->posY};
        SDL_FPoint direction = {0.0f, 1.0f}; // Down direction
        Ray ray(origin, direction);

        // RayCastManager::renderRay(renderer, ray, *camera, 90.0f, {255, 255, 255, 255});
    };
};

class FlamemammothControllerLoader
{
public:
    static FlamemammothControllerParameters fromJSON(const nlohmann::json &j, EntityManager &entityManager)
    {
        Transform *transform = entityManager.getComponent<Transform>(entityManager.getLast());
        if (!transform)
        {
            throw std::runtime_error("FlamemammothController requires a Transform component");
        }

        AnimatorComponent *animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
        if (!animator)
        {
            throw std::runtime_error("FlamemammothController requires a AnimatorComponent component");
        }

        Rigidbody *rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
        if (!rigidbody)
        {
            throw std::runtime_error("FlamemammothController requires a Rigidbody component");
        }

        HealthComponent *healthComponent = entityManager.getComponent<HealthComponent>(entityManager.getLast());
        if (!healthComponent)
        {
            throw std::runtime_error("FlamemammothController requires a HealthComponent component");
        }

        FlamemammothControllerParameters params(
            transform,
            &entityManager,
            animator,
            rigidbody,
            healthComponent);

        return params;
    }

    static FlamemammothController createFromJSON(nlohmann::json &j, EntityManager &entityManager)
    {
        return FlamemammothController(fromJSON(j, entityManager));
    }
};