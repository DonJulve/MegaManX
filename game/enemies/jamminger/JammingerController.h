#include "Components.h"
#include "InputManager.h"
#include "Debug.h"
#include "AnimatorComponent.h"
#include "effects/SmallExplosionComponent.h"

struct JammingerControllerParameters {

    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    HealthComponent* health;

    JammingerControllerParameters(Transform* t, EntityManager* em, AnimatorComponent* ac, Rigidbody* rb, HealthComponent* hc):
    transform(t), entityManager(em), animator(ac), rigidBody(rb), health(hc) {}

};

struct JammingerController : public Component {
    Transform* transform;
    EntityManager* entityManager;
    AnimatorComponent* animator;
    Rigidbody* rigidBody;
    ClassicIAComponent* iaComponent;
    HealthComponent* healthComponent;


    JammingerController(JammingerControllerParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidBody(params.rigidBody),
    healthComponent(params.health){}

    // Referencia al jugador
    std::shared_ptr<MegamanController> player;
    bool isAttacking = false;
    bool isBye = false;
    bool isLaugh = false;
    bool haDadoGolpe = false; // Variable para saber si ha dado un golpe
    int numGolpes = 0; // Numero de golpes que le han dado
    float timeAttack = 0.0f;
    float maxTimeAttack = 7.0f;
    float timeLaugh = 0.0f;
    float maxTimeLaugh = 1.5f;
    float up = 70.0f;
    float maxUp = 100.0f;
    float countUp = 0.0f;
    float speed = 70.0f; //velocidad con la que se acerca al jugador
    float activationDistance = 600.0f; // Distancia a la que se activa el enemigo

    // Funcion IA
    void setup_ia(){

        //Estado inicial
        iaComponent->setInitialState("Idle");

        iaComponent->registerAction("Idle", [this](EntityManager* entityManager, float deltaTime){
            rigidBody->velocityX = 0.0f;
            rigidBody->velocityY = 0.0f;
        });

        //Registrar de estados
        iaComponent->registerAction("Attack", [this](EntityManager* entityManager, float deltaTime){
            animator->setBool("isLaugh", false);
            isAttacking = true;
            if(player){
                Vector2D normal = {0.0, 0.0};

                // Calcular dirección hacia el jugador
                normal.x = player->getTransform()->posX - transform->posX;
                normal.y = player->getTransform()->posY - transform->posY;

                // Normalizar la dirección
                float length = sqrt(normal.x * normal.x + normal.y * normal.y);
                if (length > 0) {
                    normal.x /= length;
                    normal.y /= length;
                }

                // Aplicar zigzag en X usando la normal como base
                float zigzagAmplitude = 3.0f;  // Magnitud del zigzag
                float zigzagSpeed = 10.0f;     // Controla la frecuencia del zigzag
                float zigzagOffset = sin(SDL_GetTicks() * 0.01f * zigzagSpeed) * zigzagAmplitude; 

                // Se mueve hacia el jugador pero con oscilación
                transform->posX += normal.x * speed * deltaTime;
                transform->posY += normal.y  * speed * deltaTime;

            }

            // si lleva mucho tiempo atacando se va
            if(timeAttack >= maxTimeAttack){
                timeAttack = 0.0f;
                isAttacking = false;
                isBye = true;
            }

        });

        iaComponent->registerAction("Fly", [this](EntityManager* entityManager, float deltaTime){
            // Subo un poco
            transform->posY -= up * deltaTime;
            //Actualizo cuanto he subido y si he llegado al maximo me rio
            countUp += up * deltaTime;
            if(countUp >= maxUp){
                countUp = 0.0f;
                isLaugh = true;
            }
        });

        iaComponent->registerAction("Laugh", [this](EntityManager* entityManager, float deltaTime){
            // pongo animacion de reir
            animator->setBool("isLaugh", true);

            // si lleva mucho tiempo riendo vuelve a atacar
            if(timeLaugh >= maxTimeLaugh){
                timeLaugh = 0.0f;
                isLaugh = false;
                isAttacking = true;
            }
        });


        iaComponent->registerAction("Bye", [this](EntityManager* entityManager, float deltaTime){
            // subo infinito arriba
            rigidBody->velocityY = -100.0f;
        });

        iaComponent->registerAction("Die", [this](EntityManager* entityManager, float deltaTime){
            // me muero(exploto)
            death();
        });

        iaComponent->registerCondition("Idle", "Attack", [this](EntityManager* entityManager, float deltaTime){
            //Debug::Log("Condition Idle");
            //Mirar a ver si el enemigo esta cerca
            Vector2D normal = {0.0, 0.0};
            float distance;
            if(player){
                normal.x = abs(transform->posX - player->getTransform()->posX);
                normal.y = abs(transform->posY - player->getTransform()->posY);
                distance = sqrt(normal.x * normal.x + normal.y * normal.y);
                if ( distance <= activationDistance){
                    return true;
                }
            }

            return false;
        });

        iaComponent->registerCondition("Attack", "Fly", [this](EntityManager* entityManager, float deltaTime){
            if(haDadoGolpe){
                numGolpes++;
                haDadoGolpe = false;
                isAttacking = false;
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Attack", "Bye", [this](EntityManager* entityManager, float deltaTime){
            if(isBye || numGolpes >= 2){
                isBye = false;
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Attack", "Die", [this](EntityManager* entityManager, float deltaTime){
            if(healthComponent->isDead()){
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Fly", "Laugh", [this](EntityManager* entityManager, float deltaTime){
            if(isLaugh){
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Fly", "Die", [this](EntityManager* entityManager, float deltaTime){
            if(healthComponent->isDead()){
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Laugh", "Attack", [this](EntityManager* entityManager, float deltaTime){
            if(isAttacking){
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Laugh", "Die", [this](EntityManager* entityManager, float deltaTime){
            if(healthComponent->isDead()){
                return true;
            }
            return false;
        });

        iaComponent->registerCondition("Bye", "Die", [this](EntityManager* entityManager, float deltaTime){
            if(healthComponent->isDead()){
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
            healthComponent->takeDamage(35);
        }
        if (other->tag == "megamanMediumBullet") {
            healthComponent->takeDamage(50);
        }
        if (other->tag == "megamanStrongBullet") {
            healthComponent->takeDamage(100);
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

    void findPlayer(){
        if(!player) entityManager->findComponent<MegamanController>(player);
    }

    //Funcion inicio
    void start() override {
        Collider* collider = entityManager->getComponent<Collider>(parent);
        iaComponent = entityManager->getComponent<ClassicIAComponent>(parent);
        collider->onCollisionEnter = callbackCollisionEnter;

        setup_ia();

    }

    //Funcion de actualizacion
    void update(float deltaTime) override {

        // Buscar al jugador
        findPlayer();

        if(isAttacking){
            timeAttack += deltaTime;
        }

        if(isLaugh){
            timeLaugh += deltaTime;
        }

    }

};

class JammingerControllerLoader {
    public:

        static JammingerControllerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
            if (!transform) {
                throw std::runtime_error("JammingerController requires a Transform component");
            }

            AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
            if (!animator) {
                throw std::runtime_error("JammingerController requires a AnimatorComponent component");
            }

            Rigidbody* rigidBody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
            if (!rigidBody) {
                throw std::runtime_error("JammingerController requires a Rigidbody component");
            }

            HealthComponent* health = entityManager.getComponent<HealthComponent>(entityManager.getLast());
            if (!health) {
                throw std::runtime_error("JammingerController requires a HealthComponent component");
            }
            JammingerControllerParameters params(
                transform,
                &entityManager,
                animator,
                rigidBody,
                health
            );

            return params;
        }

        static JammingerController createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
            return JammingerController(fromJSON(j, entityManager));
        }
};
