#ifndef MEGAMANCONTROLLER_H
#define MEGAMANCONTROLLER_H

#include "Components.h"
#include "DynamicInputManager.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "AudioManager.h"
#include "MegamanBulletComponent.h"
#include "MegamanMediumBulletComponent.h"
#include "MegamanStrongBulletComponent.h"
#include "megamanArmor/MegamanArmorController.h"
#include "HealthComponent.h"
#include "PrefabManager.h"
#include "MegamanCharge.h"

#include <string>

class MegamanControllerParameters {
public:
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    HealthComponent* health;

    MegamanControllerParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb, HealthComponent* hc):
        transform(t), entityManager(em), animator(ac), rigidBody(rb), health(hc) {}
};

class MegamanController : public Component {
private:
    // -------------------------------------------
    //  Megaman Properties
    // -------------------------------------------
    EntityManager* entityManager;    
    Transform* transform;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    AudioManager* audioManager;
    HealthComponent* health;

    bool pauseEnabled = true;

    bool playable = false;
    float startTimer = 1.0f;

    Vector2D movementDirection = {0.0f, 0.0f};

    // -------------------------------------------
    //  Singleton
    // -------------------------------------------
    static MegamanController* instance;
    static std::vector<std::function<void()>> deletionCallbacks;

    void destroyInstance();
    // -------------------------------------------

    // -------------------------------------------
    //  Key Management
    // -------------------------------------------
    
    bool jumpKeyHeld = false;
    bool shootKeyHeld = false;
    bool chargingSoundPlaying = false;
    int chargingSoundChannel;
    // -------------------------------------------
    //  Timers
    // -------------------------------------------
    const float wallJumpDuration = 0.15f;
    const float shootingDuration = 0.2f;
    const float jumpingDuration = 0.5f;
    const float timeAfterHit = 1.0f;
    const float timeMediumBullet = 0.8f;
    const float timeStrongBullet = 1.5f;
    const float dashingDuration = 0.5f;
    
    float shootingTimeLeft = 0.0f;
    float wallJumpTimeLeft = 0.0f;
    float jumpingTimeLeft = 0.0f;
    float timeAfterHitLeft = 0.0f;
    float timeChargingShoot = 0.0f;
    float dashingTimeLeft = 0.0f;

    // -------------------------------------------
    //  Private Methods
    // -------------------------------------------
    void updateTimers(float deltaTime);
    void manageNotWalking(float deltaTime);
    void manageMoveLeft(float deltaTime);
    void manageMoveRight(float deltaTime);
    void manageJump(float deltaTime);
    void manageShoot(float deltaTime);
    void shootBullet();
    void initializeWeakBullet();
    void initializeMediumBullet();
    void initializeStrongBullet();
    void loadSoundEffects();

    MegamanChargeComponent* charge = nullptr;

    Vector2D normalize(const Vector2D& v);
    Vector2D getTangent(const Vector2D& normal);
    

public: 
    MegamanController(MegamanControllerParameters params);
    ~MegamanController();

    Transform* getTransform();
    AnimatorComponent* getAnimator();

    // Spawnea al megaman saltando
    void removeMegaman();
    void addHealth(float health);
    void startFromArmor(float deltaTime);
    void start() override;
    void update(float deltaTime) override;
    float checkHealth();

    static MegamanController* getInstance(std::function<void()> callback);
    static MegamanController* getInstance();
};

class MegamanControllerLoader {
public:
    static MegamanControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager);
    static MegamanController createFromJSON(nlohmann::json& j, EntityManager& entityManager);
};

#endif
