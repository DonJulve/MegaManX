#ifndef CHILLPENGUINCONTROLLER_H
#define CHILLPENGUINCONTROLLER_H

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

#include <algorithm>
#include <string>

class ChillPenguinControllerParameters {
public:

    EntityManager* entityManager;
    

    ChillPenguinControllerParameters(EntityManager* em): entityManager(em) {}
};

class ChillPenguinController : public Component {
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
    ClassicIAComponent* iaComponent;
    
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

    bool inputActive = false;
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
// -------------------------------------------
//  Managing KeyMaps
// -------------------------------------------
    void manageJump(float deltaTime) 
    {
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

    void setup_ia(){
        iaComponent->setInitialState("Idle");

        iaComponent->registerAction("Idle", [this](EntityManager* entityManager, float deltaTime){
            inputActive = true;
        });

        iaComponent->registerAction("Shoot", [this](EntityManager* entityManager, float deltaTime){
            if(inputActive) shoot(deltaTime);
        });
        
        iaComponent->registerAction("Ice", [this](EntityManager* entityManager, float deltaTime){
            if(inputActive) iceBreath(deltaTime);
        });

        iaComponent->registerAction("Dash", [this](EntityManager* entityManager, float deltaTime){
            if(inputActive) {
                manageDash(deltaTime);
                inputActive = false;
            }
        });

        iaComponent->registerAction("JumpOrBlizzard", [this](EntityManager* entityManager, float deltaTime){
            if(inputActive) {
                if(rand() % 2 == 0) {
                    manageJump(deltaTime);
                } 
                else {
                    manageBlizzard(deltaTime);
                }
                inputActive = false;
            }
        });

        iaComponent->registerAction("Death", [this](EntityManager* entityManager, float deltaTime){
            death();
        });

        iaComponent->registerCondition("Idle", "Shoot", [this](EntityManager* entityManager, float deltaTime){
            Vector2D normal = {0.0,0.0};

            if(player){
                normal.x = transform->posX - player->getTransform()->posX;
                normal.y = transform->posY - player->getTransform()->posY;
            }

            if (hasShoot) return false;
            if(normal.y > agroDistanceY && normal.y < -agroDistanceY){
                 return false;
            }
            if(normal.x <= agroDistanceX && normal.x >= 0){ //Megaman a la izquierda
                 direccion = true;
                 return true;
            }
            if(normal.x >= -agroDistanceX && normal.x < 0){ //Megaman a la derecha
                 direccion = false;
                 return true;
             }

            // Aleatorización cuando ambas condiciones ("Shoot" e "Ice") son válidas
            bool canShoot = (std::abs(normal.x) <= agroDistanceX);
            bool canIce = (std::abs(normal.x) <= iceAgroDistanceX && !hasIceShoot);

            if (canShoot && canIce) {
                return (rand() % 2 == 0);
            }
            return canShoot;
        }); 

        iaComponent->registerCondition("Idle", "Ice", [this](EntityManager* entityManager, float deltaTime) {
            Vector2D normal = {0.0, 0.0};
            if (player) {
                normal.x = transform->posX - player->getTransform()->posX;
                normal.y = transform->posY - player->getTransform()->posY;
            }

            if (hasIceShoot) return false;
            if(normal.y > agroDistanceY && normal.y < -agroDistanceY){
                 return false;
            }
            if(normal.x <= agroDistanceX && normal.x >= 0){ //Megaman a la izquierda
                 direccion = true;
                 return true;
            }
            if(normal.x >= -agroDistanceX && normal.x < 0){ //Megaman a la derecha
                 direccion = false;
                 return true;
            }

            // Aleatorización cuando ambas condiciones son válidas
            bool canShoot = (std::abs(normal.x) <= agroDistanceX && !hasShoot);
            bool canIce = (std::abs(normal.x) <= iceAgroDistanceX);

            if (canShoot && canIce) {
                return (rand() % 2 == 1); // 50% de probabilidad (complementario a Shoot)
            }
            return canIce;
        });

        // Condición para entrar en dash
        iaComponent->registerCondition("Idle", "Dash", [this](EntityManager* entityManager, float deltaTime){
            Vector2D normal = {0.0,0.0};
        
            if(player){
                normal.x = transform->posX - player->getTransform()->posX;
                normal.y = transform->posY - player->getTransform()->posY;
            }
            
            if(isDashCooldown) return false;
            if(normal.y > agroDistanceY && normal.y < -agroDistanceY){
                 return false;
            }
            if(normal.x <= mediumDistance && normal.x >= 0){ //Megaman a la izquierda
                 direccion = true;
                 return true;
            }
            if(normal.x >= -mediumDistance && normal.x < 0){ //Megaman a la derecha
                 direccion = false;
                 return true;
            }
            return true;
        });

        iaComponent->registerCondition("Idle", "JumpOrBlizzard", [this](EntityManager* entityManager, float deltaTime){
            Vector2D normal = {0.0,0.0};
        
            if(player){
                normal.x = transform->posX - player->getTransform()->posX;
                normal.y = transform->posY - player->getTransform()->posY;
            }

            if(isJumpBlizzardCooldown) return false;

            if(normal.y > agroDistanceY && normal.y < -agroDistanceY){
                 return false;
            }
            if(normal.x <= farDistance && normal.x >= 0){ //Megaman a la izquierda
                 direccion = true;
                 return true;
            }
            if(normal.x >= -farDistance && normal.x < 0){ //Megaman a la derecha
                 direccion = false;
                 return true;
             }
        
            return true;
        });

        iaComponent->registerCondition("Shoot", "Idle", [this](EntityManager* entityManager, float deltaTime){
            return !animator->getBool("isShooting");
        });

        iaComponent->registerCondition("Ice", "Idle", [this](EntityManager* entityManager, float deltaTime){
            return !animator->getBool("isIce"); 
        });

        iaComponent->registerCondition("Dash", "Idle", [this](EntityManager* entityManager, float deltaTime){
            return dashTimeLeft <= 0.0f;
        });

        iaComponent->registerCondition("JumpOrBlizzard", "Idle", [this](EntityManager* entityManager, float deltaTime){
            bool isGrounded = animator->getBool("isGrounded");
            if (isGrounded) {
                isJumpBlizzardCooldown = true; // Activar cooldown
                jumpBlizzardTimer = 0.0f;      // Reiniciar timer
            }
            return isGrounded;
        });

        iaComponent->registerCondition("Idle", "Death", [this](EntityManager* entityManager, float deltaTime){
            return health->isDead();
        });

        iaComponent->registerCondition("Shoot", "Death", [this](EntityManager* entityManager, float deltaTime){
            return health->isDead();
        });

        iaComponent->registerCondition("Ice", "Death", [this](EntityManager* entityManager, float deltaTime){
            return health->isDead();
        });

        iaComponent->registerCondition("Dash", "Death", [this](EntityManager* entityManager, float deltaTime){
            return health->isDead();
        });

        iaComponent->registerCondition("JumpOrBlizzard", "Death", [this](EntityManager* entityManager, float deltaTime){
            return health->isDead();
        });
    }

public: 
    ChillPenguinController(ChillPenguinControllerParameters params) : transform(nullptr), 
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

        iaComponent = entityManager->getComponent<ClassicIAComponent>(parent);
        if (!iaComponent) {
            throw std::runtime_error("RaybitController requires a ClassicIAComponent component");
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
                        animator->setBool("isGrounded", true);
                        animator->setBool("isFalling", false);
                    }
                }
            };

            // Modificar el callback onCollisionExit
            std::function<void(Collider*)> callbackCollisionExit = [this](Collider* other) {
                if (other->getTag() == "Terrain") {
                    animator->setBool("isGrounded", false);
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
        setup_ia();
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
        }

        if(hasIceShoot) shootIceTimer += deltaTime;
        if(shootIceTimer >= shootIceCooldown){
            shootIceTimer = 0.0f;
            hasIceShoot = false;
        }

        if(isJumpBlizzardCooldown) {
            jumpBlizzardTimer += deltaTime;
            if(jumpBlizzardTimer >= jumpBlizzardCooldown) {
                jumpBlizzardTimer = 0.0f;
                isJumpBlizzardCooldown = false;
            }
        }

        if(isDashCooldown) {
            dashCooldownTimer += deltaTime;
            if(dashCooldownTimer >= dashCooldown) {
                dashCooldownTimer = 0.0f;
                isDashCooldown = false;
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
        }

        if (iceTimeLeft > 0.0f) {
          iceTimeLeft -= deltaTime;
          if (iceTimeLeft <= 0.0f) {
              animator->setBool("isIce", false);
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
    }

};

class ChillPenguinControllerLoader {
    public:

        static ChillPenguinControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {

            
    
            ChillPenguinControllerParameters params(
                &entityManager
            );
    
            return params;
        }
    
        // Helper function to directly create a ChillPenguinControlelr component from JSON
        static ChillPenguinController createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
            return ChillPenguinController(fromJSON(j, entityManager));
        }
};

#endif
