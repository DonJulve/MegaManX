#pragma once

#include "Components.h"
#include "AnimatorComponent.h"
#include "RaycastManager.h"
#include "effects/SmallExplosionComponent.h"


struct SkyClawControllerParameters {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    HealthComponent* healthComponent;

    SkyClawControllerParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb, HealthComponent* hc):
    transform(t), entityManager(em), animator(ac), rigidbody(rb), healthComponent(hc){}
};

struct SkyClawController : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    HealthComponent* healthComponent;

    SkyClawController(SkyClawControllerParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidbody(params.rigidbody),
    healthComponent(params.healthComponent) {}


    bool bajar = false;
    bool atacar = false;
    bool subir = false;

    float timer = 3.0f;
    float timeraux = 0.0f;
    float bajarOffset = 200.0f;
    float posY = 0.0f;

    float timerAtaque = 0.7f;
    bool puedeAtacar = false;
    float velocidadaux = 0.0f;

    Collider* collider;

    void death(){
        if (!PrefabManager::getInstance().hasPrefab("smallExplosion")) {
            PrefabManager::getInstance().loadPrefab("smallExplosion");
        }
        Entity entityExplosion = PrefabManager::getInstance().instantiatePrefab("smallExplosion", *entityManager);
        Transform* explosionTransform = entityManager->getComponent<Transform>(entityExplosion);
        SmallExplosionComponent* explosionComponent = entityManager->getComponent<SmallExplosionComponent>(entityExplosion);
        if(explosionTransform){
            explosionTransform->posX = transform->posX;
            explosionTransform->posY = transform->posY;
            explosionComponent->start();
        }

        //Scamos drop
        if (!PrefabManager::getInstance().hasPrefab("drop")) {
            PrefabManager::getInstance().loadPrefab("drop");
        }
        Entity dropEntity = PrefabManager::getInstance().instantiatePrefab("drop", *entityManager);
        Transform* dropTransform = entityManager->getComponent<Transform>(dropEntity);
        DropController* dropController = entityManager->getComponent<DropController>(dropEntity);
        if(dropTransform){
            dropTransform->posX = transform->posX;
            dropTransform->posY = transform->posY;
        }
        dropController->start();

        entityManager->deleteEntity(parent);
    }

    std::function<void(Collider*, CollisionResult)> callbackCollisionEnter = [this](Collider* other, CollisionResult cd) {
        if (other->tag == "megamanBullet") {
            healthComponent->takeDamage(35);
        }
        if (other->tag == "megamanMediumBullet") {
            healthComponent->takeDamage(50);
        }
        if (other->tag == "megamanStrongBullet") {
            healthComponent->takeDamage(100);
        }
        Debug::Log("Vida: " + std::to_string(healthComponent->getCurrentHealth()));
        if(healthComponent->isDead()){
            Debug::Log("Muerto");
        }
    };

    void start() override {
        collider = entityManager->getComponent<Collider>(parent);
        collider->onCollisionEnter = callbackCollisionEnter;
        rigidbody->velocityX = 100.0f;
        posY = transform->posY;
        animator->addFrameTrigger("atacar", 3, [this](){
            atacar = false;
            subir = true;
            Debug::Log("SUBIR");
            animator->setBool("isAttacking", false);
            puedeAtacar = false;
        });
    }

    void lanzarRayos(){
        SDL_FPoint origin1 = {transform->posX+40, transform->posY};
        SDL_FPoint origin2 = {transform->posX-40, transform->posY};

        SDL_FPoint direction = {0.0f, 1.0f}; // Down direction
        Ray ray(origin1, direction);
        Ray ray2(origin2, direction);

        std::optional<RaycastHit> hit1 = RayCastManager::raycast(ray, entityManager, 300.0f, { "Player" });
        std::optional<RaycastHit> hit2 = RayCastManager::raycast(ray2, entityManager, 300.0f, { "Player" });

        if((hit1.has_value() || hit2.has_value()) && !bajar && !atacar && !subir && puedeAtacar){
            bajar = true;
            timeraux = timer;
            velocidadaux = rigidbody->velocityX;
        }
    }

    void update(float deltaTime) override {
        if(bajar){
            rigidbody->velocityX = 0.0f;
            animator->setBool("isDown", true);
            rigidbody->velocityY = 100.0f;
            if(transform->posY - posY >= bajarOffset){
                puedeAtacar = false;
                Debug::Log("Bajar");
                bajar = false;
                atacar = true;
                animator->setBool("isDown", false);
                animator->setBool("isAttacking", true);
                rigidbody->velocityY = 0.0f;
            }
            return;
        }
        if(atacar) return;
        
        if(subir){
            rigidbody->velocityX = 0.0f;
            transform->posY -= deltaTime * 100.0f;
            if(transform->posY <= posY){
                transform->posY = posY;
                rigidbody->velocityY = 0.0f;
                rigidbody->velocityX = velocidadaux;
                timer = timeraux;
                subir = false;
            }
            return;
        }
        lanzarRayos();
        Debug::Log("NO BAJANDO");
        timer -= deltaTime;
        if(timer <= 0.0f){
            timer = 3.0f;
            rigidbody->velocityX *= -1.0f;
        }
        timerAtaque -= deltaTime;
        if(timerAtaque <= 0.0f){
            puedeAtacar = true;
            timerAtaque = 0.7f;
        }
        if(healthComponent->isDead()){
            death();
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
        
        SDL_FPoint origin1 = {transform->posX+40, transform->posY};
        SDL_FPoint origin2 = {transform->posX-40, transform->posY};
        SDL_FPoint origin = { static_cast<float>(transform->posX), static_cast<float>(transform->posY) };

        SDL_FPoint direction = {0.0f, 1.0f}; // Down direction
        Ray ray(origin1, direction);
        Ray ray2(origin2, direction);
        Ray middleRay(origin, direction);

        //RayCastManager::renderRay(renderer, ray,*camera, 300.0f, {255, 255, 255, 255});
        //RayCastManager::renderRay(renderer, ray2,*camera, 300.0f, {255, 255, 255, 255});
    };

};

class SkyClawControllerLoader {
    public:
    
        static SkyClawControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("SkyClawController requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("SkyClawController requires a AnimatorComponent component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("SkyClawController requires a Rigidbody component");
            }

            HealthComponent* healthComponent = entityManager.getComponent<HealthComponent>(entityManager.getLast());
            if (!healthComponent) {
                throw std::runtime_error("SkyClawController requires a HealthComponent component");
            }

            return SkyClawControllerParameters(transform, &entityManager, animator, rigidbody, healthComponent);
        }

        static SkyClawController createFromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            SkyClawControllerParameters params = fromJSON(j, entityManager);
            return SkyClawController(params);
        }

};