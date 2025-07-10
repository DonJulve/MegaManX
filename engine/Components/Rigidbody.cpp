#include "Components.h"

RigidbodyParameters::RigidbodyParameters(float vx, float vy, float m, bool kin, float g, float d, float b)
    : velocityX(vx), velocityY(vy), accelerationX(0), accelerationY(0), mass(m), gravity(g), drag(d), kinematic(kin), bounceFactor(b) {}

Rigidbody::Rigidbody(RigidbodyParameters rbp)
    : velocityX(rbp.velocityX), velocityY(rbp.velocityY), accelerationX(rbp.accelerationX), accelerationY(rbp.accelerationY),
      mass(rbp.mass), gravity(rbp.gravity), drag(rbp.drag), kinematic(rbp.kinematic), bounceFactor(rbp.bounceFactor) {}

Rigidbody::Rigidbody()
    : velocityX(0.0f), velocityY(0.0f), accelerationX(0.0f), accelerationY(0.0f), mass(1.0f), gravity(9.8f), drag(0.01f), kinematic(false), bounceFactor(0.5f) {}

void Rigidbody::ApplyForce(float forceX, float forceY) {
    if (kinematic) return;
    accelerationX += forceX / mass;
    accelerationY += forceY / mass;
}

void Rigidbody::start() {}

void Rigidbody::update(float deltaTime) {}

RigidbodyParameters RigidbodyLoader::fromJSON(const nlohmann::json& j, const EntityManager& entityManager) {
    RigidbodyParameters params;
    params.velocityX = j.value("velocityX", 0.0f);
    params.velocityY = j.value("velocityY", 0.0f);
    params.accelerationX = 0;
    params.accelerationY = 0;
    params.mass = j.value("mass", 1.0f);
    params.gravity = j.value("gravity", 9.8f);
    params.drag = j.value("drag", 0.01f);
    params.bounceFactor = j.value("bounceFactor", 0.5f);
    params.kinematic = j.value("kinematic", false);
    return params;
}

Rigidbody RigidbodyLoader::createFromJSON(const nlohmann::json& j, const EntityManager& entityManager) {
    return Rigidbody(fromJSON(j, entityManager));
}