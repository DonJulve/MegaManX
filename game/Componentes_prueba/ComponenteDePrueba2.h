#include "Components.h"
#include "InputManager.h"
#include <functional>
#include "AnimatorComponent.h"
#include "Debug.h"

struct ComponenteDePrueba2Parameters {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;

    ComponenteDePrueba2Parameters(Transform* t,EntityManager* em,AnimatorComponent* ac, Rigidbody* rb): transform(t), entityManager(em), animator(ac), rigidbody(rb)  {}
};

struct ComponenteDePrueba2 : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidbody;

    ComponenteDePrueba2(ComponenteDePrueba2Parameters cdp2) : transform(cdp2.transform), 
    entityManager(cdp2.entityManager), animator(cdp2.animator), rigidbody(cdp2.rigidbody) {}

    void start() override
    {
        Collider* collider = entityManager->getComponent<Collider>(parent);

        Debug::Log("Asignando callback");

        if(collider)
        {
            std::function<void(Collider*)> callback = [this](Collider* other) {
                Debug::Log("Colision detectada en componente de prueba");
                Debug::Log(std::to_string(transform->posX) + " " + std::to_string(transform->posY));
            };
            //collider->onCollisionEnter = callback;
            Debug::Log("Callback de colision asignado");
        }

        
    }

    void update(float deltaTime) override
    {
        if (InputManager::isKeyPressed(SDL_SCANCODE_A))
        {
            transform->posX -= 50.0f * deltaTime;
            animator->setBool("isWalking", true);
            animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
        }   
        else if (InputManager::isKeyPressed(SDL_SCANCODE_D))
        {
            transform->posX += 50.0f * deltaTime;
            animator->setBool("isWalking", true);
            animator->spriteRenderer->flip = SDL_FLIP_NONE;
        }
        else
        {
            animator->setBool("isWalking", false);
        }

        if (InputManager::isKeyHeld(SDL_SCANCODE_SPACE))
        {
            rigidbody->velocityY = -500.0f;
        }
            
    }
};

class ComponenteDePrueba2Loader {
    public:

        static ComponenteDePrueba2Parameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {

            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("ComponenteDePrueba2 requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("ComponenteDePrueba2 requires a AnimatorComponent component");
            }

            Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidbody) {
                throw std::runtime_error("ComponenteDePrueba2 requires a Rigidbody component");
            }
    
            ComponenteDePrueba2Parameters params(
                transform,
                &entityManager,
                animator,
                rigidbody
            );
    
            return params;
        }
    
        // Helper function to directly create a Camera component from JSON
        static ComponenteDePrueba2 createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
            return ComponenteDePrueba2(fromJSON(j, entityManager));
        }
    };