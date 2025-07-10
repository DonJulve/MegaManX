#include "Components.h"
#include "InputManager.h"
#include <functional>
#include "Debug.h"
#include "RaycastManager.h"

struct ComponenteDePruebaParameters {
    Transform* transform;
    EntityManager* entityManager;

    ComponenteDePruebaParameters(Transform* t,EntityManager* eM) : transform(t),entityManager(eM) {}
};

struct ComponenteDePrueba : public Component {
    Transform* transform;
    EntityManager* entityManager;
    float m_rayDistance = 100.0f;


    ComponenteDePrueba(ComponenteDePruebaParameters t) : transform(t.transform),entityManager(t.entityManager) {}

    void update(float deltaTime) override
    {
        SDL_FPoint origin = {transform->posX, transform->posY};
        SDL_FPoint direction = {0.0f, 1.0f}; // Down direction
        Ray ray(origin, direction);

        m_rayDistance = 100.0f;

        auto hit = RayCastManager::raycast(ray, entityManager, m_rayDistance, {"Untagged"});

        if (hit) {
            m_rayDistance = hit->distance;
        }
    };

    void render(EntityManager& entityManager,SDL_Renderer* renderer) override
    {
        Camera* camera = nullptr;
        for (Entity entity : entityManager.getEntities()) {
            camera = entityManager.getComponent<Camera>(entity);
            if (camera) break; // Assuming there is only one camera in the scene
        }
        
        SDL_FPoint origin = {transform->posX, transform->posY};
        SDL_FPoint direction = {0.0f, 1.0f}; // Down direction
        Ray ray(origin, direction);

        RayCastManager::renderRay(renderer, ray,*camera, m_rayDistance, {255, 255, 255, 255});
    };
};

class ComponenteDePruebaLoader {
    public:

        static ComponenteDePruebaParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {

            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("ComponenteDePrueba requires a Transform component");
            }
    
            ComponenteDePruebaParameters params(
                transform,
                &entityManager
            );
    
            return params;
        }
    
        // Helper function to directly create a Camera component from JSON
        static ComponenteDePrueba createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
            return ComponenteDePrueba(fromJSON(j, entityManager));
        }
    };