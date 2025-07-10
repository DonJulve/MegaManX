#include "UIComponent.h"

using json = nlohmann::json;

// Implementation of UILoader::fromJSON
UIParameters UILoader::fromJSON(const json& j, EntityManager& entityManager) {
    Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
    if (!transform) {
        throw std::runtime_error("SpriteRenderer requiere un componente Transform");
    }
    
    return UIParameters(transform);
}

// Implementation of UILoader::createFromJSON
UIComponent UILoader::createFromJSON(json& j, EntityManager& entityManager) {
    // Get UI parameters from JSON
    UIParameters params = fromJSON(j, entityManager);
    
    // Create and return UIComponent with the parameters
    return UIComponent(params);
}

void UIComponent::render(EntityManager& entityManager, SDL_Renderer* renderer) {
    auto spriteRenderer = entityManager.getComponent<SpriteRenderer>(parent);

    if (!spriteRenderer) return;

    Camera* camera = nullptr;
    for (Entity entity : entityManager.getEntities()) {
        camera = entityManager.getComponent<Camera>(entity);
        if (camera) break;
    }

    if (!camera) return;

    // Convert percentage to pixel coordinates
    int renderX = static_cast<int>(transform->posX * camera->windowWidth) + camera->excessWidth / 2;
    int renderY = static_cast<int>(transform->posY * camera->windowHeight) + camera->excessHeight / 2;

    
    
    // Scale width and height
    float scaleFactor = static_cast<float>(camera->windowWidth) / 800.0f;
    int scaledWidth = static_cast<int>(spriteRenderer->width * transform->sizeX * scaleFactor);

    scaleFactor = static_cast<float>(camera->windowHeight) / 600.0f;
    int scaledHeight = static_cast<int>(spriteRenderer->height * transform->sizeY * scaleFactor);

    SDL_Point center = { scaledWidth / 2, scaledHeight / 2 }; // Default center

    // Adjust for anchor points
    if (spriteRenderer->centerMode == AnchorPoint::TOP_LEFT) {
        center = { 0, 0 };
    } else if (spriteRenderer->centerMode == AnchorPoint::CENTER) {
        renderX -= scaledWidth / 2;
        renderY -= scaledHeight / 2;
    }

    SDL_Rect destRect = { renderX, renderY, scaledWidth, scaledHeight };

    SDL_SetTextureAlphaMod(spriteRenderer->texture, spriteRenderer->alpha);
    
    SDL_RenderCopyEx(
        renderer,
        spriteRenderer->texture,
        nullptr,
        &destRect,
        transform->rotZ,
        &center,
        spriteRenderer->flip
    );
}