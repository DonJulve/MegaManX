#include "Components.h"
#include "InputManager.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "effects/SmallExplosionComponent.h"
#include "IAComponents.h"
#include "RaycastManager.h"
#include <SDL2/SDL.h>

struct BattonBoneControllerParameters {

    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    Collider* collider;

    BattonBoneControllerParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb):
    transform(t), entityManager(em), animator(ac), rigidBody(rb) {}

};

struct BattonBoneController : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;

    ClassicIAComponent* iaComponent;
    std::shared_ptr<MegamanController> player;
    float agroDistance = 300.0f;
    bool haDadoGolpe = false; // Variable para saber si ha dado un golpe
    bool meHanGolpeado = false; // Variable para saber si le han dado un golpe y morir
    float speed = 90.0f; //velocidad con la que se acerca al jugador
    bool hayTecho = false; // Variable para saber si hay techo y posarme en el 
    bool meVoy = false; // Variable para saber si me estoy yendo cuando le doy un golpe
    float tiempoTranscurrido = 2.0f; // Tiempo que ha pasado desde el ultimo ataque
    float tiempoProxAtaque = 2.0f; // Tiempo que tiene que pasar para volver a atacar
    bool proximoAtaque = false; // Variable para saber si estoy preparado para atacar

    //Constructor del Controller
    BattonBoneController(BattonBoneControllerParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidBody(params.rigidBody){}

    //Funcion para la ia 
    void setup_ia() {

        iaComponent->setInitialState("Idle");

        iaComponent->registerAction("Idle", [this](EntityManager* entityManager, float deltaTime){
            //Debug::Log("Action Idle");
            animator->setBool("isFlying", false);
            hayTecho = false;
        });

        iaComponent->registerAction("Attack", [this](EntityManager* entityManager, float deltaTime){
            // perseguir al jugador
            animator->setBool("isFlying", true);
            Vector2D normal = {0.0,0.0};
            normal.x = player->getTransform()->posX - transform->posX;
            normal.y = player->getTransform()->posY - transform->posY;
            // normalizar
            float length = sqrt(normal.x * normal.x + normal.y * normal.y);
            if (length > 0) {
                normal.x /= length;
                normal.y /= length;
            }
            normal.x = normal.x * speed * deltaTime;
            normal.y = normal.y * speed * deltaTime;
            transform->posX += normal.x;
            transform->posY += normal.y;
        });

        iaComponent->registerAction("Bye", [this](EntityManager* entityManager, float deltaTime){
            //Debug::Log("Action Bye");
            animator->setBool("isFlying", true);
            rigidBody->velocityY = -300.0f;
            meVoy = true;
        });

        iaComponent->registerAction("Die", [this](EntityManager* entityManager, float deltaTime){
            death();
        });

        iaComponent->registerCondition("Idle", "Attack", [this](EntityManager* entityManager, float deltaTime){
            //Mirar a ver si el enemigo esta cerca
            Vector2D normal = {0.0,0.0};
            float distance;
            if(player){
                normal.x = abs(transform->posX - player->getTransform()->posX);
                normal.y = abs(transform->posY - player->getTransform()->posY);
                distance = sqrt(normal.x * normal.x + normal.y * normal.y);
                if ( distance <= agroDistance && tiempoTranscurrido >= tiempoProxAtaque){
                    tiempoTranscurrido = 0.0f;
                    proximoAtaque = false;
                    return true;
                }
            }

            return false;
        });

        iaComponent->registerCondition("Idle", "Die", [this](EntityManager* entityManager, float deltaTime){
            if(meHanGolpeado){
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Attack", "Bye", [this](EntityManager* entityManager, float deltaTime){
            if(haDadoGolpe){
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Attack", "Die", [this](EntityManager* entityManager, float deltaTime){
            if(meHanGolpeado){
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Bye", "Idle", [this](EntityManager* entityManager, float deltaTime){
            if(hayTecho){
                rigidBody->velocityY = 0.0f;
                return true;
            }
            return false;
        });


    }

    std::function<void(Collider*, CollisionResult)> callbackCollisionEnter = [this](Collider* other, CollisionResult cd) {
        if (other->tag == "Player"){
            haDadoGolpe = true;
        }
        if (other->tag == "megamanBullet") {
            meHanGolpeado = true;
        }
        if (other->tag == "megamanMediumBullet") {
            meHanGolpeado = true;
        }
        if (other->tag == "megamanStrongBullet") {
            meHanGolpeado = true;
        }
    };


    void death(){
        if (!PrefabManager::getInstance().hasPrefab("smallExplosion")) {
            PrefabManager::getInstance().loadPrefab("smallExplosion");
        }
        Entity explosionEntity = PrefabManager::getInstance().instantiatePrefab("smallExplosion", *entityManager);
        Transform* explosionTransform = entityManager->getComponent<Transform>(explosionEntity);
        SmallExplosionComponent* explosionComponent = entityManager->getComponent<SmallExplosionComponent>(explosionEntity);
        if(explosionTransform){
            explosionTransform->posX = transform->posX;
            explosionTransform->posY = transform->posY;
            explosionComponent->start();
        }

        //Scamos drop
        if (!PrefabManager::getInstance().hasPrefab("drop")) {
            PrefabManager::getInstance().loadPrefab("drop");
        }
        Entity dropEntity = PrefabManager::getInstance().instantiatePrefab("drop", *entityManager);
        Transform* dropTransform = entityManager->getComponent<Transform>(dropEntity);
        DropController* dropController = entityManager->getComponent<DropController>(dropEntity);
        if(dropTransform){
            dropTransform->posX = transform->posX;
            dropTransform->posY = transform->posY;
        }
        dropController->start();

        entityManager->deleteEntity(parent);
    }
    
    void findPlayer()
    {
        if(!player) entityManager->findComponent<MegamanController>(player);
    }

    //Funcion inicio
    void start() override {

        iaComponent = entityManager->getComponent<ClassicIAComponent>(parent);
        Collider* collider = entityManager->getComponent<Collider>(parent);

        collider->onCollisionEnter = callbackCollisionEnter;

        setup_ia();
    }

    //Funcion de actualizacion
    void update(float deltaTime) override {

        findPlayer();
        if (meVoy){
            SDL_FPoint origin = { static_cast<float>(transform->posX), static_cast<float>(transform->posY) };
            SDL_FPoint direction = SDL_FPoint{ 0.0f, -1.0f };
    
            Ray ray(origin, direction);
            Debug::Log("Ray origin: " + std::to_string(ray.origin.x) + ", " + std::to_string(ray.origin.y));
            Debug::Log("Centro de la entidad: " + std::to_string(transform->posX) + ", " + std::to_string(transform->posY));
    
            // Realizar el raycast
            std::optional<RaycastHit> hit = RayCastManager::raycast(ray, entityManager, 23.0f, { "Terrain" });
            Debug::Log("Ray hit: " + std::to_string(hit.has_value()));
    
            if(hit.has_value()){
                Debug::Log("Hay techo");
                hayTecho = true;
                meVoy = false;
                haDadoGolpe = false;
                proximoAtaque = true;
            }
            
        }

        if(proximoAtaque){
            tiempoTranscurrido += deltaTime;
        } 
    
    }

};

class BattonBoneControllerLoader {
    public:

        static BattonBoneControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("BattonBoneController requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("BattonBoneController requires a AnimatorComponent component");
            }

            Rigidbody* rigidBody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidBody) {
                throw std::runtime_error("BattonBoneController requires a Rigidbody component");
            }

            BattonBoneControllerParameters params(
                transform,
                &entityManager,
                animator,
                rigidBody
            );

            return params;
        }

        static BattonBoneController createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return BattonBoneController(fromJSON(j, entityManager));
        }
};
