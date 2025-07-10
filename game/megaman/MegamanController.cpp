#include "MegamanController.h"
#include "../menus/PauseMenuComponent.h"
#include "../MegamanLevelsManager.h"
#include "RaycastManager.h"

MegamanController *MegamanController::instance = nullptr;
std::vector<std::function<void()>> MegamanController::deletionCallbacks;

// Normalize a 2D vector
Vector2D MegamanController::normalize(const Vector2D &v)
{
    float length = std::sqrt(v.x * v.x + v.y * v.y);
    return {v.x / length, v.y / length};
}

Vector2D MegamanController::getTangent(const Vector2D &normal)
{
    Vector2D tangent = {-normal.y, normal.x}; // 90-degree rotation

    // Ensure tangent points mostly to the right (positive x)
    if (tangent.x < 0.0f)
    {
        tangent.x = -tangent.x;
        tangent.y = -tangent.y;
    }

    return tangent;
}

MegamanController::~MegamanController()
{
    destroyInstance();
}

MegamanController *MegamanController::getInstance(std::function<void()> callback)
{
    if (callback && instance)
    {
        deletionCallbacks.push_back(callback);
    }
    return instance;
}

MegamanController *MegamanController::getInstance()
{
    return instance;
}

void MegamanController::destroyInstance()
{
    instance = nullptr;
    for (const auto &callback : deletionCallbacks)
    {
        if (callback)
        {
            callback(); // Notify all callbacks
        }
    }
    deletionCallbacks.clear(); // Clear callbacks after deletion
}

MegamanController::MegamanController(MegamanControllerParameters params)
    : transform(params.transform),
      entityManager(params.entityManager),
      animator(params.animator),
      rigidBody(params.rigidBody),
      health(params.health) { instance = this; }

void MegamanController::updateTimers(float deltaTime)
{
    // Debug::Log("MegamanController::updateTimers");
    // Shooting timer
    if (shootingTimeLeft > 0)
    {
        shootingTimeLeft -= deltaTime;
    }
    else
    {
        animator->setBool("isShooting", false);
    }

    // Walljump timer
    if (wallJumpTimeLeft > 0)
    {
        wallJumpTimeLeft -= deltaTime;
    }
    else
    {
        rigidBody->velocityX = 0;
        animator->setBool("isActive", true);
    }

    // Invul after being hit
    if (timeAfterHitLeft > 0)
        timeAfterHitLeft -= deltaTime;

    // Jumping timer
    if (jumpingTimeLeft > 0)
        jumpingTimeLeft -= deltaTime;

    // Dashing timer
    if (dashingTimeLeft > 0)
        dashingTimeLeft -= deltaTime;

    // Charging shoot
    if (shootKeyHeld)
    {
        // Debug::Log("CARGANDO CARGA FUERA");
        timeChargingShoot += deltaTime;
        if (timeChargingShoot > timeMediumBullet && timeChargingShoot < timeStrongBullet)
        {
            // NUEVO
            // Debug::Log("CARGANDO CARGA");
            charge->aparecer();
        }
        else if (timeChargingShoot > timeStrongBullet)
        {
            charge->start();
            charge->cambiarCarga();
        }
    }

    // Debug::Log("FIN");
}

// -------------------------------------------
//  Managing Keymaps
// -------------------------------------------

void MegamanController::manageNotWalking(float deltaTime)
{
    animator->setBool("isMoving", false);
}

void MegamanController::manageMoveLeft(float deltaTime)
{
    if (!animator->getBool("isActive"))
        return;

    if (animator->getBool("isDashing") && animator->getBool("isGrounded"))
    {
        transform->posX -= 500.0f * movementDirection.x * deltaTime;
        transform->posY -= 500.0f * movementDirection.y * deltaTime;
    }
    else
    {
        transform->posX -= 200.0f * movementDirection.x * deltaTime;
        transform->posY -= 200.0f * movementDirection.y * deltaTime;
    }

    animator->setBool("isMoving", true);
    animator->spriteRenderer->flip = SDL_FLIP_HORIZONTAL;
}

void MegamanController::manageMoveRight(float deltaTime)
{
    if (!animator->getBool("isActive"))
        return;

    if (animator->getBool("isDashing") && animator->getBool("isGrounded"))
    {
        transform->posX += 500.0f * movementDirection.x * deltaTime;
        transform->posY += 500.0f * movementDirection.y * deltaTime;
    }
    else
    {
        transform->posX += 200.0f * movementDirection.x * deltaTime;
        transform->posY += 200.0f * movementDirection.y * deltaTime;
    }

    animator->setBool("isMoving", true);
    animator->spriteRenderer->flip = SDL_FLIP_NONE;
}

void MegamanController::manageJump(float deltaTime)
{
    if (!animator->getBool("isActive"))
        return;

    if (animator->getBool("isGrounded"))
    {
        if (jumpingTimeLeft > 0.0f)
            return;
        animator->setBool("isGrounded", false);
        Debug::Log("Jumping");
        rigidBody->velocityY -= 500.0;
        jumpingTimeLeft = jumpingDuration;
        audioManager->playSound("jump");

        movementDirection = {1.0f, 0.0f};
    }
    // TODO WALLJUMP CAMBIAR PARA DASH EN WALLJUMP
    else if (animator->getBool("touchingWall"))
    {
        if (wallJumpTimeLeft > 0.0f)
            return;
        animator->setBool("isWallJumping", true);
        animator->setBool("isActive", false);
        wallJumpTimeLeft = wallJumpDuration;
        rigidBody->velocityY -= 500.0;

        if (animator->spriteRenderer->flip != SDL_FLIP_NONE)
        {
            rigidBody->velocityX += 200;
        }
        else
        {
            rigidBody->velocityX -= 200;
        }

        audioManager->playSound("jump");

        movementDirection = {1.0f, 0.0f}; // Reset movement direction after jump
    }
}

void MegamanController::manageShoot(float deltaTime)
{
    if (shootingTimeLeft > 0)
        return;

    // Debug::Log("DISPARANDO");
    shootingTimeLeft = shootingDuration;
    animator->setBool("isShooting", true);
    shootBullet();
    audioManager->playSound("shoot");
    if (chargingSoundPlaying)
    {
        Debug::Log("- - - - Desctivando carga - - - ");
        chargingSoundPlaying = false;
    }
}

// -------------------------------------------
//  Managing Bullets
// -------------------------------------------

void MegamanController::shootBullet()
{
    // Debug::Log("MegamanController::shootBullet");
    Debug::Log("MegamanController::shootBullet");
    // Detener el sonido de carga si estaba reproduciéndose
    if (chargingSoundPlaying) {
        Debug::Log("- - - - Desactivando carga al disparar - - - ");
        chargingSoundPlaying = false;
        Mix_HaltChannel(chargingSoundChannel);
    }
    if (timeChargingShoot < timeMediumBullet)
    {
        initializeWeakBullet();
    }
    else if (timeChargingShoot < timeStrongBullet)
    {
        initializeMediumBullet();
    }
    else
    {
        initializeStrongBullet();
    }
    // NUEVO
    if (charge)
    {
        charge->desaparecer();
    }
}

void MegamanController::initializeWeakBullet()
{
    if (!PrefabManager::getInstance().hasPrefab("megamanBullet"))
    {
        PrefabManager::getInstance().loadPrefab("megamanBullet");
    }
    Entity bulletEntity = PrefabManager::getInstance().instantiatePrefab("megamanBullet", *entityManager);
    Transform *bulletTransform = entityManager->getComponent<Transform>(bulletEntity);
    MegamanBulletComponent *bulletComponent = entityManager->getComponent<MegamanBulletComponent>(bulletEntity);
    if (bulletTransform)
    {
        bulletTransform->posX = transform->posX + 60;
        bulletComponent->movingRight = true;
        bulletTransform->posY = transform->posY;


        // Solo giramos la bala si estamos giraos o si estamos tocando una pared sin girar
        if (animator->spriteRenderer->flip == SDL_FLIP_HORIZONTAL && !animator->getBool("touchingWall")) {
            bulletTransform->posX = this->transform->posX - 60;
            bulletComponent->movingRight = false;
        }
        else if (animator->spriteRenderer->flip == SDL_FLIP_NONE && animator->getBool("touchingWall"))
        {
            bulletTransform->posX = this->transform->posX - 60;
            bulletComponent->movingRight = false;
        }

        bulletTransform->posY = this->transform->posY - 4;
        bulletComponent->start();
    }
}

void MegamanController::initializeMediumBullet()
{
    if (!PrefabManager::getInstance().hasPrefab("megamanMediumBullet"))
    {
        PrefabManager::getInstance().loadPrefab("megamanMediumBullet");
    }
    Entity bulletEntity = PrefabManager::getInstance().instantiatePrefab("megamanMediumBullet", *entityManager);
    Transform *bulletTransform = entityManager->getComponent<Transform>(bulletEntity);
    MegamanMediumBulletComponent *bulletComponent = entityManager->getComponent<MegamanMediumBulletComponent>(bulletEntity);
    if (bulletTransform)
    {
        bulletTransform->posX = transform->posX + 60;
        bulletComponent->movingRight = true;
        bulletTransform->posY = transform->posY;


        // Solo giramos la bala si estamos giraos o si estamos tocando una pared sin girar
        if (animator->spriteRenderer->flip == SDL_FLIP_HORIZONTAL && !animator->getBool("touchingWall")) {
            bulletTransform->posX = this->transform->posX - 60;
            bulletComponent->movingRight = false;
        }
        else if (animator->spriteRenderer->flip == SDL_FLIP_NONE && animator->getBool("touchingWall"))
        {
            bulletTransform->posX = this->transform->posX - 60;
            bulletComponent->movingRight = false;
        }
        
        bulletTransform->posY = this->transform->posY - 4;
        bulletComponent->start();
    }
}

void MegamanController::initializeStrongBullet()
{
    if (!PrefabManager::getInstance().hasPrefab("megamanStrongBullet"))
    {
        PrefabManager::getInstance().loadPrefab("megamanStrongBullet");
    }
    Entity bulletEntity = PrefabManager::getInstance().instantiatePrefab("megamanStrongBullet", *entityManager);
    Transform *bulletTransform = entityManager->getComponent<Transform>(bulletEntity);
    MegamanStrongBulletComponent *bulletComponent = entityManager->getComponent<MegamanStrongBulletComponent>(bulletEntity);
    if (bulletTransform)
    {
        bulletTransform->posX = transform->posX + 60;
        bulletComponent->movingRight = true;
        bulletTransform->posY = transform->posY;


        // Solo giramos la bala si estamos giraos o si estamos tocando una pared sin girar
        if (animator->spriteRenderer->flip == SDL_FLIP_HORIZONTAL && !animator->getBool("touchingWall")) {
            bulletTransform->posX = this->transform->posX - 60;
            bulletComponent->movingRight = false;
        }
        else if (animator->spriteRenderer->flip == SDL_FLIP_NONE && animator->getBool("touchingWall"))
        {
            bulletTransform->posX = this->transform->posX - 60;
            bulletComponent->movingRight = false;
        }
        
        bulletTransform->posY = this->transform->posY - 4;
        bulletComponent->start();
    }
}

// -------------------------------------------
//  Utils
// -------------------------------------------

void MegamanController::loadSoundEffects()
{
    audioManager = AudioManager::getInstance();

    // Cargar pistas
    audioManager->loadSound("death", "sound_effects/megaman/Die.wav");
    audioManager->loadSound("land", "sound_effects/megaman/Land.wav");
    audioManager->loadSound("jump", "sound_effects/megaman/Jump.wav");
    audioManager->loadSound("spawn", "sound_effects/megaman/Fade In.wav");
    audioManager->loadSound("dash", "sound_effects/megaman/Dash.wav");
    audioManager->loadSound("hurt", "sound_effects/megaman/Hurt.wav");
    Debug::Log("- - - - - CARGANDO SONDIDO CARGA BALA - - - - -");
    audioManager->loadSound("charge", "sound_effects/megaman/Charge.wav");
    audioManager->loadSound("charge_loop", "sound_effects/megaman/Charge_loop.wav");

    // TODO CHANGE THIS SOUND
    audioManager->loadSound("shoot", "sound_effects/95 - MMX - Misc. dash, jump, move (7).wav");

    // Asignarlas a sus movimientos
    animator->addFrameTrigger("jump", 0, [this]()
                              { audioManager->playSound("jump"); });

    animator->addFrameTrigger("dash", 1, [this]()
                              { audioManager->playSound("dash"); });

    animator->addFrameTrigger("damage", 0, [this]()
                              { audioManager->playSound("hurt"); });
}

float MegamanController::checkHealth()
{
    if (health)
    {
        return health->getCurrentHealth();
    }
    return 0.0f;
}


void MegamanController::removeMegaman() {
    entityManager->deleteEntity(parent);
}

Transform *MegamanController::getTransform()
{
    return transform;
}

AnimatorComponent *MegamanController::getAnimator()
{
    return animator;
}

void MegamanController::addHealth(float health)
{
    if (this->health)
    {
        this->health->heal(health);
    }
}

void MegamanController::startFromArmor(float deltaTime)
{
    animator->setBool("isActive", true);
    animator->setBool("isGrounded", true);
    jumpingTimeLeft = 0;
    manageJump(deltaTime);
}

void MegamanController::start()
{
    DynamicInputManager::getInstance().loadFromJson("controls.config");

    if (!charge)
    {
        // Debug::Log("CREANDO CARGA dentro del if");
        if (!PrefabManager::getInstance().hasPrefab("megamanCharge"))
        {
            PrefabManager::getInstance().loadPrefab("megamanCharge");
        }
        Entity chargeEntity = PrefabManager::getInstance().instantiatePrefab("megamanCharge", *entityManager);
        charge = entityManager->getComponent<MegamanChargeComponent>(chargeEntity);
        charge->desaparecer();
    }

    Collider *collider = entityManager->getComponent<Collider>(parent);

    if (collider)
    {
        // Tocar collider
        std::function<void(Collider *, CollisionResult)> callbackCollisionEnter = [this](Collider *other, CollisionResult cd)
        {
            Vector2D resultVector = cd.normal;

            movementDirection = getTangent(normalize(resultVector));

            if (abs(movementDirection.y) >= 0.98f)
            {
                movementDirection = {1.0f, 0.0f};
            }

            Debug::Log("- - - - - MEGAMAN COLISION- - - - - ");
            // Colisiones con mapa
            if (other->tag == "Terrain")
            {
                if (abs(resultVector.x) >= 0.98f)
                {
                    // Colision con pared
                    animator->setBool("touchingWall", true);
                    //rigidBody->velocityY = 0.0f;
                }
                else
                {
                    // Colision con suelo
                    animator->setBool("isGrounded", true);
                    animator->setBool("isFalling", false);
                    rigidBody->velocityX = 0;
                    audioManager->playSound("land");
                }
            }

            // Colision con la armadura
            if (other->tag == "MegamanArmor")
            {
                Debug::Log("- - - - - MEGAMAN TOCANDO ARMADURA- - - - - ");
                Entity armorEntity = other->parent;
                MegamanArmorController *armor = entityManager->getComponent<MegamanArmorController>(armorEntity);
                armor->activate();
                removeMegaman();
            }

            // Player hit
            if (other->tag == "Enemy" || other->tag == "Penguin" || other->tag == "EnemyBullet")
            {
                // Solo actuar si no se ha acabado la invul.
                if (timeAfterHitLeft <= 0)
                {
                    // Desactivar propiedades por hit
                    animator->setBool("isDashing", false);
                    animator->setBool("touchingWall", false);
                    animator->setBool("isWallJumping", false);
                    animator->setBool("isActive", false);

                    health->takeDamage(50);

                    // Activar invul
                    timeAfterHitLeft = timeAfterHit;

                    if (health->isDead())
                    {
                        animator->setBool("isHit", true);
                        animator->setBool("isDead", true);
                    }
                    else
                    {
                        animator->setBool("isHit", true);
                        // TODO CAMBIAR HACERLO CON LOS ANGULOS DE COLISION
                        // Retrocedar un poco cuando te dan
                        if (animator->spriteRenderer->flip == SDL_FLIP_NONE)
                        {
                            transform->posX -= 20;
                            transform->posY -= 20;
                        }
                        else
                        {
                            transform->posX += 20;
                            transform->posY -= 20;
                        }
                    }
                }
            }
        };

        std::function<void(Collider *, CollisionResult)> callbackCollisionStay = [this](Collider *other, CollisionResult cd)
        {
            Vector2D resultVector = cd.normal;
            // Seguimos chocando con terreno
            if (other->getTag() == "Terrain")
            {
                // Colision con pared
                if (abs(resultVector.x) == 1.0f)
                {
                }
                // Colision con terreno
                else
                {
                    //animator->setBool("isGrounded", true);
                    //animator->setBool("isFalling", false);
                }
            }
            if (other->tag == "MegamanArmor")
            {
                Debug::Log("- - - - - MEGAMAN TOCANDO ARMADURA- - - - - ");
            }
        };

        // Nos caemos de una plataforma
        std::function<void(Collider *)> callbackCollisionExit = [this](Collider *other)
        {
            // Colision con terreno
            if (other->getTag() == "Terrain")
            {
                if (animator->getBool("touchingWall"))
                {
                    animator->setBool("touchingWall", false);
                    animator->setBool("isWallJumping", false);
                }
                else
                {
                    animator->setBool("isGrounded", false);
                    if (rigidBody->velocityY >= 0.0)
                    {
                        animator->setBool("isFalling", true);
                    }
                }
            }
        };
        collider->onCollisionEnter = callbackCollisionEnter;
        collider->onCollisionStay = callbackCollisionStay;
        collider->onCollisionExit = callbackCollisionExit;

        animator->addFrameTrigger("idle", 0, [this]()
                                  { animator->setBool("isShooting", false); });

        animator->addFrameTrigger("spawn", 7, [this]()
                                  { animator->setBool("isActive", true); });

        animator->addFrameTrigger("dead", 1, [this]()
                                  { audioManager->playSound("death"); });

        // ---------------------
        //  Stop shooting callbacks
        // ---------------------

        animator->addFrameTrigger("idleShooting", 0, [this]()
                                  { animator->setBool("isShooting", false); });
        animator->addFrameTrigger("walkShooting", 0, [this]()
                                  { animator->setBool("isShooting", false); });
        animator->addFrameTrigger("dashShooting", 0, [this]()
                                  { animator->setBool("isShooting", false); });
        animator->addFrameTrigger("jumpShooting", 0, [this]()
                                  { animator->setBool("isShooting", false); });
        animator->addFrameTrigger("fallShooting", 0, [this]()
                                  { animator->setBool("isShooting", false); });
        animator->addFrameTrigger("wallJumpShooting", 0, [this]()
                                  { animator->setBool("isWallJumping", false); });
        animator->addFrameTrigger("wallJump", 0, [this]()
                                  { animator->setBool("isWallJumping", false); });
        animator->addFrameTrigger("damage", 1, [this]()
                                  {
            animator->setBool("isHit", false);
            animator->setBool("isActive", true); });
        animator->addFrameTrigger("dead", 7, [this]()
                                  { removeMegaman(); });
    }

    loadSoundEffects();
    audioManager->playSound("spawn");
}

void MegamanController::update(float deltaTime)
{

    bool wallTouching = animator->getBool("touchingWall");
    bool grounded = animator->getBool("isGrounded");
    bool falling = animator->getBool("isFalling");
    bool shooting = animator->getBool("isShooting");

    // NUEVO
    if (charge)
    {
        charge->mover(transform->posX, transform->posY);
    }

    updateTimers(deltaTime);

    if (wallTouching)
    {
        rigidBody->gravity = 9.8;
    }
    else
    {
        rigidBody->gravity = 9.8;
    }

    // -------------------------------------------
    //  Movement Controller
    // -------------------------------------------
    if (DynamicInputManager::getInstance().isActionPressed("MoveLeft"))
    {
        manageMoveLeft(deltaTime);
    }
    else if (DynamicInputManager::getInstance().isActionPressed("MoveRight"))
    {
        manageMoveRight(deltaTime);
    }
    else
    {
        manageNotWalking(deltaTime);
    }

    // Gestion Dash
    if (DynamicInputManager::getInstance().isActionPressed("Dash"))
    {
        // Solo dasheamos durante periodo de tiempo acotado
        if (dashingTimeLeft > 0)
        {
            animator->setBool("isDashing", true);
        }
        else
        {
            animator->setBool("isDashing", false);
        }
    }
    else
    {
        dashingTimeLeft = dashingDuration;
        animator->setBool("isDashing", false);
    }

    // Gestion Saltar
    if (DynamicInputManager::getInstance().isActionPressed("Jump"))
    {
        if (!jumpKeyHeld)
        {
            manageJump(deltaTime);
            jumpKeyHeld = true;
        }
    }
    else
    {
        jumpKeyHeld = false;
    }

    // Gestion disparo
    // Tecla presionada
    // TODO SONIDO DE CARGA DE LA BALA
    if (DynamicInputManager::getInstance().isActionPressed("Shoot"))
    {
        if (!chargingSoundPlaying)
        {
            Debug::Log("- - - - Activando carga - - - ");
            chargingSoundPlaying = true;
            chargingSoundChannel = audioManager->playSound("charge", 0);
        }

        if (chargingSoundPlaying && !Mix_Playing(chargingSoundChannel)) {
            chargingSoundChannel = audioManager->playSound("charge_loop", -1);
        }

        if (!shootKeyHeld)
        {
            manageShoot(deltaTime);
        }

        shootKeyHeld = true;
    }
    // Tecla no presionada
    else
    {
        // Se ha soltado la tecla en este frame
        if (shootKeyHeld)
        {
            if (timeChargingShoot > 0.3)
            {
                manageShoot(deltaTime);
                timeChargingShoot = 0;
            }
        }
        shootKeyHeld = false;
        // Detener el sonido de carga si estaba reproduciéndose
        if (chargingSoundPlaying) {
            Debug::Log("- - - - Desactivando carga - - - ");
            chargingSoundPlaying = false;
            // Detener el sonido en el canal específico
            Mix_HaltChannel(chargingSoundChannel);
        }
    }

    if (InputManager::isKeyPressed(SDL_SCANCODE_ESCAPE) && pauseEnabled)
    {
        Scene *levelScene = entityManager->getSceneManager()->getScene(entityManager->scenaName);
        if (levelScene)
        {
            levelScene->deactivate();
        }

        Scene *pauseScene = entityManager->getSceneManager()->getScene("pauseMenu");
        if (pauseScene)
        {
            std::shared_ptr<PauseMenuComponent> pauseMenu;
            Entity pauseMenuEntity = pauseScene->getEntityManager().findComponent<PauseMenuComponent>(pauseMenu);

            if (pauseMenu)
            {
                pauseMenu->inputEnabled = false;
                pauseMenu->levelName = entityManager->scenaName;

                Debug::Log("Pausa: " + pauseMenu->levelName);
            }

            pauseScene->activate();
            pauseEnabled = false;
        }
    }

    if (!InputManager::isKeyPressed(SDL_SCANCODE_ESCAPE))
    {
        pauseEnabled = true;
    }

    if (health->isDead())
    {
        MegamanLevelsManager::getInstance()->loadLevel("levelSelectorMenu", entityManager->getSceneManager()->getSceneNameByEntityManager(entityManager), 255.0f / 1.5f);
        audioManager->stopMusic();
        animator->setBool("isHit", true);
        animator->setBool("isDead", true);
    }

    // -------------------------------------------
    //  MISCELANEOUS
    // -------------------------------------------

    // Gestion con raycasts 

    SDL_FPoint megaman_left_point = {transform->posX - 35, transform->posY};
    SDL_FPoint megaman_right_point = {transform->posX + 35, transform->posY};
    SDL_FPoint down_direction = {0.0f, 1.0f};
    Ray ray_down_left(megaman_left_point, down_direction);
    Ray ray_down_right(megaman_right_point, down_direction);

    std::optional<RaycastHit> hit_down_left = RayCastManager::raycast(ray_down_left, entityManager, 45.0f, { "Terrain" });
    std::optional<RaycastHit> hit_down_right = RayCastManager::raycast(ray_down_right, entityManager, 45.0f, { "Terrain" });

    // Gestion del landeo
    if (hit_down_left.has_value() || hit_down_right.has_value()) {
        // Solo comprobar si estas en el aire
        if (!animator->getBool("isGrounded") && jumpingTimeLeft <= 0) {
            animator->setBool("isGrounded", true);
            animator->setBool("isFalling", false);
            rigidBody->velocityX = 0;
            audioManager->playSound("land");
        }
    }
    else {
        animator->setBool("isGrounded", false);
    }

    // Gestion del walljump

    // Son 2 los raycasts por cada lado que hay que lanzar para saber si estas 
    // tocando alguna pared
    SDL_FPoint megaman_up_point = {transform->posX, transform->posY - 35};
    SDL_FPoint megaman_down_point = {transform->posX, transform->posY + 35};
    SDL_FPoint right_direction = {1.0f, 0.0f};
    SDL_FPoint left_direction = {-1.0f, 0.0f};
    Ray ray_right_up(megaman_up_point, right_direction);
    Ray ray_right_down(megaman_down_point, right_direction);
    Ray ray_left_up(megaman_up_point, left_direction);
    Ray ray_left_down(megaman_down_point, left_direction);

    std::optional<RaycastHit> hit_left_up = RayCastManager::raycast(ray_left_up, entityManager, 45.0f, { "Terrain" });
    std::optional<RaycastHit> hit_left_down = RayCastManager::raycast(ray_left_down, entityManager, 45.0f, { "Terrain" });
    std::optional<RaycastHit> hit_right_up = RayCastManager::raycast(ray_right_up, entityManager, 45.0f, { "Terrain" });  
    std::optional<RaycastHit> hit_right_down = RayCastManager::raycast(ray_right_down, entityManager, 45.0f, { "Terrain" });  

    // Si tocas algun terrain izda o dcha
    if (hit_left_up.has_value() || hit_left_down.has_value()
        || hit_right_up.has_value() || hit_right_down.has_value()) {
        Debug::Log("[?] Tocando pared con raycast"); 
        animator->setBool("touchingWall", true);
    }
    else {
        animator->setBool("touchingWall", false);
    }
    // Falling
    if ((animator->currentState() == "jump" || animator->currentState() == "jumpShooting") && rigidBody->velocityY > 0.0)
    {
        animator->setBool("isFalling", true);
    }
}




MegamanControllerParameters MegamanControllerLoader::fromJSON(const nlohmann::json &j, EntityManager &entityManager)
{
    Transform *transform = entityManager.getComponent<Transform>(entityManager.getLast());
    if (!transform)
    {
        throw std::runtime_error("MegamanController requires a Transform component");
    }

    AnimatorComponent *animator = entityManager.getComponent<AnimatorComponent>(entityManager.getLast());
    if (!animator)
    {
        throw std::runtime_error("MegamanController requires a AnimatorComponent component");
    }

    Rigidbody *rigidbody = entityManager.getComponent<Rigidbody>(entityManager.getLast());
    if (!rigidbody)
    {
        throw std::runtime_error("MegamanController requires a Rigidbody component");
    }

    HealthComponent *health = entityManager.getComponent<HealthComponent>(entityManager.getLast());
    if (!health)
    {
        throw std::runtime_error("MegamanController requires a Rigidbody component");
    }

    MegamanControllerParameters params(
        transform,
        &entityManager,
        animator,
        rigidbody,
        health);

    return params;
}

MegamanController MegamanControllerLoader::createFromJSON(nlohmann::json &j, EntityManager &entityManager)
{
    return MegamanController(fromJSON(j, entityManager));
}
