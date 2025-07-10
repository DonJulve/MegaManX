#include "Components.h"

CameraParameters::CameraParameters(Transform* t, float z, int windowW, int windowH)
    : transform(t), zoom(z), windowWidth(windowW), windowHeight(windowH) {}

Camera::Camera(CameraParameters cp)
    : transform(cp.transform), zoom(cp.zoom), windowWidth(cp.windowWidth), windowHeight(cp.windowHeight) {
    aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
}

Camera::Camera()
    : transform(nullptr), zoom(1), windowWidth(800), windowHeight(600) {
    aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
}

void Camera::start() {}

void Camera::update(float deltaTime) {}

void Camera::updateScaling(int newWindowWidth, int newWindowHeight) {
    windowWidth = newWindowWidth;
    windowHeight = newWindowHeight;
    aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    zoom = static_cast<float>(windowWidth) / 800.0f;
}

float Camera::getPosX() {
    return transform->posX;
}

float Camera::getPosY() {
    return transform->posY;
}

CameraParameters CameraLoader::fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
    Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
    if (!transform) {
        throw std::runtime_error("Camera requiere un componente Transform");
    }
    CameraParameters params(
        transform,
        j.value("zoom", 1.0f),
        j.value("windowWidth", 800),
        j.value("windowHeight", 600)
    );
    return params;
}

Camera CameraLoader::createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
    return Camera(fromJSON(j, entityManager));
}