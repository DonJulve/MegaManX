#include "Components.h"
#include "EntityManager.h"


struct FlamemammothFloorParameters {
    Transform* transform;
    EntityManager* entityManager;
    Rigidbody* rigidBody;

    FlamemammothFloorParameters(Transform* t, EntityManager* em, Rigidbody* rb)
        : transform(t), entityManager(em), rigidBody(rb), {}
};

struct FlamemammothFloor : public Component {
    Transform* transform;
    EntityManager* entityManager;
    Rigidbody* rigidBody;

    Collider* collider;

    FlamemammothFloor(FlamemammothFloorParameters params) :
        transform(params.transform),
        entityManager(params.entityManager),
        rigidBody(params.rigidBody) {}

    void start() override {
        collider = entityManager->getComponent<Collider>(parent);
        if (!collider) {
            throw std::runtime_error("FlamemammothFloor requires a Collider component");
        }
    }

    void update(float deltaTime) override {
        // Update logic for the FlamemammothFloor can be added here
    }
};

class FlamemammothFloorLoader {
    public:
        static FlamemammothFloorParameters fromJSON(const nlohmann::json& j, const EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("FlamemammothController requires a Transform component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("FlamemammothController requires a Rigidbody component");
            }
            return FlamemammothFloorParameters(transform, &entityManager, rigidbody);
        }

        static FlamemammothFloor createFromJSON(const nlohmann::json& j, const EntityManager& entityManager) {
            FlamemammothFloorParameters params = fromJSON(j, entityManager);
            return FlamemammothFloor(params);
        }
};