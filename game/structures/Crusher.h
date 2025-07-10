#include "Components.h"

struct CrusherParameters
{
    Transform *transform;
    EntityManager *entityManager;
    Rigidbody *rigidbody;

    CrusherParameters(Transform *t, EntityManager *em, Rigidbody *rb) : transform(t), entityManager(em), rigidbody(rb) {}
};

struct Crusher : public Component
{
    Transform *transform;
    EntityManager *entityManager;
    Rigidbody *rigidbody;

    Crusher(CrusherParameters params) : transform(params.transform),
                                        entityManager(params.entityManager),
                                        rigidbody(params.rigidbody) {}

    float posY = 0.0f;

    float abajo = 1.5f;
    bool isDown = false;
    bool tocaSubir = false;

    bool tocaBajar = false;
    float arriba = 0.7f;

    void start() override
    {
        posY = transform->posY;
    }

    void update(float deltaTime) override
    {

        if (isDown)
        {
            if (abajo > 0.0f)
            {
                abajo -= deltaTime;
            }
            else
            {
                tocaSubir = true;
                isDown = false;
                abajo = 1.5f;
            }
            return;
        }

        if (tocaSubir)
        {
            transform->posY -= deltaTime * 100.0f;
            if (transform->posY <= posY)
            {
                transform->posY = posY;
                tocaSubir = false;
            }
            return;
        }

        if (tocaBajar)
        {
            if (arriba > 0.0f)
            {
                arriba -= deltaTime;
            }
            else
            {
                rigidbody->velocityY = 170.0f;
                tocaBajar = false;
            }
        }

        SDL_FPoint origin1 = {transform->posX + 40, transform->posY};
        SDL_FPoint origin2 = {transform->posX - 40, transform->posY};
        SDL_FPoint origin = {static_cast<float>(transform->posX), static_cast<float>(transform->posY) + 100};

        SDL_FPoint direction = {0.0f, 1.0f}; // Down direction
        Ray ray(origin1, direction);
        Ray ray2(origin2, direction);
        Ray middleRay(origin, direction);

        std::optional<RaycastHit> hit1 = RayCastManager::raycast(ray, entityManager, 500.0f, {"Enemy", "Player"});
        std::optional<RaycastHit> hit2 = RayCastManager::raycast(ray2, entityManager, 500.0f, {"Enemy", "Player"});
        std::optional<RaycastHit> hitCenter = RayCastManager::raycast(middleRay, entityManager, 70.0f, {"Terrain"});

        if (hit1.has_value() || hit2.has_value())
        {
            tocaBajar = true;
        }
        if (hitCenter.has_value())
        {
            isDown = true;
            rigidbody->velocityY = 0.0f;
        }
    }

    // TODO; ES PARA DEBUG
    void render(EntityManager &entityManager, SDL_Renderer *renderer) override
    {
        Camera *camera = nullptr;
        for (Entity entity : entityManager.getEntities())
        {
            camera = entityManager.getComponent<Camera>(entity);
            if (camera)
                break; // Assuming there is only one camera in the scene
        }

        SDL_FPoint origin1 = {transform->posX + 40, transform->posY};
        SDL_FPoint origin2 = {transform->posX - 40, transform->posY};
        SDL_FPoint origin = {static_cast<float>(transform->posX), static_cast<float>(transform->posY)};

        SDL_FPoint direction = {0.0f, 1.0f}; // Down direction
        Ray ray(origin1, direction);
        Ray ray2(origin2, direction);
        Ray middleRay(origin, direction);

        // RayCastManager::renderRay(renderer, ray, *camera, 500.0f, {255, 255, 255, 255});
        // RayCastManager::renderRay(renderer, ray2, *camera, 500.0f, {255, 255, 255, 255});
        // RayCastManager::renderRay(renderer, middleRay, *camera, 170.0f, {255, 255, 255, 255});
    };
};

class CrusherLoader
{
public:
    static CrusherParameters fromJSON(const nlohmann::json &j, EntityManager &entityManager)
    {
        Transform *transform = entityManager.getComponent<Transform>(entityManager.getLast());
        if (!transform)
        {
            throw std::runtime_error("Crusher requires a Transform component");
        }

        Rigidbody *rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
        if (!rigidbody)
        {
            throw std::runtime_error("Crusher requires a Rigidbody component");
        }

        CrusherParameters params(
            transform,
            &entityManager,
            rigidbody);

        return params;
    }

    static Crusher createFromJSON(const nlohmann::json &j, EntityManager &entityManager)
    {
        auto params = fromJSON(j, entityManager);
        return Crusher(params);
    }
};
