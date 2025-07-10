#ifndef MEGAMANARMORCONTROLLER_H
#define MEGAMANARMORCONTROLLER_H

#include "DynamicInputManager.h"
#include "Components.h"
#include "InputManager.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "HealthComponent.h"
#include "megaman/MegamanController.h"
#include "PrefabManager.h"

struct MegamanArmorControllerParameters {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;
    HealthComponent* health;

    MegamanArmorControllerParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb, HealthComponent* hc):
        transform(t), entityManager(em), animator(ac), rigidbody(rb), health(hc) {}
};

struct MegamanArmorController : public Component {
private:
    // -------------------------------------------
    //  Armor Properties
    // -------------------------------------------
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    Collider* collider;
    HealthComponent* health;
    bool toKill = false;
    // -------------------------------------------
    //  Timers
    // -------------------------------------------
    const float attackDuration = 0.2f;
    const float dashingDuration = 0.5f;
    const float timeAfterHit = 1.0f;

    float dashingTimeLeft = 0.0f;
    float attackTimeLeft = 0.0f;
    float jumpingTimeLeft = 0.0f;
    float timeAfterHitLeft = 0.0f;

    // -------------------------------------------
    //  Key Management
    // -------------------------------------------
    bool jumpKeyHeld = false;
    bool jumpDashing = false;

    // -------------------------------------------
    //  Managing KeyMaps
    // -------------------------------------------
    void updateTimers(float deltaTime);
    void manageNotWalking(float deltaTime);
    void manageMoveLeft(float deltaTime);
    void manageMoveRight(float deltaTime);
    void manageAttack(float deltaTime);
    void manageJump(float deltaTime);
    void manageExitArmor(float deltaTime);

public:
    // -------------------------------------------
    //  Constructor
    // -------------------------------------------
    MegamanArmorController(MegamanArmorControllerParameters params);

    // -------------------------------------------
    //  Start
    // -------------------------------------------
    void activate();
    void start() override;

    // -------------------------------------------
    //  Update
    // -------------------------------------------
    void update(float deltaTime) override;
    void render(EntityManager& entityManager,SDL_Renderer* renderer) override;
};

class MegamanArmorControllerLoader {
public:
    static MegamanArmorControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager);
    static MegamanArmorController createFromJSON(nlohmann::json& j, EntityManager& entityManager);
};

#endif