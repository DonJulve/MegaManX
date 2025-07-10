#pragma once

#include "Components.h"
#include "AnimatorComponent.h"
#include "RaycastManager.h"
#include "Debug.h"

struct  LavaParameters {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    LavaParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb):
    transform(t), entityManager(em), animator(ac), rigidBody(rb) {}
};

struct LavaComponent : public Component{
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    Collider* collider;

    LavaComponent(LavaParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidBody(params.rigidBody) {}

    bool suelo = false;

    std::function<void(Collider*, CollisionResult)> callbackCollisionEnter = [this](Collider* other, CollisionResult cd) {
        if (other->tag == "Terrain"){
            rigidBody->velocityX = 0;
        }
        if(suelo && other->tag == "EnemyBullet"){
            collider->scaleY = 40;
            collider->scaleX = 30;
            transform->posY = transform->posY - 20;
            animator->setBool("isBurning", true);
            collider->setTag("EnemyBullet");
        }

    };

    void setVelocity(float x, float y) {
        rigidBody->velocityX = x;
        rigidBody->velocityY = y;
    }

    void start() override {
        collider = entityManager->getComponent<Collider>(parent);
        if (!collider) {
            throw std::runtime_error("LavaComponent requires a Collider component");
        }
        collider->onCollisionEnter = callbackCollisionEnter;
        
        animator->addFrameTrigger("land", 3, [this](){
            // Destruir el fuego
            animator->setBool("staying", true);
        });

        animator->addFrameTrigger("burn", 3, [this](){
            entityManager->deleteEntity(parent);
        });
    }

    void update(float deltaTime) override {
        if(!suelo){
            SDL_FPoint origin = { static_cast<float>(transform->posX), static_cast<float>(transform->posY) };
            SDL_FPoint direction = SDL_FPoint{ 0.0f, 1.0f };
    
            Ray ray(origin, direction);
            
            // Realizar el raycast
            std::optional<RaycastHit> hit = RayCastManager::raycast(ray, entityManager, 10.0f, { "Terrain" });
    
            if(hit.has_value()){
                suelo = true;
                Debug::Log("Hay suelo");
                rigidBody->kinematic = true;
                rigidBody->mass = 0;
                collider->scaleX = 40;
                collider->scaleY = 10;
                collider->isTrigger = true;
                rigidBody->velocityX = 0;
                rigidBody->velocityY = 0;
                animator->setBool("isLanded", true);
                transform->posY = hit.value().point.y - 2;
            }
        }
        
    }

    //TODO; ES PARA DEBUG
    void render(EntityManager& entityManager,SDL_Renderer* renderer) override
    {
        Camera* camera = nullptr;
        for (Entity entity : entityManager.getEntities()) {
            camera = entityManager.getComponent<Camera>(entity);
            if (camera) break; // Assuming there is only one camera in the scene
        }
        
        SDL_FPoint origin = {transform->posX, transform->posY};
        SDL_FPoint direction = {0.0f, 1.0f}; // Down direction
        Ray ray(origin, direction);

        // RayCastManager::renderRay(renderer, ray,*camera, 10.0f, {255, 255, 255, 255});
    };
};

class LavaLoader {
    public:
        static LavaParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("LavaComponent requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("LavaComponent requires a AnimatorComponent component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("LavaComponent requires a Rigidbody component");
            }

            LavaParameters params(
                transform,
                &entityManager,
                animator,
                rigidbody
            );

            return params;
        }

        static LavaComponent createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return LavaComponent(fromJSON(j, entityManager));
        }

    };
