#ifndef CHILLPENGUINCONTROLLER_H
#define CHILLPENGUINCONTROLLER_H

#include <SDL2/SDL.h>
#include "Components.h"
#include "InputManager.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "Debug.h"
#include "AudioManager.h"
#include "RaycastManager.h"
#include "ChillPenguinBlizzardComponent.h"
#include "ChillPenguinBulletComponent.h"
#include "ChillPenguinIceBulletComponent.h"
#include "ChillPenguinPenguinComponent.h"

#include <algorithm>
#include <string>

class ChillPenguinControllerParameters {
public:
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    ChillPenguinControllerParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb): transform(t), entityManager(em), animator(ac), rigidBody(rb) {}
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
    AudioManager* audioManager;
    
// -------------------------------------------
//  Global variables
// -------------------------------------------
    bool dashPressed = false;
    bool shootPressed = false;
    bool icePressed = false;
    bool blizzardPressed = false;
    bool blizzardAttack = false;

    float dashForce = 200.0f; 
    float dashDuration = 1.0f;  
    float dashTimeLeft = 0.0f;

    float iceDuration = 2.0f;  
    float iceTimeLeft = 0.0f; 

    float blizzardDuration = 2.0f;  
    float blizzardTimeLeft = 0.0f;  

    float jumpForceY = 900.0f;
    float jumpForceX = 200.0f;

    float snowSpawnPositionX = 0.0f;
    float snowSpawnPositionY = 0.0f;

    int iceBreathLength = 7;

    Entity blizzardEntity;        
    std::vector<ChillPenguinPenguinComponent*> penguins; // Lista de pingüinos
// -------------------------------------------
//  Managing KeyMaps
// -------------------------------------------
    void manageMoveLeft(float deltaTime) 
    {
        if (!animator->getBool("isActive")) return;
        if (!animator->getBool("isGrounded")) return;
        if (animator->getBool("isShooting")) return;
        if (animator->getBool("isDashing")) return;
        if (animator->getBool("isIce")) return;
        if (animator->getBool("isBlizzard")) return;

        animator->spriteRenderer->flip = SDL_FLIP_NONE;
    }

    void manageMoveRight(float deltaTime) 
    {
        if (!animator->getBool("isActive")) return;
        if (!animator->getBool("isGrounded")) return;
        if (animator->getBool("isShooting")) return;
        if (animator->getBool("isDashing")) return;
        if (animator->getBool("isIce")) return;
        if (animator->getBool("isBlizzard")) return;

        animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
    }

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

      if (dashPressed) return;
    
      if (animator->spriteRenderer->flip == SDL_FLIP_NONE) {
        rigidBody->velocityX = -dashForce;
      } else {
        rigidBody->velocityX = dashForce;
      }
    
      animator->setBool("isDashing", true);
      audioManager->playSound("dash");

      dashTimeLeft = dashDuration;
      dashPressed = true;
    }

    void manageShoot(float deltaTime)
    {
        if (!animator->getBool("isActive")) return;
        if (animator->getBool("isDashing")) return;
        if (!animator->getBool("isGrounded")) return;
        if (animator->getBool("isShooting")) return;
        if (animator->getBool("isIce")) return;
        if (animator->getBool("isBlizzard")) return;

        if (shootPressed) return;

        animator->setBool("isShooting", true);
        shootPressed = true;
        audioManager->playSound("shoot");
        
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

        if (icePressed) return;
    
        animator->setBool("isIce", true);

        iceTimeLeft = iceDuration;
        icePressed = true;
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

        if (blizzardPressed) return;
        

        animator->setBool("isGrounded", false);
        
        rigidBody->velocityY -= jumpForceY;

        blizzardTimeLeft = blizzardDuration;
        blizzardPressed = true;
        blizzardAttack = true;
    }

// -------------------------------------------
//  Utilities
// -------------------------------------------
    void loadSoundEffects() {
        audioManager = AudioManager::getInstance();

        audioManager->loadSound("spawn", "sound_effects/chill penguin/Boss Intro.wav");
        audioManager->loadSound("dash", "sound_effects/chill penguin/Dash.wav");
        audioManager->loadSound("Ice", "sound_effects/chill penguin/Chill Penguin Breath.wav");
        audioManager->loadSound("Blizzard", "sound_effects/chill penguin/Blizzard.wav");
        audioManager->loadSound("Ice_break", "sound_effects/chill penguin/Ice Break.wav");
    }

public: 
    ChillPenguinController(ChillPenguinControllerParameters params) : transform(params.transform), 
    entityManager(params.entityManager), animator(params.animator), rigidBody(params.rigidBody) {}


    void start() override 
    { 
        blizzardEntity = -1;
        Collider* collider = entityManager->getComponent<Collider>(parent);
        
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
                  bulletTransform->posX = this->transform->posX - 40;
                  bulletComponent->movingRight = false;
                }
                else {
                  bulletTransform->posX = this->transform->posX + 40;
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

                //Tocar suelo despues de salto
                animator->setBool("isGrounded", true);
                animator->setBool("isFalling", false);
                audioManager->playSound("land");
            };
            collider->onCollisionEnter = callbackCollisionEnter;

            std::function<void(Collider*)> callbackCollisionExit = [this](Collider* other) {
                animator->setBool("isGrounded", false);
                //Nos caemos de una plataforma
                if(rigidBody->velocityY >= 0.0){
                    animator->setBool("isFalling", true);
                }
            };

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

        loadSoundEffects();
        audioManager->playSound("spawn");

        
    }

    void update(float deltaTime) override 
    {
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
        //  Movement Controller
        // -------------------------------------------
        if (InputManager::isKeyPressed(SDL_SCANCODE_A) && animator->getBool("isGrounded")){
            manageMoveLeft(deltaTime);
        }
        else if (InputManager::isKeyPressed(SDL_SCANCODE_D) && animator->getBool("isGrounded")){
            manageMoveRight(deltaTime);
        }

        //Gestion Dash
        if (InputManager::isKeyPressed(SDL_SCANCODE_M) && !dashPressed) {
            manageDash(deltaTime);
        }
        if (!InputManager::isKeyPressed(SDL_SCANCODE_M)) {
            dashPressed = false;
        }
        //Gestion Saltar
        if (InputManager::isKeyPressed(SDL_SCANCODE_W)){
            manageJump(deltaTime);
        }
        //Gestion Disparo niebla
        if (InputManager::isKeyPressed(SDL_SCANCODE_N) && !blizzardPressed) {
            manageBlizzard(deltaTime);
        }
        if (!InputManager::isKeyPressed(SDL_SCANCODE_N)){
            blizzardPressed = false;
        }
        //Gestion Disparo pinguinos de hielo
        if (InputManager::isKeyPressed(SDL_SCANCODE_P) && !icePressed) {
            manageIcePenguin(deltaTime);
        }
        if (!InputManager::isKeyPressed(SDL_SCANCODE_P)){
            icePressed = false;
        }
        //Gestion Disparar
        if (InputManager::isKeyPressed(SDL_SCANCODE_SPACE) && !shootPressed){
            manageShoot(deltaTime);
        }
        if (!InputManager::isKeyPressed(SDL_SCANCODE_SPACE)){
            shootPressed = false;
        }
        
        // -------------------------------------------
        //  UTILS
        // -------------------------------------------
        
        // Falling

        if ((animator->currentState() == "jump" || animator->currentState() == "blizzard")
             && rigidBody->velocityY > 0.0){
            animator->setBool("isFalling", true);
        }
    }

};

class ChillPenguinControllerLoader {
    public:

        static ChillPenguinControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {

            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("ChillPenguinController requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("ChillPenguinController requires a AnimatorComponent component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("ChillPenguinController requires a Rigidbody component");
            }
    
            ChillPenguinControllerParameters params(
                transform,
                &entityManager,
                animator,
                rigidbody
            );
    
            return params;
        }
    
        // Helper function to directly create a ChillPenguinControlelr component from JSON
        static ChillPenguinController createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
            return ChillPenguinController(fromJSON(j, entityManager));
        }
};

#endif
