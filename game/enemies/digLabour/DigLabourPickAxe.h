#pragma once

#include "Components.h"
#include "Debug.h"
#include <cmath>

struct DigLabourPickAxeParameters {
    
    Transform* transform;
    EntityManager* entityManager;
    Rigidbody* rigidbody;

    DigLabourPickAxeParameters(Transform* t, EntityManager* em, Rigidbody* rb):
    transform(t), entityManager(em), rigidbody(rb) {}
};

struct DigLabourPickAxe : public Component{
    Transform* transform;
    EntityManager* entityManager;
    Rigidbody* rigidbody;

    Collider* collider;
    bool direccion = true;
    float rotSpeed = 370.0f;

    float time = 0.0f;
    Vector2D velocity = {0.0f, 0.0f};

    std::function<void()> callBackParent;
    std::shared_ptr<MegamanController> player;

    float timeToLive = 5.0f;

    DigLabourPickAxe(DigLabourPickAxeParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    rigidbody(params.rigidbody) {}


    void findPlayer(){
        if(!player) entityManager->findComponent<MegamanController>(player);
    }

    void setPosInicial(Vector2D pos){
        transform->posX = pos.x;
        transform->posY = pos.y;
        calcularVelocidad();
    }

    void calcularVelocidad() {
        findPlayer();
        if (!player) return;
    
        float g = 9.8f;
    
        // Altura máxima (en módulo), tú decides este valor
        velocity.y = 300.0f;
    
        // Diferencia horizontal hacia el jugador
        float xMax = player->getTransform()->posX - transform->posX;

    
        // Velocidad horizontal necesaria para llegar a xMax en ese tiempo
        velocity.x = xMax / 2.0f;
    
        // Asignamos al rigidbody
        rigidbody->velocityX = velocity.x;
        rigidbody->velocityY = -velocity.y;
    
        // Determinar dirección (hacia la derecha o izquierda)
        direccion = velocity.x > 0;
    }
    
    

    void setCallBackParent(std::function<void()> callBack){
        callBackParent = callBack;
    }

    std::function<void(Collider*, CollisionResult)> callbackCollisionEnter = [this](Collider* other, CollisionResult cd) {
        if(other->tag == "Player"){
            if(callBackParent){
                callBackParent();
                entityManager->deleteEntity(parent);
            }
        }
    };

    void start() override{
        collider = entityManager->getComponent<Collider>(parent);
        if(!collider) {
            throw std::runtime_error("DigLabourPickAxe requires a Collider component");
        }
        collider->onCollisionEnter = callbackCollisionEnter;
    }

    void update(float deltaTime) override {
        if(direccion){
            transform->rotZ += rotSpeed * deltaTime;
        }
        else{
            transform->rotZ -= rotSpeed * deltaTime;
        }
        timeToLive -= deltaTime;
        if(timeToLive <= 0.0f){
            
            entityManager->deleteEntity(parent);
        }
    }
};


class DigLabourPickAxeLoader {
    public: 
        static DigLabourPickAxeParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("DigLabourPickAxe requires a Transform component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("DigLabourPickAxe requires a Rigidbody component");
            }

            DigLabourPickAxeParameters params(
                transform,
                &entityManager,
                rigidbody
            );

            return params;
        }

        static DigLabourPickAxe createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return DigLabourPickAxe(fromJSON(j, entityManager));
        }
};