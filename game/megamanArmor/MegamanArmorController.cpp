#include "MegamanArmorController.h"
#include "RaycastManager.h"

MegamanArmorController::MegamanArmorController(MegamanArmorControllerParameters params):
    transform(params.transform),
    entityManager(params.entityManager),
    animator(params.animator),
    rigidBody(params.rigidbody),
    health(params.health) {}

void MegamanArmorController::updateTimers(float deltaTime) {
    // Attack timer
    if (attackTimeLeft > 0) attackTimeLeft -= deltaTime;

    // Dashing timer
    if (dashingTimeLeft > 0) dashingTimeLeft -= deltaTime;

    if (jumpingTimeLeft > 0) jumpingTimeLeft -= deltaTime;

    if (timeAfterHitLeft > 0) timeAfterHitLeft -= deltaTime;
}
// -------------------------------------------
//  Managing KeyMaps
// -------------------------------------------
void MegamanArmorController::manageNotWalking(float deltaTime) {
    animator->setBool("isWalking", false);
}

void MegamanArmorController::manageMoveLeft(float deltaTime) {
    if (!animator->getBool("isActive")) return;
    
    if ((animator->getBool("isDashing") && animator->getBool("isGrounded"))
         || (jumpDashing && !animator->getBool("isGrounded"))) {
        transform->posX -= 500.0f * deltaTime;
    }
    else {
        transform->posX -= 120.0f * deltaTime;
    }
    
    animator->setBool("isWalking", true);
    animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
}

void MegamanArmorController::manageMoveRight(float deltaTime) {
    if (!animator->getBool("isActive")) return;

    if ((animator->getBool("isDashing") && animator->getBool("isGrounded"))
        || (jumpDashing && !animator->getBool("isGrounded"))) {
        transform->posX += 500.0f * deltaTime;
    }
    else {
        transform->posX += 120.0f * deltaTime;
    }

    animator->setBool("isWalking", true);
    animator->spriteRenderer->flip = SDL_FLIP_NONE;
}

void MegamanArmorController::manageAttack(float deltaTime) {
    if (!animator->getBool("isActive")) return;
    if (attackTimeLeft > 0) return;

    attackTimeLeft = attackDuration;
    animator->setBool("isAttacking", true);


}

void MegamanArmorController::manageJump(float deltaTime) {
    if (!animator->getBool("isActive")) return;

    if (animator->getBool("isDashing")) jumpDashing = true;

    if (animator->getBool("isGrounded")) {
        if (jumpingTimeLeft > 0.0f)
            return;
        animator->setBool("isGrounded", false);   
        rigidBody->velocityY -= 500.0;
        jumpingTimeLeft = 1.0f;
    }
}

void MegamanArmorController::manageExitArmor(float deltaTime) {
    if (!animator->getBool("isActive")) return;

    // Instanciar megaman
    if (!PrefabManager::getInstance().hasPrefab("megaman")) {
        PrefabManager::getInstance().loadPrefab("megaman");
    }   
    Entity megamanEntity = PrefabManager::getInstance().instantiatePrefab("megaman", *entityManager);
    Transform* megamanTransform = entityManager->getComponent<Transform>(megamanEntity);
    MegamanController* megamanController = entityManager->getComponent<MegamanController>(megamanEntity);
    if (megamanTransform) {
        // Al salir de la armadura salimos saltando
        megamanTransform->posX = this->transform->posX;
        megamanTransform->posY = this->transform->posY - 150;
        megamanController->start();
        megamanController->startFromArmor(deltaTime);
    }   
    animator->setBool("isPlayable", false);
    animator->setBool("isActive", false);
}

void MegamanArmorController::activate() {
    animator->setBool("isPlayable", true);
}

void MegamanArmorController::start() {
    DynamicInputManager::getInstance().loadFromJson("controls.config");
    collider = entityManager->getComponent<Collider>(parent);

    // Callbacks colisiones
    std::function<void(Collider*, CollisionResult)> callbackCollisionEnter = [this](Collider* other, CollisionResult cd){
        Vector2D resultVector = cd.normal;
        Debug::Log("- - - -  COLISION DESDE LA ARMADURA - - - - ");
        // Colisiones con el mapa
        /*
        if (other->tag == "Terrain") {
            if (abs(resultVector.x) != 1.0f ) {
                // Colision con suelo
                animator->setBool("isGrounded", true);
                animator->setBool("isFalling", false);
                rigidBody->velocityX = 0;
            }
        }*/

        if (other->tag == "Player") {
            Debug::Log("- - - - - MEGAMAN TOCANDO ARMADURA- - - - - ");
            Entity megamanEntity = other->parent;
            MegamanController* megaman = entityManager->getComponent<MegamanController>(megamanEntity);
            activate();
            megaman->removeMegaman();
        }

        if ((other->tag == "Enemy" || other->tag == "Penguin" || other->tag == "EnemyBullet")&&animator->getBool("isActive"))
        {
            // Solo actuar si no se ha acabado la invul.
            if (timeAfterHitLeft <= 0)
            {

                health->takeDamage(50);

                // Activar invul
                timeAfterHitLeft = timeAfterHit;
            }
        }
    };
    std::function<void(Collider*, CollisionResult)> callbackCollisionStay = [this](Collider* other, CollisionResult cd) {
        Vector2D resultVector = cd.normal;
        /*
        if (other->getTag() == "Terrain" ) {
            // Colision con terreno
            if (abs(resultVector.x) != 1.0f) {
                animator->setBool("isGrounded", true);
                animator->setBool("isFalling", false); 
            }
        }
            */
    };
    std::function<void(Collider*)> callbackCollisionExit = [this](Collider* other) {
        //animator->setBool("isGrounded", false);
    };
    collider->onCollisionEnter = callbackCollisionEnter;
    collider->onCollisionStay = callbackCollisionStay;
    collider->onCollisionExit = callbackCollisionExit;
    
    // Frame Triggers
    animator->addFrameTrigger("activation", 10, [this](){
        animator->setBool("isActive", true);
    });

    animator->addFrameTrigger("attack", 4, [this](){
        animator->setBool("isAttacking", false);
        SDL_FPoint megaman_left_point = {transform->posX - 40, transform->posY+20};
        SDL_FPoint megaman_right_point = {transform->posX + 40, transform->posY+20};
        SDL_FPoint right_direction = {1.0f, 0.0f};
        SDL_FPoint left_direction = {-1.0f, 0.0f};
        Ray ray_left(megaman_left_point, left_direction);
        Ray ray_right(megaman_right_point, right_direction);

        std::optional<RaycastHit> hit_left = RayCastManager::raycast(ray_left, entityManager, 100.0f, { "Enemy" });
        std::optional<RaycastHit> hit_right = RayCastManager::raycast(ray_right, entityManager, 100.0f, { "Enemy" });

        Debug::Log("MEGAMAN ARMOR IS ATTACKING");

        if(animator->spriteRenderer->flip == SDL_FLIP_HORIZONTAL) {
            Debug::Log("MEGAMAN ARMOR IS ATTACKING LEFT");
        }
        else if(animator->spriteRenderer->flip == SDL_FLIP_NONE) {
            Debug::Log("MEGAMAN ARMOR IS ATTACKING RIGHT");
        }

        // Gestion del landeo
        if (hit_left.has_value()/* && animator->spriteRenderer->flip == SDL_FLIP_HORIZONTAL*/) {
            Debug::Log("HIT LEFT");
            Debug::Log("HIT LEFT ENTITY: " + std::to_string(hit_left.value().entity));
            HealthComponent* enemyHealth = entityManager->getComponent<HealthComponent>(hit_left.value().entity);
            enemyHealth->takeDamage(500);
        }
        else if(hit_right.has_value()/* && animator->spriteRenderer->flip == SDL_FLIP_NONE*/){
            Debug::Log("HIT RIGHT");
            Debug::Log("HIT RIGHT ENTITY: " + std::to_string(hit_right.value().entity));
             HealthComponent* enemyHealth = entityManager->getComponent<HealthComponent>(hit_right.value().entity);
            enemyHealth->takeDamage(500);
        }else
        {
            Debug::Log("NO HITS");
        }
    });

    animator->addFrameTrigger("jumpAttack", 4, [this](){
        animator->setBool("isAttacking", false);
    });

    animator->addFrameTrigger("land", 0, [this](){
        animator->setBool("isActive", false);
    });
    animator->addFrameTrigger("land", 9, [this](){
        animator->setBool("isActive", true);
    });
}

void MegamanArmorController::update(float deltaTime) {

    Debug::Log("To kill: " + std::to_string(toKill));

    // Gestion timers
    updateTimers(deltaTime);

    // Gesiton Izquierda / Derecha / Parado
    if (DynamicInputManager::getInstance().isActionPressed("MoveLeft")) {
        manageMoveLeft(deltaTime);
    }
    else if (DynamicInputManager::getInstance().isActionPressed("MoveRight")) {
        manageMoveRight(deltaTime);
    }
    else {
        manageNotWalking(deltaTime);
    }

    // Gestion Dash
    if (DynamicInputManager::getInstance().isActionPressed("Dash")) {
        // Solo dasheamos durante periodo de tiempo acotado
        if (dashingTimeLeft > 0) {
            animator->setBool("isDashing", true);   
        }   
        else {
            animator->setBool("isDashing", false);
        }
    }
    else {
        dashingTimeLeft = dashingDuration;
        animator->setBool("isDashing", false);
    }

    // Gestion ataque
    if (DynamicInputManager::getInstance().isActionPressed("Shoot")){  
        manageAttack(deltaTime);
    }

    // Gesiton salto
    if (DynamicInputManager::getInstance().isActionPressed("Jump")){
        if (!jumpKeyHeld) {
            manageJump(deltaTime);
            jumpKeyHeld = true;
        } 
    }
    else {
        jumpKeyHeld = false;
    }

    // Gestion salir de la armadura
    if (DynamicInputManager::getInstance().isActionPressed("ExitArmor")) {
        manageExitArmor(deltaTime);
    }

    if(InputManager::isKeyPressed(SDL_SCANCODE_P)) {
        health->takeDamage(1000);
    }

    if(toKill) {

        std::shared_ptr<MegamanController> mc;

        Entity t = entityManager->findComponent<MegamanController>(mc);

        Transform* megamanTransform = entityManager->getComponent<Transform>(t);

        if (megamanTransform) {
            entityManager->deleteEntity(parent);
        }

    }

    if(health->isDead()&&animator->getBool("isActive")&&!toKill) {
        Debug::Log("MEGAMAN ARMOR MUERTE");
        manageExitArmor(deltaTime);
        toKill = true;
    }

    // Guarrada absoluta pero me la pela pq funciona
    if (animator->getBool("isGrounded")) {
        jumpDashing = false;
    }


    //Gestion con raycasts
    SDL_FPoint megaman_left_point = {transform->posX - 35, transform->posY};
    SDL_FPoint megaman_right_point = {transform->posX + 35, transform->posY};
    SDL_FPoint down_direction = {0.0f, 1.0f};
    Ray ray_down_left(megaman_left_point, down_direction);
    Ray ray_down_right(megaman_right_point, down_direction);

    std::optional<RaycastHit> hit_down_left = RayCastManager::raycast(ray_down_left, entityManager, 100.0f, { "Terrain" });
    std::optional<RaycastHit> hit_down_right = RayCastManager::raycast(ray_down_right, entityManager, 100.0f, { "Terrain" });

    // Gestion del landeo
    if (hit_down_left.has_value() || hit_down_right.has_value()) {
        // Solo comprobar si estas en el aire
        if (!animator->getBool("isGrounded") && jumpingTimeLeft <= 0) {
            // Colision con suelo
            animator->setBool("isGrounded", true);
            animator->setBool("isFalling", false);
            rigidBody->velocityX = 0;
        }
    }
    else {
        animator->setBool("isGrounded", false);
    }
}

void MegamanArmorController::render(EntityManager& entityManager,SDL_Renderer* renderer)
{
    Camera *camera = nullptr;
    for (Entity entity : entityManager.getEntities())
    {
        camera = entityManager.getComponent<Camera>(entity);
        if (camera)
            break; // Assuming there is only one camera in the scene
    }

    SDL_FPoint megaman_left_point = {transform->posX - 40, transform->posY};
    SDL_FPoint megaman_right_point = {transform->posX + 40, transform->posY};
    SDL_FPoint down_direction = {0.0f, 1.0f};
    Ray ray_down_left(megaman_left_point, down_direction);
    Ray ray_down_right(megaman_right_point, down_direction);

    RayCastManager::renderRay(renderer, ray_down_left, *camera, 100.0f, {255, 0, 0, 255});
    RayCastManager::renderRay(renderer, ray_down_right, *camera, 100.0f, {255, 0, 0, 255});

    SDL_FPoint megaman_left_point2 = {transform->posX - 40, transform->posY+20};
    SDL_FPoint megaman_right_point2 = {transform->posX + 40, transform->posY+20};
    SDL_FPoint right_direction = {1.0f, 0.0f};
    SDL_FPoint left_direction = {-1.0f, 0.0f};
    Ray r1(megaman_left_point2, left_direction);
    Ray r2(megaman_right_point2, right_direction);

    RayCastManager::renderRay(renderer, r1, *camera, 100.0f, {0, 255, 255, 255});
    RayCastManager::renderRay(renderer, r2, *camera, 100.0f, {0, 255, 255, 255});

    
}

MegamanArmorControllerParameters MegamanArmorControllerLoader::fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
    Transform* transform = entityManager.getComponent<Transform>(entityManager.getLast());
    if (!transform) {
        throw std::runtime_error("MegamanArmorController requires a Transform component");
    }

    AnimatorComponent* animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
    if (!animator) {
        throw std::runtime_error("MegamanArmorController requires a AnimatorComponent component");
    }

    Rigidbody* rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
    if (!rigidbody) {
        throw std::runtime_error("MegamanArmorController requires a Rigidbody component");
    }
    
    HealthComponent* health = entityManager.getComponent<HealthComponent>(entityManager.getLast());
    if (!health) {
        throw std::runtime_error("MegamanArmorController requires a Rigidbody component");
    }
    MegamanArmorControllerParameters params(
        transform,
        &entityManager,
        animator,
        rigidbody,
        health
    );

    return params;
}

MegamanArmorController MegamanArmorControllerLoader::createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
    return MegamanArmorController(fromJSON(j, entityManager));
}