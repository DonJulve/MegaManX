#ifndef CHILLPENGUINADVANCEDIA_H
#define CHILLPENGUINADVANCEDIA_H

#include <SDL2/SDL.h>
#include "Components.h"
#include "InputManager.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "AudioManager.h"
#include "RaycastManager.h"
#include "HealthComponent.h"
#include "IAComponents.h"
#include "effects/SmallExplosionComponent.h"
#include "ChillPenguinBlizzardComponent.h"
#include "ChillPenguinBulletComponent.h"
#include "ChillPenguinIceBulletComponent.h"
#include "ChillPenguinPenguinComponent.h"
#include "../MegamanLevelsManager.h"
#include "ReinforcementIAComponent.h"

#include <algorithm>
#include <string>

class ChillPenguinAdvancedIAParameters {
public:

    EntityManager* entityManager;
    

    ChillPenguinAdvancedIAParameters(EntityManager* em): entityManager(em) {}
};

class ChillPenguinAdvancedIA : public Component {
private:
// -------------------------------------------
//  ChillPenguin Properties
// -------------------------------------------
    EntityManager* entityManager;    

    // Componentes
    Transform* transform;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    HealthComponent* health;
    AudioManager* audioManager;
    ReinforcementIAComponent* riaComponent;
    
// -------------------------------------------
//  Global variables
// -------------------------------------------
    std::shared_ptr<MegamanController> player;
    bool blizzardAttack = false;

    float dashForce = 200.0f; 
    float dashDuration = 1.0f;  
    float dashTimeLeft = 0.0f;

    float iceDuration = 2.0f;  
    float iceTimeLeft = 0.0f; 

    float blizzardDuration = 2.0f;  
    float blizzardTimeLeft = 0.0f;  

    float jumpForceY = 900.0f;
    float jumpForceX = 300.0f;

    float snowSpawnPositionX = 4285.0f;
    float snowSpawnPositionY = -1301.0f;

    int iceBreathLength = 7;

    Entity blizzardEntity;        
    std::vector<ChillPenguinPenguinComponent*> penguins; // Lista de pingüinos

// -------------------------------------------
//  IA variables
// -------------------------------------------

    bool inputActive = true;
    bool direccion = true;  //true derecha, false izquierda
    bool hasShoot = false;
    bool hasIceShoot = false;
    float agroDistanceX = 400.0f;
    float agroDistanceY = 50.0f;
    float iceAgroDistanceX = 400.0f;

    float shootCooldown = 2.0f;
    float shootTimer = 0.0f;
    
    float shootIceCooldown = 5.0f;
    float shootIceTimer = 0.0f;

    float farDistance = 600.0f;  // Distancia para considerar que Megaman está lejos
    float mediumDistance = 500.0f;
  
    bool isJumpBlizzardCooldown = false;
    float jumpBlizzardCooldown = 3.0f;
    float jumpBlizzardTimer = 0.0f;

    float dashCooldown = 5.0f;
    float dashCooldownTimer = 0.0f;
    bool isDashCooldown = false;

    float vidaActual = 0.0f;
    float vidaAnterior = 0.0f;

    float vidaMegamanActual = 0.0f;
    float vidaMegamanAnterior = 0.0f;

    bool hasJumped = false;

    float timerGuardarAprendizaje = 0.0f;
    float tiempoGuardarAprendizaje = 3.0f; // Guardar cada 5 segundos

    bool esperarIA = false;
    float esperarTimer = 0.0f;
    float esperarCooldown = 0.5f;

    bool suelo = true;

    
// -------------------------------------------
//  Managing KeyMaps
// -------------------------------------------

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
            // if(isJumpBlizzardCooldown) return;
            if(!inputActive) return;
            if(!suelo) return;
            calcularDireccion();
            manageJump(dt);
        });
        
        riaComponent->registerAction("Idle", "Dash", [this](EntityManager* em, float dt){
            // if(isDashCooldown) return;
            if(!inputActive) return;
            if(!suelo) return;
            calcularDireccion();
            manageDash(dt);
        });

        riaComponent->registerAction("Idle", "Shoot", [this](EntityManager* em, float dt){
            // if(hasShoot) return;
            if(!inputActive) return;
            if(!suelo) return;
            calcularDireccion();
            shoot(dt);
        });

        riaComponent->registerAction("Idle", "Ice", [this](EntityManager* em, float dt){
            // if(hasIceShoot) return;
            if(!inputActive) return;
            if(!suelo) return;
            calcularDireccion();
            iceBreath(dt);
        });

        riaComponent->registerAction("Idle", "Blizzard", [this](EntityManager* em, float dt){
            // if(isJumpBlizzardCooldown) return;
            if(!inputActive) return;
            if(!suelo) return;
            calcularDireccion();
            manageBlizzard(dt);
        });

        riaComponent->registerAction("Idle", "Stay", [this](EntityManager* em, float dt){
            // Do nothing, just stay idle
            // inputActive = true;
        });

        riaComponent->registerRewardFunction([this](EntityManager* em, const std::string& state, const std::string& action, float deltaTime) -> float
        {
            if(state == "Idle" && action == "Jump"){
                return repartirPesos();
            }
            if(state == "Idle" && action == "Dash"){
                return repartirPesos();
            }
            if(state == "Idle" && action == "Shoot"){
                return repartirPesos();
            }
            if(state == "Idle" && action == "Ice"){
                return repartirPesos();
            }
            if(state == "Idle" && action == "Blizzard"){
                return repartirPesos();
            }
            if(state == "Idle" && action == "Stay"){
                return repartirPesos();
            }
            return 0.0f;
        });
    }

    void manageJump(float deltaTime) 
    {
        hasJumped = true;
        inputActive = false;
        suelo = false;
        if (!animator->getBool("isActive")) return;
        if (!animator->getBool("isGrounded")) return;
        if (animator->getBool("isShooting")) return;
        if (animator->getBool("isDashing")) return;
        if (animator->getBool("isIce")) return;

        rigidBody->velocityY -= jumpForceY;
        
        animator->setBool("isGrounded", false);   
    }
    
    void manageDash(float deltaTime)
    {
        inputActive = false;
      if (dashTimeLeft > 0.0f) return;
      if (!animator->getBool("isActive")) return;
      if (!animator->getBool("isGrounded")) return;
      if (animator->getBool("isShooting")) return;
      if (animator->getBool("isDashing")) return;
      if (animator->getBool("isIce")) return;
      if (animator->getBool("isBlizzard")) return;

      if (animator->spriteRenderer->flip == SDL_FLIP_NONE) {
        rigidBody->velocityX = -dashForce;
      } else {
        rigidBody->velocityX = dashForce;
      }
    
      animator->setBool("isDashing", true);
      audioManager->playSound("dash");

      dashTimeLeft = dashDuration;
      isDashCooldown = true;
    }

    void manageShoot(float deltaTime)
    {
        if (!animator->getBool("isActive")) return;
        if (animator->getBool("isDashing")) return;
        if (!animator->getBool("isGrounded")) return;
        if (animator->getBool("isShooting")) return;
        if (animator->getBool("isIce")) return;
        if (animator->getBool("isBlizzard")) return;


        animator->setBool("isShooting", true);
        
        animator->addFrameTrigger("idleShooting", 5, [this](){
            esperarIA = true;
          animator->setBool("isShooting", false);
        });
    }

    void manageIcePenguin(float deltaTime)
    {
        if (dashTimeLeft > 0.0f) return;
        if (!animator->getBool("isActive")) return;
        if (!animator->getBool("isGrounded")) return;
        if (animator->getBool("isShooting")) return;
        if (animator->getBool("isDashing")) return;
        if (animator->getBool("isIce")) return;
        if (animator->getBool("isBlizzard")) return;

        animator->setBool("isIce", true);

        iceTimeLeft = iceDuration;
    }

    void manageBlizzard(float deltaTime)
    {
        hasJumped = true;
        suelo = false;
        inputActive = false;
        if (blizzardTimeLeft > 0.0f) return;
        if (!animator->getBool("isActive")) return;
        if (!animator->getBool("isGrounded")) return;
        if (animator->getBool("isShooting")) return;
        if (animator->getBool("isDashing")) return;
        if (animator->getBool("isIce")) return;
        if (animator->getBool("isBlizzard")) return;

        animator->setBool("isGrounded", false);
        
        rigidBody->velocityY -= jumpForceY;

        blizzardTimeLeft = blizzardDuration;
        blizzardAttack = true;
    }

// -------------------------------------------
//  Utilities
// -------------------------------------------
    void loadSoundEffects() {
        audioManager = AudioManager::getInstance();
        audioManager->loadMusic("boss", "music/20. Boss Battle.mp3");
        audioManager->loadSound("spawn", "sound_effects/chill penguin/Boss Intro.wav");
        audioManager->loadSound("dash", "sound_effects/chill penguin/Dash.wav");
        audioManager->loadSound("Ice", "sound_effects/chill penguin/Chill Penguin Breath.wav");
        audioManager->loadSound("Blizzard", "sound_effects/chill penguin/Blizzard.wav");
        audioManager->loadSound("Ice_break", "sound_effects/chill penguin/Ice Break.wav");
    }

// -------------------------------------------
//  IA
// -------------------------------------------

    void findPlayer(){
        if(!player) entityManager->findComponent<MegamanController>(player);
    }


public: 
    ChillPenguinAdvancedIA(ChillPenguinAdvancedIAParameters params) : transform(nullptr), 
    entityManager(params.entityManager), animator(nullptr), rigidBody(nullptr), health(nullptr) {}

    void start() override 
    { 

        transform = entityManager->getComponent<Transform>(parent);
        if (!transform) {
            throw std::runtime_error("ChillPenguinController requires a Transform component");
        }

        animator = entityManager->getComponent<AnimatorComponent>(parent);
        if (!animator) {
            throw std::runtime_error("ChillPenguinController requires a AnimatorComponent component");
        }

        rigidBody = entityManager->getComponent<Rigidbody>(parent);
        if (!rigidBody) {
            throw std::runtime_error("ChillPenguinController requires a Rigidbody component");
        }

        health = entityManager->getComponent<HealthComponent>(parent);
        if (!health) {
            throw std::runtime_error("ChillPenguinController requires a HealthComponent component");
        }

        blizzardEntity = -1;
        Collider* collider = entityManager->getComponent<Collider>(parent);

        riaComponent = entityManager->getComponent<ReinforcementIAComponent>(parent);
        if (!riaComponent) {
            throw std::runtime_error("RaybitController requires a Reinforcement IA component");
        }

        animator->addFrameTrigger("idle", 0, [this](){
           if(direccion){
              animator->spriteRenderer->flip = SDL_FLIP_NONE;
            } else {
              animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
            } 
        });

        animator->addFrameTrigger("idleShooting", 0, [this](){
           if(direccion){
              animator->spriteRenderer->flip = SDL_FLIP_NONE;
            } else {
              animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
            } 
        });
        animator->addFrameTrigger("ice", 0, [this](){
           if(direccion){
               animator->spriteRenderer->flip = SDL_FLIP_NONE;
            } else {
              animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
            } 
        });
        animator->addFrameTrigger("jump", 0, [this](){
           if(direccion){
               animator->spriteRenderer->flip = SDL_FLIP_NONE;
            } else {
              animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
            } 
        });
        animator->addFrameTrigger("blizzard", 0, [this](){
           if(direccion){
               animator->spriteRenderer->flip = SDL_FLIP_NONE;
            } else {
              animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
            } 
        });
        animator->addFrameTrigger("dash", 0, [this](){
           if(direccion){
               animator->spriteRenderer->flip = SDL_FLIP_NONE;
            } else {
              animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
            } 
        });
        
        animator->addFrameTrigger("blizzard", 2, [this](){
            if (!PrefabManager::getInstance().hasPrefab("chillpenguinBlizzard")) {
                PrefabManager::getInstance().loadPrefab("chillpenguinBlizzard");
            }
            blizzardEntity = PrefabManager::getInstance().instantiatePrefab("chillpenguinBlizzard", *entityManager);
            Transform* blizzardTransform = entityManager->getComponent<Transform>(blizzardEntity);
            ChillPenguinBlizzardComponent* blizzardComponent = entityManager->getComponent<ChillPenguinBlizzardComponent>(blizzardEntity);
            if(blizzardTransform){
                blizzardTransform->posX = snowSpawnPositionX;
                blizzardTransform->posY = snowSpawnPositionY;
                blizzardComponent->start();
                for (ChillPenguinPenguinComponent* penguin : penguins) {
                    if (penguin) {
                        penguin->blizzard = true;
                    }
                }

                audioManager->playSound("Blizzard");
                animator->addFrameTrigger("fall", 0, [this](){
                  if (blizzardEntity != -1){
                    entityManager->deleteEntity(blizzardEntity);
                    blizzardEntity = -1;
                    for (ChillPenguinPenguinComponent* penguin : penguins) {
                        if (penguin) {
                            penguin->blizzard = false;
                        }
                    }
                  }
                });
            }
        });

        animator->addFrameTrigger("start", 4, [this](){
           esperarIA = true;
        });

        animator->addFrameTrigger("idleShooting", 4, [this](){

            if (!PrefabManager::getInstance().hasPrefab("chillpenguinBullet")) {
                PrefabManager::getInstance().loadPrefab("chillpenguinBullet");
            }
            Entity bulletEntity = PrefabManager::getInstance().instantiatePrefab("chillpenguinBullet", *entityManager);
            Transform* bulletTransform = entityManager->getComponent<Transform>(bulletEntity);
            ChillPenguinBulletComponent* bulletComponent = entityManager->getComponent<ChillPenguinBulletComponent>(bulletEntity);
            if(bulletTransform){
                bulletTransform->posX = transform->posX;
                bulletTransform->posY = transform->posY;
                
                if (animator->spriteRenderer->flip == SDL_FLIP_NONE){
                  bulletTransform->posX = this->transform->posX - 50;
                  bulletComponent->movingRight = false;
                }
                else {
                  bulletTransform->posX = this->transform->posX + 50;
                  bulletComponent->movingRight = true;
                }
                bulletTransform->posY = this->transform->posY - 9;
                bulletComponent->start();
            }
        });

        animator->addFrameTrigger("ice", 4, [this](){
          if (!PrefabManager::getInstance().hasPrefab("chillpenguinIceBullet")) {
            PrefabManager::getInstance().loadPrefab("chillpenguinIceBullet");
          }
    
          std::vector<Entity> iceBullets; // Vector para almacenar las entidades de las balas

          for (int i = 0; i < iceBreathLength; i++) {
            Entity bullet = PrefabManager::getInstance().instantiatePrefab("chillpenguinIceBullet", *entityManager);
            Transform* bulletTransform = entityManager->getComponent<Transform>(bullet);
            ChillPenguinIceBulletComponent* bulletComponent = entityManager->getComponent<ChillPenguinIceBulletComponent>(bullet);
            if (bulletTransform) {
              // Offset: 40 para el primero y luego incrementa de 20 en 20
              int offset = 40 + i * 20;
              if (animator->spriteRenderer->flip == SDL_FLIP_NONE) {
                  bulletTransform->posX = this->transform->posX - offset;
                  bulletComponent->movingRight = false;
              } 
              else {
                  bulletTransform->posX = this->transform->posX + offset;
                  bulletComponent->movingRight = true;
              }
              bulletTransform->posY = this->transform->posY - 9;
              bulletComponent->start();
              audioManager->playSound("Ice");
            
              // Almacenar la entidad para luego eliminarla
              iceBullets.push_back(bullet);
            }
          }
    
          // Se añade un trigger para eliminar todas las balas cuando se active el frame "idle"
          animator->addFrameTrigger("idle", 0, [this, iceBullets]() {
            for (Entity bullet : iceBullets) {
                if (bullet != -1) {
                  entityManager->deleteEntity(bullet);
                }
            }
          });
        });
 
        if (collider)
        {
            collider->removeCollisionTag("Penguin");


            std::function<void(Collider*, CollisionResult)> callbackCollisionEnter = [this](Collider* other, CollisionResult cd) {
                Vector2D resultVector = cd.normal;
    
                // Solo considerar colisión con el suelo (componente Y)
                if (abs(resultVector.y) == 1.0f) {
                    animator->setBool("isGrounded", true);
                    suelo = true;
                    if(hasJumped){
                        hasJumped = false;
                        esperarIA = true;
                    }
                    animator->setBool("isFalling", false);
                    rigidBody->velocityY = 0;  // Resetear velocidad vertical
                    audioManager->playSound("land");
                }

                if (other->tag == "megamanBullet") health->takeDamage(35);
                if (other->tag == "megamanMediumBullet") health->takeDamage(50);
                if (other->tag == "megamanStrongBullet") health->takeDamage(100);
            };

            // Añadir callback onCollisionStay
            std::function<void(Collider*, CollisionResult)> callbackCollisionStay = [this](Collider* other, CollisionResult cd) {
                Vector2D resultVector = cd.normal;
                if (other->getTag() == "Terrain") {
                    // Colisión con suelo
                    if (abs(resultVector.y) == 1.0f) {
                        suelo = true;
                        animator->setBool("isGrounded", true);
                        animator->setBool("isFalling", false);
                    }
                }
            };

            // Modificar el callback onCollisionExit
            std::function<void(Collider*)> callbackCollisionExit = [this](Collider* other) {
                if (other->getTag() == "Terrain") {
                    animator->setBool("isGrounded", false);
                    suelo = false;
                    if(rigidBody->velocityY >= 0.0){
                        animator->setBool("isFalling", true);
                    }
                }
            };

            // Asignar los callbacks
            collider->onCollisionEnter = callbackCollisionEnter;
            collider->onCollisionStay = callbackCollisionStay;
            collider->onCollisionExit = callbackCollisionExit;
            
            animator->addFrameTrigger("spawn", 4, [this](){
                animator->setBool("isActive", true);
            });

            // ---------------------
            //  Stop shooting callbacks
            // ---------------------
            animator->addFrameTrigger("idleShooting", 0, [this] () {
                animator->setBool("isShooting", false);
            });
        }
        setupReinforcementIa();
        loadSoundEffects();
        audioManager->stopMusic();
        // audioManager->playSound("spawn");
        audioManager->playMusic("boss", -1, 0);
    }
    void shoot(float deltaTime){
        inputActive = false;
        hasShoot = true;
        manageShoot(deltaTime);
    }
    
    void iceBreath(float deltaTime){
        inputActive = false;
        hasIceShoot = true;
        manageIcePenguin(deltaTime);
    }

    void death(){
        inputActive = false;
        if (!PrefabManager::getInstance().hasPrefab("smallExplosion")) {
            PrefabManager::getInstance().loadPrefab("smallExplosion");
        }
        Entity explosionEntity = PrefabManager::getInstance().instantiatePrefab("smallExplosion", *entityManager);
        Transform* explosionTransform = entityManager->getComponent<Transform>(explosionEntity);
        SmallExplosionComponent* explosionComponent = entityManager->getComponent<SmallExplosionComponent>(explosionEntity);
        if(explosionTransform){
            explosionTransform->posX = transform->posX;
            explosionTransform->posY = transform->posY;
        }
        //Borrar la explosion
        explosionComponent->start();
        audioManager->stopMusic();
        MegamanLevelsManager::getInstance()->loadLevel("levelSelectorMenu", entityManager->getSceneManager()->getSceneNameByEntityManager(entityManager), 255.0f / 1.5f);
        entityManager->deleteEntity(parent);
    }

    void calcularDireccion(){
        if(player->getTransform()->posX < transform->posX){
            animator->spriteRenderer->flip = SDL_FLIP_NONE;
            direccion = true; // Megaman a la izquierda
        } else {
            animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
            direccion = false; // Megaman a la derecha
        }       
    }

    void update(float deltaTime) override 
    {
        findPlayer();
//-----------------------------
// COOLDOWNS IA
//-----------------------------
        if(hasShoot) shootTimer += deltaTime;
        if(shootTimer >= shootCooldown){
            shootTimer = 0.0f;
            hasShoot = false;
            esperarIA = true;
        }

        if(hasIceShoot) shootIceTimer += deltaTime;
        if(shootIceTimer >= shootIceCooldown){
            shootIceTimer = 0.0f;
            hasIceShoot = false;
            esperarIA = true;
        }

        if(isJumpBlizzardCooldown) {
            jumpBlizzardTimer += deltaTime;
            if(jumpBlizzardTimer >= jumpBlizzardCooldown) {
                jumpBlizzardTimer = 0.0f;
                isJumpBlizzardCooldown = false;
                esperarIA = true;
            }
        }

        if(isDashCooldown) {
            dashCooldownTimer += deltaTime;
            if(dashCooldownTimer >= dashCooldown) {
                dashCooldownTimer = 0.0f;
                isDashCooldown = false;
                esperarIA = true;
            }
        }
//-----------------------------
// COOLDOWNS
//-----------------------------
        if (dashTimeLeft > 0.0f) {
          dashTimeLeft -= deltaTime; 
        }
        if (dashTimeLeft <= 0.0f) {
            rigidBody->velocityX = 0.0f; // Detener el dash suavemente
            animator->setBool("isDashing", false);
            esperarIA = true;
        }

        if (iceTimeLeft > 0.0f) {
          iceTimeLeft -= deltaTime;
          if (iceTimeLeft <= 0.0f) {
              animator->setBool("isIce", false);
              esperarIA = true;
          }
        }

        if (blizzardTimeLeft > 0.0f) {
          blizzardTimeLeft -= deltaTime;
          if (blizzardTimeLeft <= 0.0f) {
              animator->setBool("isBlizzard", false);
              blizzardAttack = false;
              rigidBody->gravity = 9.8f;
          }
        }

        if (!(animator->getBool("isGrounded")) && (rigidBody->velocityY >= 0.0f) && blizzardAttack){
          rigidBody->gravity = 0.0f;
          rigidBody->velocityY = 0.0f;
          animator->setBool("isBlizzard", true);
        }

        
        if (!animator->getBool("isGrounded")){
          if (rigidBody->gravity == 0.0f) return;
          if (animator->spriteRenderer->flip == SDL_FLIP_NONE) {
            rigidBody->velocityX -= jumpForceX;
          } else {
            rigidBody->velocityX += jumpForceX;
          }
        }

        // Buscar nuevas entidades de pingüino
        std::set<Entity> entities = entityManager->getEntities();
        for (Entity entity : entities) {
            ChillPenguinPenguinComponent* penguinComponent = entityManager->getComponent<ChillPenguinPenguinComponent>(entity);
            if (penguinComponent) {
                // Verificar si el pingüino ya está en la lista
                auto it = std::find(penguins.begin(), penguins.end(), penguinComponent);
                if (it == penguins.end()) {
                    // Añadir el pingüino a la lista
                    penguins.push_back(penguinComponent);
                }
            }
        }

        if (animator->getBool("isDashing")) {
          // Definir el rayo en la dirección del dash
          SDL_FPoint origin = { static_cast<float>(transform->posX), static_cast<float>(transform->posY) };
          SDL_FPoint direction = animator->spriteRenderer->flip == SDL_FLIP_NONE ? SDL_FPoint{ -1.0f, 0.0f } : SDL_FPoint{ 1.0f, 0.0f };
          Ray ray(origin, direction);

          // Realizar el raycast
          std::optional<RaycastHit> hit = RayCastManager::raycast(ray, entityManager, 30.0f, {"Penguin"});

          // Si el rayo intersecta con un Collider con la etiqueta "EnemyBullet"
          if (hit.has_value()) {
            Entity hitEntity = hit.value().collider->parent;
            ChillPenguinPenguinComponent* penguinComponent = entityManager->getComponent<ChillPenguinPenguinComponent>(hitEntity);
            if (penguinComponent) {
              // Destruir el pingüino de hielo
              entityManager->deleteEntity(hitEntity);
              audioManager->playSound("Ice_break");
            }
          }
        }

        if(esperarIA){
            esperarTimer += deltaTime;
            if(esperarTimer >= esperarCooldown){
                esperarIA = false;
                esperarTimer = 0.0f;
                inputActive = true;
            }
        }
        
        // -------------------------------------------
        //  UTILS
        // -------------------------------------------
        
        // Falling

        if (!animator->getBool("isGrounded") && 
        (animator->currentState() == "jump" || animator->currentState() == "blizzard") &&
        rigidBody->velocityY > 0.0) {
            animator->setBool("isFalling", true);
        } 
        if (animator->getBool("isGrounded")) {
            animator->setBool("isFalling", false);
        }

        if(health->isDead()){
            death();
        }

        vidaAnterior = vidaActual;
        vidaActual = health->getCurrentHealth();
        vidaMegamanAnterior = vidaMegamanActual;
        if (player) {
            vidaMegamanActual = player->checkHealth();
        }

        // Guardar aprendizaje
        timerGuardarAprendizaje += deltaTime;
        if (timerGuardarAprendizaje >= tiempoGuardarAprendizaje) {
            timerGuardarAprendizaje = 0.0f;
            riaComponent->saveQTable("chillPenguinQTable.json");
        }
    }

};

class ChillPenguinAdvancedIALoader {
    public:

        static ChillPenguinAdvancedIAParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {

            
    
            ChillPenguinAdvancedIAParameters params(
                &entityManager
            );
    
            return params;
        }
    
        // Helper function to directly create a ChillPenguinControlelr component from JSON
        static ChillPenguinAdvancedIA createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
            return ChillPenguinAdvancedIA(fromJSON(j, entityManager));
        }
};

#endif
