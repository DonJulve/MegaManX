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
#include "ReinforcementIAComponent.h"

struct FlamemammothAdvancedIAParameters
{
    Transform *transform;
    EntityManager *entityManager;
    AnimatorComponent *animator;
    Rigidbody *rigidBody;
    HealthComponent *healthComponent;

    FlamemammothAdvancedIAParameters(Transform *t, EntityManager *em, AnimatorComponent *ac, Rigidbody *rb, HealthComponent *hc)
        : transform(t), entityManager(em), animator(ac), rigidBody(rb), healthComponent(hc) {}
};

struct FlamemammothAdvancedIA : public Component
{
    Transform *transform;
    EntityManager *entityManager;
    AnimatorComponent *animator;
    Rigidbody *rigidBody;
    HealthComponent *healthComponent;

    Collider *collider;
    ReinforcementIAComponent *riaComponent;
    std::shared_ptr<MegamanController> player;

    bool waitStart = true;
    float startTimer = 3.0f;

    float jumpForceY = -800.0f;
    float jumpForceX = 400.0f;
    bool isJumping = false;
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

    
    
    float vidaActual = 0.0f;
    float vidaAnterior = 0.0f;
    
    float vidaMegamanActual = 0.0f;
    float vidaMegamanAnterior = 0.0f;
    
    bool tieneAccionIAavanzada = false;

    bool waitIaAvanzada = false;
    float iaAvanzadaTimer = 0.0f;
    float iaAvanzadaCooldown = 0.5f;

    bool suelo = true;

    float timerGuardarIA = 3.0f;

    std::random_device rd;
    std::mt19937 generator;
    std::uniform_real_distribution<float> dis;

    FlamemammothAdvancedIA(FlamemammothAdvancedIAParameters params) : transform(params.transform),
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
        suelo = false;
        waitIaAvanzada = false;
        tieneAccionIAavanzada = false;
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
        tieneAccionIAavanzada = false;
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
        tieneAccionIAavanzada = false;
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
            // Si hay colisión con el suelo, no está cayendo
            isJumping = false;
            waitIaAvanzada = true;
            Debug::Log("Hay suelo");
            suelo = true;
            animator->setBool("isFalling", false);
        }
        else
        {
            // Si no hay colisión, está cayendo
            isJumping = true;
            tieneAccionIAavanzada = false;
            Debug::Log("No hay suelo");
            suelo = false;
            animator->setBool("isFalling", true);
        }
    }

    //De meomento es para probar
    float repartirPesos(){
        if(vidaMegamanActual < vidaMegamanAnterior){
            return 16.0f;
        }
        if(vidaMegamanActual >= vidaMegamanAnterior){
            return -30.0f;
        }

        if(vidaActual < vidaAnterior){
            return -10.0f;
        }
        // if(vidaActual >= vidaAnterior){
        //     return 15.0f;
        // }
        return 0.0f;
    }

    void setupReinforcementIa(){
        riaComponent->registerAction("Idle", "Jump", [this](EntityManager* em, float dt){
            //Calcular hacia donde saltar
            Debug::Log(std::to_string(tieneAccionIAavanzada));
            if(!tieneAccionIAavanzada) return;
            if(!suelo) return;
            Debug::Log("tieneAccionIAavanzada");
            Debug::Log("Jump");
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
        });
        riaComponent->registerAction("Idle", "ShootLava", [this](EntityManager* em, float dt){
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
        });
        riaComponent->registerAction("Idle", "ShootFire", [this](EntityManager* em, float dt){
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
        });
        
        riaComponent->registerAction("Idle", "Stay", [this](EntityManager* em, float dt){
            //NO HACE NADA
        });

        riaComponent->registerRewardFunction([this](EntityManager* em, const std::string& state, const std::string& action, float deltaTime) -> float
        {
            if(state == "Idle" && action == "Jump"){
                return repartirPesos();
            }
            if(state == "Idle" && action == "ShootLava"){
                return repartirPesos();
            }
            if(state == "Idle" && action == "ShootFire"){
                return repartirPesos();
            }
            if(state == "Idle" && action == "Stay"){
                return repartirPesos();
            }
            return -1.0f;
        });
        
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

        riaComponent = entityManager->getComponent<ReinforcementIAComponent>(parent);
        if (!riaComponent)
        {
            throw std::runtime_error("FlamemammothController requires a ReinforcementIAComponent component");
        }

        findPlayer();

        vidaAnterior = healthComponent->getCurrentHealth();
        vidaMegamanAnterior = player->checkHealth();
        vidaActual = vidaAnterior;
        vidaMegamanActual = vidaMegamanAnterior;
        
        setupReinforcementIa();
        AudioManager::getInstance()->stopMusic();
        AudioManager::getInstance()->loadMusic("boss", "music/20. Boss Battle.mp3");
        AudioManager::getInstance()->playMusic("boss", -1, 0);

        animator->addFrameTrigger("jump", 0, [this]()
                                  {
            isJumping = true;
            tieneAccionIAavanzada = false;
            input = false; });

        animator->addFrameTrigger("idle", 0, [this]()
                                  { input = true; });

        animator->addFrameTrigger("shootLava", 5, [this]()
                                  { crearLava(); });

        animator->addFrameTrigger("shootLava", 7, [this]()
                                  {
            input = true;
            waitIaAvanzada = true;
            animator->setBool("isShootingLava", false); });

        animator->addFrameTrigger("shootFire", 2, [this]()
                                  { crearFuego(); });

        animator->addFrameTrigger("shootFire", 4, [this]()
                                  {
            input = true;
            waitIaAvanzada = true;
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
                tieneAccionIAavanzada = true;
                tieneAccion = true;
                waitIaAvanzada = false;
            }
        }
        //##########################
        //Ia mejorada
        if(healthComponent->isDead()){
            death();
        }
        if(isJumping){
            manageIsFalling();
        }

        vidaAnterior = vidaActual;
        vidaActual = healthComponent->getCurrentHealth();
        vidaMegamanAnterior = vidaMegamanActual;
        if (player)
        {
            vidaMegamanActual = player->checkHealth();
        }
        
        if(waitIaAvanzada){
            iaAvanzadaTimer += deltaTime;
            if(iaAvanzadaTimer >= iaAvanzadaCooldown){
                iaAvanzadaTimer = 0.0f;
                tieneAccionIAavanzada = true;
                waitIaAvanzada = false;
                iaAvanzadaTimer = 0.0f;
            }
        }

        timerGuardarIA -= deltaTime;
        if (timerGuardarIA <= 0.0f)
        {
            riaComponent->saveQTable("flamemammoth_qtable.json");
            timerGuardarIA = 3.0f;
        }
    };

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

class FlamemammothAdvancedIALoader
{
public:
    static FlamemammothAdvancedIAParameters fromJSON(const nlohmann::json &j, EntityManager &entityManager)
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

        FlamemammothAdvancedIAParameters params(
            transform,
            &entityManager,
            animator,
            rigidbody,
            healthComponent);

        return params;
    }

    static FlamemammothAdvancedIA createFromJSON(nlohmann::json &j, EntityManager &entityManager)
    {
        return FlamemammothAdvancedIA(fromJSON(j, entityManager));
    }
};