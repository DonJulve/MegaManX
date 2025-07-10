#pragma once
#include "Components.h"
#include "InputManager.h"
#include "Debug.h"
#include "AnimatorComponent.h"

struct TombotBootsControllerParameters {
    Transform* transform;
    EntityManager* entityManager;
    Rigidbody* rigidbody;

    TombotBootsControllerParameters(Transform* t, EntityManager* em, Rigidbody* rb) :
        transform(t), entityManager(em), rigidbody(rb) {}
};


struct TombotBootsController : public Component {
    Transform* transform;
    EntityManager* entityManager;
    Rigidbody* rigidbody;

    TombotBootsController(TombotBootsControllerParameters params) :
        transform(params.transform),
        entityManager(params.entityManager),
        rigidbody(params.rigidbody) {}

    
    void start() override {
        rigidbody->velocityY = 200.0f;
    }

    void update(float deltaTime) override {
    }
};

class TombotBootsControllerLoader {
    public:
        static TombotBootsControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("TombotBootsController requires a Transform component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("TombotBootsController requires a Rigidbody component");
            }

            return TombotBootsControllerParameters(transform, &entityManager, rigidbody);
        }

        static TombotBootsController createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return TombotBootsController(fromJSON(j, entityManager));
        }
};