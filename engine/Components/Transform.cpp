#include "Components.h" // Or "TransformComponent.h" if you name it that

Transform::Transform(TransformParameters tp)
    : posX(tp.posX), posY(tp.posY), rotZ(tp.rotZ), sizeX(tp.sizeX), sizeY(tp.sizeY), centerMode(tp.centerMode) {}

Transform::Transform()
    : posX(0), posY(0), rotZ(0), sizeX(0), sizeY(0), centerMode(AnchorPoint::CENTER) {}

void Transform::start() {
    // Empty for now, as in the original
}

void Transform::update(float deltaTime) {
    // Empty for now, as in the original
}

TransformParameters TransformLoader::fromJSON(const nlohmann::json& j, const EntityManager& entityManager) {
    TransformParameters params;
    params.posX = j.value("posX", 0.0f);
    params.posY = j.value("posY", 0.0f);
    params.rotZ = j.value("rotZ", 0.0f);
    params.sizeX = j.value("sizeX", 0.0f);
    params.sizeY = j.value("sizeY", 0.0f);
    params.centerMode = j.value("anchor", AnchorPoint::CENTER);
    return params;
}

Transform TransformLoader::createFromJSON(const nlohmann::json& j, const EntityManager& entityManager) {
    return Transform(fromJSON(j, entityManager));
}