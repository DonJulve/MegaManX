#include "Systems.h"
#include <iostream>
#include "Debug.h"

void PhysicsSystem::update(EntityManager& manager, float deltaTime) {
    for (Entity entity : manager.getEntities()) {
        auto transform = manager.getComponent<Transform>(entity);
        auto rigidbody = manager.getComponent<Rigidbody>(entity);

        if (!transform || !rigidbody) continue;  // Skip if there is no transform or rigidbody

        if (!rigidbody->kinematic) {  // Non-kinematic objects are affected by forces

            // Apply gravity (gravity should affect all objects, regardless of mass)
            rigidbody->accelerationY += rigidbody->gravity * rigidbody->mass;

            // Update velocity using acceleration
            rigidbody->velocityX += rigidbody->accelerationX * deltaTime;
            rigidbody->velocityY += rigidbody->accelerationY * deltaTime;

            rigidbody->velocityX *= (1.0f - rigidbody->drag * deltaTime);
            rigidbody->velocityY *= (1.0f - rigidbody->drag * deltaTime);

            // Apply velocity limits (if needed)
            //rigidbody->velocityX = std::abs(rigidbody->velocityX > 0.01) ? rigidbody->velocityX : 0;
            //rigidbody->velocityY = std::abs(rigidbody->velocityY > 0.01) ? rigidbody->velocityY : 0;
        }

        // Update position based on velocity
        transform->posX += rigidbody->velocityX * deltaTime;
        transform->posY += rigidbody->velocityY * deltaTime;

        // Reset acceleration after applying it for the frame (forces must be recalculated next frame)
        rigidbody->accelerationX = 0.0f;
        rigidbody->accelerationY = 0.0f;
    }
}

void CollisionSystem::update(EntityManager& manager, float deltaTime) {
    auto& entities = manager.getEntities();
    std::vector<Collider*> dynamicColliders;

    // Collect dynamic colliders
    for (auto& entity : entities) {
        auto collider = manager.getComponent<Collider>(entity);
        auto rigidbody = manager.getComponent<Rigidbody>(entity);

        if (collider && rigidbody) {
            if(collider->tag != "Terrain") dynamicColliders.push_back(collider);
        }
    }

    if (dynamicEntities) {
        delete dynamicEntities;
    }
    
    dynamicEntities = new BVH(2);
    dynamicEntities->Build(dynamicColliders);

    // Check collisions for dynamic objects
    for (auto* colliderA : dynamicColliders) {
        ;

        // Retrieve potential colliders from both quadtrees
        std::vector<Collider*> possibleStaticCollisions = staticEntities->GetPotentialCollisions(colliderA->getBoundingBox());
        std::vector<Collider*> possibleDynamicCollisions = dynamicEntities->GetPotentialCollisions(colliderA->getBoundingBox());

        for (auto* colliderB : possibleStaticCollisions) {
            if (colliderA == colliderB) continue; // Avoid self-collision

            // Get entity and components
            Entity entityA = colliderA->parent;
            Entity entityB = colliderB->parent;

            auto rigidbodyA = manager.getComponent<Rigidbody>(entityA);
            auto transformA = manager.getComponent<Transform>(entityA);

            auto rigidbodyB = manager.getComponent<Rigidbody>(entityB);
            auto transformB = manager.getComponent<Transform>(entityB);

            Vector2D velocityA = {0, 0 };
            Vector2D velocityB = {0, 0 };

            if (rigidbodyA) {
                velocityA.x = rigidbodyA->velocityX;
                velocityA.y = rigidbodyA->velocityY;
            }

            if (rigidbodyB) {
                velocityB.x = rigidbodyB->velocityX;
                velocityB.y = rigidbodyB->velocityY;
            }

            // Check collision
            CollisionResult result = colliderA->checkCollision(colliderB,velocityA, velocityB, deltaTime);
            
            

            if (result.isColliding) {

                colliderA->colPoint.x = result.collisionPoint.x;
                colliderA->colPoint.y = result.collisionPoint.y;
                colliderB->colPoint.x = result.collisionPoint.x;
                colliderB->colPoint.y = result.collisionPoint.y;

                //Debug::Log("Collision detected between " + std::to_string(entityA) + " and " + std::to_string(entityB));

                ResolveCollision(colliderA, rigidbodyA, transformA, colliderB, rigidbodyB, transformB, result);
            }
        }

        for (auto* colliderB : possibleDynamicCollisions) {
            if (colliderA == colliderB) continue; // Avoid self-collision

            // Get entity and components
            Entity entityA = colliderA->parent;
            Entity entityB = colliderB->parent;

            auto rigidbodyA = manager.getComponent<Rigidbody>(entityA);
            auto transformA = manager.getComponent<Transform>(entityA);

            auto rigidbodyB = manager.getComponent<Rigidbody>(entityB);
            auto transformB = manager.getComponent<Transform>(entityB);

            Vector2D velocityA = {0, 0 };
            Vector2D velocityB = {0, 0 };
            if (rigidbodyA) {
                velocityA.x = rigidbodyA->velocityX;
                velocityA.y = rigidbodyA->velocityY;
            }
            if (rigidbodyB) {
                velocityB.x = rigidbodyB->velocityX;
                velocityB.y = rigidbodyB->velocityY;
            }



            // Check collision
            CollisionResult result = colliderA->checkCollision(colliderB,velocityA, velocityB, deltaTime);
            
            

            if (result.isColliding) {

                colliderA->colPoint.x = result.collisionPoint.x;
                colliderA->colPoint.y = result.collisionPoint.y;
                colliderB->colPoint.x = result.collisionPoint.x;
                colliderB->colPoint.y = result.collisionPoint.y;

                //Debug::Log("Collision detected between " + std::to_string(entityA) + " and " + std::to_string(entityB));

                ResolveCollision(colliderA, rigidbodyA, transformA, colliderB, rigidbodyB, transformB, result);
            }
        }
    }
}


void CollisionSystem::ResolveCollision(Collider* col1, Rigidbody* rb1, Transform* t1, 
                                      Collider* col2, Rigidbody* rb2, Transform* t2, 
                                      const CollisionResult& collisionResult) {
    if (!col1 || !col2 || col1->isTrigger || col2->isTrigger || !rb1 || !rb2 || !collisionResult.isColliding) {
        return;
    }

    // Vector de velocidad relativa (rb1 respecto a rb2)
    float relativeVelX = rb1->velocityX - rb2->velocityX;
    float relativeVelY = rb1->velocityY - rb2->velocityY;

    // Producto escalar entre velocidad relativa y normal de colisión
    float dotProduct = relativeVelX * collisionResult.normal.x + 
                      relativeVelY * collisionResult.normal.y;

    if(col1->parent == 220) Debug::Log("velocity y: " + std::to_string(rb1->velocityY) + ", " + std::to_string(rb2->velocityY) + ", normal: " + std::to_string(collisionResult.normal.y));
    if(col1->parent == 220) Debug::Log("Dot product: " + std::to_string(dotProduct));

    // Si se alejan, no corregir
    if (dotProduct > 0) {
        return;
    }

    float absX = std::abs(collisionResult.normal.x);
    float absY = std::abs(collisionResult.normal.y);

    Vector2D normal;

    if (absX > absY) {
        // X-axis dominates: snap to left or right
        normal = collisionResult.normal.x > 0 ? Vector2D{1.0f, 0.0f} : Vector2D{-1.0f, 0.0f};
    } else {
        // Y-axis dominates: snap to up or down
        normal = collisionResult.normal.y > 0 ? Vector2D{0.0f, 1.0f} : Vector2D{0.0f, -1.0f};
    }

    t1->posX += collisionResult.normal.x * collisionResult.penetrationDepth;
    t1->posY += collisionResult.normal.y * collisionResult.penetrationDepth;

    rb1->velocityX = 0;
    rb1->velocityY = 0;
    
    col1->updateBoundingBox();
}


void RenderSystem::largest_4_3_resolution(int x, int y, int* out_width, int* out_height, int* excess_width, int* excess_height) {
    // Option 1: Try to use full height
    double temp_width = (4.0 / 3.0) * y;

    if (temp_width <= x) {
        *out_width = static_cast<int>(temp_width);
        *out_height = y;
    } else {
        // Option 2: Use full width
        double temp_height = (3.0 / 4.0) * x;
        *out_width = x;
        *out_height = static_cast<int>(temp_height);
    }

    *excess_width = x - *out_width;
    *excess_height = y - *out_height;
}


void RenderSystem::render(EntityManager& manager, SDL_Renderer* renderer) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        
    // Retrieve the camera
    Camera* camera = nullptr;
    for (Entity entity : manager.getEntities()) {
        camera = manager.getComponent<Camera>(entity);
        if (camera) break;
    }

    if (!camera) return;

    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    largest_4_3_resolution(windowWidth, windowHeight, &camera->outWidth, &camera->outHeight, &camera->excessWidth, &camera->excessHeight);

    camera->updateScaling(camera->outWidth, camera->outHeight);

    for (Entity entity : manager.getEntities()) {
        auto parallax = manager.getComponent<ParallaxLayer>(entity);
        if (parallax) 
        {
            parallax->render(manager, renderer);
        }
    }

    // Render regular entities first (world space)
    for (Entity entity : manager.getEntities()) {
        auto uiComponent = manager.getComponent<UIComponent>(entity);
        if (uiComponent) continue;  // Skip UI components for now

        auto parallax = manager.getComponent<ParallaxLayer>(entity);
        if (parallax) continue;

        auto transform = manager.getComponent<Transform>(entity);
        auto spriteRenderer = manager.getComponent<SpriteRenderer>(entity);

        if (!transform || !spriteRenderer) continue;
        
        // Compute world space position relative to the camera
        float worldX = transform->posX - camera->getPosX();
        float worldY = transform->posY - camera->getPosY();

        // Calculate scaled dimensions
        float scaledWidth = spriteRenderer->width * transform->sizeX * camera->zoom;
        float scaledHeight = spriteRenderer->height * transform->sizeY * camera->zoom;

        // Compute render position in screen space
        float renderX = worldX * camera->zoom + camera->windowWidth / 2.0f;
        float renderY = worldY * camera->zoom + camera->windowHeight / 2.0f;

        // Adjust for anchor point
        if (spriteRenderer->centerMode == AnchorPoint::CENTER) {
            renderX -= scaledWidth / 2.0f;
            renderY -= scaledHeight / 2.0f;
        } else if (spriteRenderer->centerMode == AnchorPoint::TOP_LEFT) {
            // No adjustment needed
        }

        renderX += camera->excessWidth / 2.0f; // Adjust for excess width
        renderY += camera->excessHeight / 2.0f; // Adjust for excess height

        // Convert to integer pixel coordinates, matching collider logic
        int x = static_cast<int>(std::floor(renderX));          // Top-left x (inclusive start)
        int y = static_cast<int>(std::floor(renderY));          // Top-left y (inclusive start)
        int w = static_cast<int>(std::ceil(renderX + scaledWidth)) - x; // Width to exclusive end
        int h = static_cast<int>(std::ceil(renderY + scaledHeight)) - y; // Height to exclusive end

        // Define the destination rectangle
        SDL_Rect destRect = {x, y, w, h};

        // Center point for rotation (relative to destRect)
        Vector2D centerFloat = {scaledWidth / 2.0f, scaledHeight / 2.0f};
        SDL_Point center = {static_cast<int>(std::floor(centerFloat.x)), 
                        static_cast<int>(std::floor(centerFloat.y))};
        if (spriteRenderer->centerMode == AnchorPoint::TOP_LEFT) {
            center = {0, 0};
        }

        // Set texture alpha
        SDL_SetTextureAlphaMod(spriteRenderer->texture, spriteRenderer->alpha);

        // Render the sprite
        SDL_RenderCopyEx(
            renderer,
            spriteRenderer->texture,
            nullptr,           // Source rect (full texture)
            &destRect,         // Destination rect
            transform->rotZ,   // Rotation angle
            &center,           // Center point for rotation
            spriteRenderer->flip
        );

    }

    // Render UI components last (screen space)
    for (Entity entity : manager.getEntities()) {
        UIComponent* uiComponent = manager.getComponent<UIComponent>(entity);
        if (uiComponent) {
            uiComponent->render(manager, renderer);
        }
    }

    // Draw black rectangles in the unused areas (pillarbox/letterbox regions) after UI rendering
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black color
    if (camera->excessWidth > 0) {
        // Pillarboxing: Draw left and right black rectangles
        SDL_Rect leftBar = {0, 0, camera->excessWidth / 2, windowHeight};
        SDL_Rect rightBar = {windowWidth - (camera->excessWidth / 2), 0, camera->excessWidth / 2, windowHeight};
        SDL_RenderFillRect(renderer, &leftBar);
        SDL_RenderFillRect(renderer, &rightBar);
    } else if (camera->excessHeight > 0) {
        // Letterboxing: Draw top and bottom black rectangles
        SDL_Rect topBar = {0, 0, windowWidth, camera->excessHeight / 2};
        SDL_Rect bottomBar = {0, windowHeight - (camera->excessHeight / 2), windowWidth, camera->excessHeight / 2};
        SDL_RenderFillRect(renderer, &topBar);
        SDL_RenderFillRect(renderer, &bottomBar);
    }


}

