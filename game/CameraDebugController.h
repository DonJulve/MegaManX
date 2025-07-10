#include "Components.h"
#include "InputManager.h"
#include <functional>
#include "AnimatorComponent.h"
#include "Debug.h"
#include "RenderTextManager.h"


struct CameraDebugControllerParameters {
    Transform* transform;
    EntityManager* entityManager;

    CameraDebugControllerParameters(Transform* t,EntityManager* em): transform(t), entityManager(em) {}
};

struct CameraDebugController : public Component {
    Transform* transform;
    EntityManager* entityManager;
    Camera* camera;
    float zoomCD = 0.0f;

    SDL_FPoint* p1;
    SDL_FPoint* p2;
    SDL_FPoint* p3;

    CameraDebugController(CameraDebugControllerParameters cdp2) : transform(cdp2.transform), 
    entityManager(cdp2.entityManager) 
    {
        p1 = nullptr;
        p2 = nullptr;
    }

    void start() override
    {
        Collider* collider = entityManager->getComponent<Collider>(parent);

        Debug::Log("Asignando callback");

        if(collider)
        {
            std::function<void(Collider*,CollisionResult)> callback = [this](Collider* other,CollisionResult cd) {
                Debug::Log("Colision detectada en componente de prueba");
                Debug::Log(std::to_string(transform->posX) + " " + std::to_string(transform->posY));
            };
            //collider->onCollisionEnter = callback;
            Debug::Log("Callback de colision asignado");
        }

        

        RenderTextManager::getInstance().loadFont("selectedFont", "sprites/letters/lettersSelected", 7, 7);
        
    }

    void update(float deltaTime) override
    {

        camera = entityManager->getComponent<Camera>(parent);

        if (InputManager::isKeyPressed(SDL_SCANCODE_LSHIFT))
        {
            if (InputManager::isKeyPressed(SDL_SCANCODE_W)) transform->posY -= 2000.0f * deltaTime * (1/camera->zoom);
            if (InputManager::isKeyPressed(SDL_SCANCODE_A)) transform->posX -= 2000.0f * deltaTime * (1/camera->zoom);
            if (InputManager::isKeyPressed(SDL_SCANCODE_S)) transform->posY += 2000.0f * deltaTime * (1/camera->zoom);
            if (InputManager::isKeyPressed(SDL_SCANCODE_D)) transform->posX += 2000.0f * deltaTime * (1/camera->zoom);
        }
        else
        {
            if (InputManager::isKeyPressed(SDL_SCANCODE_W)) transform->posY -= 500.0f * deltaTime * (1/camera->zoom);
            if (InputManager::isKeyPressed(SDL_SCANCODE_A)) transform->posX -= 500.0f * deltaTime * (1/camera->zoom);
            if (InputManager::isKeyPressed(SDL_SCANCODE_S)) transform->posY += 500.0f * deltaTime * (1/camera->zoom);
            if (InputManager::isKeyPressed(SDL_SCANCODE_D)) transform->posX += 500.0f * deltaTime * (1/camera->zoom);
        }

        if (InputManager::isMouseButtonPressed(SDL_BUTTON_LEFT)) {
            SDL_FPoint mousePos = InputManager::screenToWorld(InputManager::getMouseX(), InputManager::getMouseY(), *camera);
            
            if (!p1) p1 = new SDL_FPoint{mousePos.x, mousePos.y};
            else if (!p2) p2 = new SDL_FPoint{mousePos.x, mousePos.y};
            
        }

        if (p1 && p2) {
            // Determine top-left and bottom-right based on the point coordinates
            float leftX = std::min(p1->x, p2->x); // Minimum X is the leftmost point
            float topY = std::min(p1->y, p2->y);  // Minimum Y is the topmost point
            
            float rightX = std::max(p1->x, p2->x); // Maximum X is the rightmost point
            float bottomY = std::max(p1->y, p2->y); // Maximum Y is the bottommost point
            
            // Calculate width and height
            float width = rightX - leftX;  // Distance between left and right X
            float height = bottomY - topY; // Distance between top and bottom Y
        
            // Set rotation to 0 (no rotation)
            float angle = 0.0f;
        
            // Create entity and add components
            Entity entity = entityManager->createEntity();
            TransformParameters tp(leftX, topY, angle, width, height, AnchorPoint::TOP_LEFT);
            Transform* transform = new Transform(tp);
            entityManager->addComponent(entity, std::type_index(typeid(Transform)), transform);
        
            ColliderParameters cp(transform, 1.0f, 1.0f, false, "DebugTerrain");
            Collider* collider = new Collider(cp);
            entityManager->addComponent(entity, std::type_index(typeid(Collider)), collider);
        
            RigidbodyParameters rp(0, 0, 100, true, 9.8, 0.01, 0.0);
            Rigidbody* rb = new Rigidbody(rp);
            entityManager->addComponent(entity, std::type_index(typeid(Rigidbody)), rb);
        
            // Debug output
            collider->updateBoundingBox();
        
            // Clean up the point data
            delete p1;
            delete p2;
            p1 = nullptr;
            p2 = nullptr;
        }
        

        if(InputManager::isKeyPressed(SDL_SCANCODE_LCTRL))
        {
            if(p1)
            {
                delete p1;
                p1 = nullptr;
            }

            if(p2)
            {
                delete p2;
                p2 = nullptr;
            }
        }

        if(zoomCD > 0.0f)
        {
            zoomCD -= deltaTime;
        }

        if(InputManager::isKeyPressed(SDL_SCANCODE_UP))
        {
            camera->zoom += 0.1f;
            if(camera->zoom > 2.0f) camera->zoom = 2.0f;
            zoomCD = 0.05;
        }

        if(InputManager::isKeyPressed(SDL_SCANCODE_DOWN))
        {
            camera->zoom -= 0.1f;
            if(camera->zoom < 0.1f) camera->zoom = 0.1f;
            zoomCD = 0.05;
        }



        

        if (InputManager::isMouseButtonPressed(SDL_BUTTON_RIGHT)) {
            Debug::Log("Right mouse button pressed");
            SDL_FPoint mousePos = InputManager::screenToWorld(InputManager::getMouseX(), InputManager::getMouseY(), *camera);
            Entity entityToDelete = -1;
            Collider* colliderToDelete = nullptr;
            bool found = false;
            
            Debug::Log("Starting entity search");
            for (Entity entity : entityManager->getEntities()) {
                Collider* collider = entityManager->getComponent<Collider>(entity);
                if (collider && collider->tag == "DebugTerrain") {
                    if (mousePos.x >= collider->boundingBox.x && 
                        mousePos.x <= collider->boundingBox.x + collider->boundingBox.w &&
                        mousePos.y >= collider->boundingBox.y && 
                        mousePos.y <= collider->boundingBox.y + collider->boundingBox.h) {
                        found = true;
                        entityToDelete = entity;
                        colliderToDelete = collider;
                        break;
                    }
                }
            }
            
            if (found) {
                // Extract AABB coordinates (assuming boundingBox is in world space)
                float x1 = colliderToDelete->boundingBox.x;                    // Top-left
                float y1 = colliderToDelete->boundingBox.y;
                float x2 = colliderToDelete->boundingBox.x + colliderToDelete->boundingBox.w - 1;  // Top-right
                float y2 = colliderToDelete->boundingBox.y;
                float x3 = colliderToDelete->boundingBox.x;                    // Bottom-left
                float y3 = colliderToDelete->boundingBox.y + colliderToDelete->boundingBox.h - 1;
                float x4 = colliderToDelete->boundingBox.x + colliderToDelete->boundingBox.w - 1;  // Bottom-right
                float y4 = colliderToDelete->boundingBox.y + colliderToDelete->boundingBox.h - 1;
                float midX = x1 + colliderToDelete->boundingBox.w / 2.0f;      // Center
                float midY = y1 + colliderToDelete->boundingBox.h / 2.0f;
                CollisionResult result;
                Triangle triangle;
            
                // Debug output
                std::cout << "Mouse Pos: (" << mousePos.x << ", " << mousePos.y << ")\n";
                std::cout << "Zoom: " << camera->zoom << "\n";
                std::cout << "AABB: (" << x1 << ", " << y1 << ") to (" << x4 << ", " << y4 << ")\n";
                std::cout << "Center: (" << midX << ", " << midY << ")\n";

                Vector2D velocity1 = {0, 0};
                Vector2D velocity2 = {0, 0};
            
                // Top triangle
                if (colliderToDelete->checkTop) {
                    triangle.p1 = {x1, y1};
                    triangle.p2 = {x2, y2};
                    triangle.p3 = {midX, midY};
                    bool topResult = colliderToDelete->pointInTriangle(mousePos.x, mousePos.y, triangle, result, velocity1, velocity2);
                    if (topResult) {
                        colliderToDelete->checkTop = false;
                        std::cout << "Disabled Top triangle\n";
                    }
                }
                // Left triangle
                if (colliderToDelete->checkLeft) {
                    triangle.p1 = {x1, y1};
                    triangle.p2 = {midX, midY};
                    triangle.p3 = {x3, y3};
                    bool leftResult = colliderToDelete->pointInTriangle(mousePos.x, mousePos.y, triangle, result, velocity1, velocity2);
                    if (leftResult) {
                        colliderToDelete->checkLeft = false;
                        std::cout << "Disabled Left triangle\n";
                    }
                }
                // Right triangle
                if (colliderToDelete->checkRight) {
                    triangle.p1 = {x2, y2};
                    triangle.p2 = {midX, midY};
                    triangle.p3 = {x4, y4};
                    bool rightResult = colliderToDelete->pointInTriangle(mousePos.x, mousePos.y, triangle, result, velocity1, velocity2);
                    if (rightResult) {
                        colliderToDelete->checkRight = false;
                        std::cout << "Disabled Right triangle\n";
                    }
                }
                // Bottom triangle
                if (colliderToDelete->checkBottom) {
                    triangle.p1 = {x3, y3};
                    triangle.p2 = {x4, y4};
                    triangle.p3 = {midX, midY};
                    bool bottomResult = colliderToDelete->pointInTriangle(mousePos.x, mousePos.y, triangle, result, velocity1, velocity2);
                    if (bottomResult) {
                        colliderToDelete->checkBottom = false;
                        std::cout << "Disabled Bottom triangle\n";
                    }
                }
                // Check for entity deletion
                if (!colliderToDelete->checkTop && !colliderToDelete->checkLeft && 
                    !colliderToDelete->checkRight && !colliderToDelete->checkBottom) {
                    entityManager->deleteEntity(entityToDelete);
                    std::cout << "Entity " << entityToDelete << " deleted\n";
                }
            }
        }

        


        if (InputManager::isKeyPressed(SDL_SCANCODE_LSHIFT))
        {
            if(InputManager::isKeyPressed(SDL_SCANCODE_SPACE))
            {
                saveTerrainToJson(*entityManager);
            }
        }
        else
        {
            if(InputManager::isKeyPressed(SDL_SCANCODE_SPACE))
            {
                SDL_FPoint mousePos = InputManager::screenToWorld(InputManager::getMouseX(), InputManager::getMouseY(), *camera);
            
                if (p3) delete p3;
                p3 = new SDL_FPoint{mousePos.x, mousePos.y};
            }
        }
            
    }

    void render(EntityManager& entityManager,SDL_Renderer* renderer) override
    {
        if(p1)
        {
            SDL_FPoint screenPos = InputManager::worldToScreen(p1->x, p1->y, *camera);
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderDrawPoint(renderer, (int)screenPos.x, (int)screenPos.y);
            SDL_Rect dot = {(int)screenPos.x - 1, (int)screenPos.y - 1, 3, 3};
            SDL_RenderFillRect(renderer, &dot);
        }

        RenderTextManager::getInstance().renderText("selectedFont", std::to_string(camera->zoom), 0.8f, 0.0f, 4, 4, &entityManager);
        RenderTextManager::getInstance().renderText("selectedFont", std::to_string(camera->getPosX()) + "   " + std::to_string(camera->getPosY()), 0.0f, 0.8f, 4, 4, &entityManager);

        //RenderTextManager::getInstance().renderText("selectedFont", std::to_string(camera->zoom), 0.0f, 0.0f, 4, 4, *camera);
        if(!p3) return;
        RenderTextManager::getInstance().renderText("selectedFont", std::to_string(p3->x) + "   " + std::to_string(p3->y), 0.0f, 0.0f, 4, 4, &entityManager);
        
    }

    void saveTerrainToJson(EntityManager& manager) {
        // Create a json object to hold all entities
        nlohmann::ordered_json entitiesJson;
    
        // Iterate over all entities in the manager
        auto& entities = manager.getEntities();
        for (const auto& entity : entities) {
            nlohmann::ordered_json entityJson;
            nlohmann::ordered_json componentsJson;
    
            // Get components
            auto transform = manager.getComponent<Transform>(entity);
            auto collider = manager.getComponent<Collider>(entity);
            auto rigidbody = manager.getComponent<Rigidbody>(entity);
    
            // Skip entities that don't have all required components
            if (!transform || !collider || !rigidbody) continue;
    
            // Only serialize entities with tag "DebugTerrain"
            if (collider->tag != "DebugTerrain") continue;
    
            // Serialize Transform component
            if (transform) {
                nlohmann::ordered_json transformJson;
                transformJson["posX"] = transform->posX;
                transformJson["posY"] = transform->posY;
                transformJson["rotZ"] = transform->rotZ;
                transformJson["sizeX"] = transform->sizeX;
                transformJson["sizeY"] = transform->sizeY;
                transformJson["anchor"] = transform->centerMode;  // Assuming anchor is a string enum
    
                componentsJson["Transform"] = transformJson;
            }
    
            // Serialize Collider component
            if (collider) {
                nlohmann::ordered_json colliderJson;
                colliderJson["scaleX"] = collider->scaleX;
                colliderJson["scaleY"] = collider->scaleY;
                colliderJson["isTrigger"] = collider->isTrigger;
                colliderJson["tag"] = "Terrain";
    
                // Serialize collisionTags if any exist
                if (!collider->collisionTags.empty()) {
                    colliderJson["collisionTags"] = collider->collisionTags;
                }
    
                // Serialize specific collider checks (assuming they're flags for collision checking)
                colliderJson["checkTop"] = collider->checkTop;
                colliderJson["checkLeft"] = collider->checkLeft;
                colliderJson["checkRight"] = collider->checkRight;
                colliderJson["checkBottom"] = collider->checkBottom;
    
                componentsJson["Collider"] = colliderJson;
            }
    
            // Serialize Rigidbody component
            if (rigidbody) {
                nlohmann::ordered_json rigidbodyJson;
                rigidbodyJson["velocityX"] = rigidbody->velocityX;
                rigidbodyJson["velocityY"] = rigidbody->velocityY;
                rigidbodyJson["mass"] = rigidbody->mass;
                rigidbodyJson["gravity"] = rigidbody->gravity;
                rigidbodyJson["drag"] = rigidbody->drag;
                rigidbodyJson["bounceFactor"] = rigidbody->bounceFactor;
                rigidbodyJson["kinematic"] = rigidbody->kinematic;
    
                componentsJson["Rigidbody"] = rigidbodyJson;
            }
    
            // Add components JSON to the entity's JSON
            entityJson["components"] = componentsJson;
    
            // Add the entity to the entities array
            entitiesJson.push_back(entityJson);
        }
    
        // Save the entities JSON to a file
        std::ofstream file("resources/entities.json");
        if (file.is_open()) {
            file << entitiesJson.dump(4);  // Pretty print with 4 spaces
            file.close();
            Debug::Log("Entities saved to entities.json");
        } else {
            Debug::Log("Failed to open entities.json for saving");
        }
    }
    
};

class CameraDebugControllerLoader {
    public:

        static CameraDebugControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {

            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("CameraDebugController requires a Transform component");
            }
    
            CameraDebugControllerParameters params(
                transform,
                &entityManager
            );
    
            return params;
        }
    
        // Helper function to directly create a Camera component from JSON
        static CameraDebugController createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
            return CameraDebugController(fromJSON(j, entityManager));
        }
    };