#pragma once
#include <string>
#include <stdexcept>
#include "EntityManager.h"
#include "Components.h"
#include "megaman/MegamanController.h"
#include "RenderTextManager.h"
#include <SDL2/SDL_image.h>
#include "menus/OptionsMenuComponent.h"
#include "Debug.h"

struct CameraTrigger{
    float greaterX;
    float greaterY;
    float lessX;
    float lessY;
    float x;
    float y;
    std::string texturePath; // Texture for the trigger
};

struct MegamanCameraParameters {
    EntityManager* entityManager;
    Vector2D levelBoundsMin;   // Level boundaries
    Vector2D levelBoundsMax;
    float verticalThreshold;   // Vertical movement threshold
    float followSpeed;         // Camera follow speed
    float bossZone;
    Vector2D bossCenter;
    Vector2D bossSpawn;
    std::vector<CameraTrigger> cameraTriggers; // Camera bounds triggers
    std::vector<Vector2D> cameraMinBounds; // Camera bounds triggers
    std::vector<Vector2D> cameraMaxBounds; // Camera bounds triggers
    std::string bossName;
};



class MegamanCamera : public Component {
    private:
        EntityManager* entityManager;
        Camera* camera;
        Transform* cameraTransform;    // Camera's Transform
        Transform* megamanTransform;   // Mega Man's Transform
        Vector2D levelBoundsMin;       // Minimum level bounds (e.g., {0, 0})
        Vector2D levelBoundsMax;       // Maximum level bounds (e.g., {levelWidth, levelHeight})
        float verticalThreshold;       // Distance Mega Man must move vertically before camera follows
        float followSpeed;             // Smoothing factor for camera movement (0.0f to 1.0f)
        float bossZone;
        int actual_Trigger = 0;
        Vector2D bossCenter;
        Vector2D bossSpawn;
        std::vector<CameraTrigger> cameraTriggers; // Camera bounds triggers
        std::vector<Vector2D> cameraMinBounds; // Camera bounds triggers
        std::vector<Vector2D> cameraMaxBounds; // Camera bounds triggers
        std::string bossName;

        SpriteRenderer* spriteRenderer; // Optional sprite renderer for the camera

        bool inputEnabled = true; // Flag to enable/disable input handling

        bool inBossZone;
    
        // Linear interpolation helper
        float lerp(float a, float b, float t) const {
            return a + t * (b - a);
        }
    
    public:
        
    
        MegamanCamera(MegamanCameraParameters params) 
            : entityManager(params.entityManager),
              levelBoundsMin(params.levelBoundsMin),
              levelBoundsMax(params.levelBoundsMax),
              verticalThreshold(params.verticalThreshold),
              followSpeed(params.followSpeed),
              bossZone(params.bossZone),
              inBossZone(false),
              bossCenter(params.bossCenter),
              bossSpawn(params.bossSpawn),
              cameraTriggers(params.cameraTriggers),
              cameraMinBounds(params.cameraMinBounds),
              cameraMaxBounds(params.cameraMaxBounds),
              bossName(params.bossName) {
            if (!entityManager) {
                throw std::runtime_error("MegamanCamera requires a valid EntityManager");
            }
    
        }
    
        void start() override {
            Debug::Log("MegamanCamera started");

            camera = entityManager->getComponent<Camera>(parent);
            if (!camera) {
                throw std::runtime_error("MegamanCamera requires a Camera component on its entity");
            }

            // Initialize camera transform
            cameraTransform = entityManager->getComponent<Transform>(parent);
            if (!cameraTransform) {
                throw std::runtime_error("MegamanCamera requires a Transform component on its entity");
            }

            getMegamanTransform(); // Initialize Mega

            

            camera->zoom = 1.6;


            clampToBounds(); // Ensure initial position is within bounds

            RenderTextManager::getInstance().loadFont("selectedFont", "sprites/letters/lettersSelected", 7, 7);

            std::shared_ptr<ParallaxLayer> parallaxLayer;

            Entity t = entityManager->findComponent<ParallaxLayer>(parallaxLayer);
            if (t != -1) {
                spriteRenderer = entityManager->getComponent<SpriteRenderer>(t);
            } else {
                spriteRenderer = nullptr;
            }
        }

        bool getMegamanTransform()
        {
            std::shared_ptr<MegamanController> mc;

            Entity t = entityManager->findComponent<MegamanController>(mc);
    
            // Initialize Mega Man's transform
            megamanTransform = entityManager->getComponent<Transform>(t);
            if (!megamanTransform || t==0) {
                
                std::shared_ptr<MegamanArmorController> mAc;

                t = entityManager->findComponent<MegamanArmorController>(mAc);
    
                // Initialize Mega Man's transform
                megamanTransform = entityManager->getComponent<Transform>(t);

                if (!megamanTransform || t==0) {
                    //Debug::Log("MEGAMAN AND ARMOR NOT FOUND");
                    return false;
                }

                //Debug::Log("MEGAMAN ARMOR FOUND");
                return true;
            }

            //Debug::Log("MEGAMAN FOUND");
    
            // Center camera on Mega Man initially
            cameraTransform->posX = megamanTransform->posX;
            cameraTransform->posY = megamanTransform->posY;

            return true;
        }
    
        void update(float deltaTime) override {

            if(!getMegamanTransform()) return;

            if(inBossZone)
            {
                //cameraTransform->posX = bossCenter.x;
                //cameraTransform->posY = bossCenter.y;
                //return;
            }
            else
            {
                if(megamanTransform->posX>bossZone)
                {
                    inBossZone = true;
                    //cameraTransform->posX = bossCenter.x;
                    //cameraTransform->posY = bossCenter.y;

                    if (!PrefabManager::getInstance().hasPrefab(bossName)) {
                        PrefabManager::getInstance().loadPrefab(bossName);
                    }
                    Entity penguin = PrefabManager::getInstance().instantiatePrefab(bossName, *entityManager);
                    Transform* tP = entityManager->getComponent<Transform>(penguin);
                    if(tP)
                    {
                        tP->posX = bossSpawn.x;
                        tP->posY = bossSpawn.y;
                    }

                    return;
                }
            }

             // Check greaterX condition

            bool conditionsMet = true; // Flag to check if all conditions are met

            if (cameraTriggers[actual_Trigger].greaterX != -100000) {
                if (megamanTransform->posX <= cameraTriggers[actual_Trigger].greaterX) {
                    conditionsMet = false;
                }
            }

            // Check greaterY condition
            if (cameraTriggers[actual_Trigger].greaterY != -100000) {
                if (megamanTransform->posY <= cameraTriggers[actual_Trigger].greaterY) {
                    conditionsMet = false;
                }
            }

            // Check lessX condition
            if (cameraTriggers[actual_Trigger].lessX != -100000) {
                if (megamanTransform->posX >= cameraTriggers[actual_Trigger].lessX) {
                    conditionsMet = false;
                }
            }

            // Check lessY condition
            if (cameraTriggers[actual_Trigger].lessY != -100000) {
                if (megamanTransform->posY >= cameraTriggers[actual_Trigger].lessY) {
                    conditionsMet = false;
                }
            }

            // If all conditions are met, update bounds and advance trigger
            if (conditionsMet) {
                levelBoundsMin.x = cameraMinBounds[actual_Trigger].x;
                levelBoundsMax.x = cameraMaxBounds[actual_Trigger].x;
                levelBoundsMin.y = cameraMinBounds[actual_Trigger].y;
                levelBoundsMax.y = cameraMaxBounds[actual_Trigger].y;

                if(cameraTriggers[actual_Trigger].texturePath != "")
                {
                    if(spriteRenderer)
                    {
                        spriteRenderer->texture = TextureManager::getInstance()->getTexture(cameraTriggers[actual_Trigger].texturePath);
                        SDL_QueryTexture(spriteRenderer->texture, nullptr, nullptr, &(spriteRenderer->width), &(spriteRenderer->height));
                    }
                }


                actual_Trigger++;
                if(actual_Trigger >= cameraTriggers.size()) actual_Trigger = cameraTriggers.size()-1;
            }

            if(InputManager::isKeyPressed(SDL_SCANCODE_F1) && inputEnabled)
            {
                //Go to the next trigger
                megamanTransform->posX = cameraTriggers[actual_Trigger].x + 1;
                megamanTransform->posY = cameraTriggers[actual_Trigger].y + 1;

                levelBoundsMin.x = cameraMinBounds[actual_Trigger].x;
                levelBoundsMax.x = cameraMaxBounds[actual_Trigger].x;
                levelBoundsMin.y = cameraMinBounds[actual_Trigger].y;
                levelBoundsMax.y = cameraMaxBounds[actual_Trigger].y;

                if(cameraTriggers[actual_Trigger].texturePath != "")
                {
                    if(spriteRenderer)
                    {
                        spriteRenderer->texture = TextureManager::getInstance()->getTexture(cameraTriggers[actual_Trigger].texturePath);
                    }
                }


                actual_Trigger++;
                if(actual_Trigger >= cameraTriggers.size()) actual_Trigger = cameraTriggers.size()-1;

                inputEnabled = false; 
            }
            else if(!InputManager::isKeyPressed(SDL_SCANCODE_F1) && !inputEnabled)
            {
                inputEnabled = true;
            }

            cameraTransform->posX = megamanTransform->posX;
            cameraTransform->posY = megamanTransform->posY;

            clampToBounds(); // Ensure camera stays within level bounds

            if(megamanTransform->posY > 8000)
            {
                std::shared_ptr<MegamanController> mc;
    
                HealthComponent* megamanLife = entityManager->getComponent<HealthComponent>(entityManager->findComponent<MegamanController>(mc));

                if(megamanLife && megamanTransform->posY > 10000){
                    megamanLife->takeDamage(10000);
                }
                else
                {
                    std::shared_ptr<MegamanArmorController> mAc;

                    megamanLife = entityManager->getComponent<HealthComponent>(entityManager->findComponent<MegamanArmorController>(mAc));

                    if(megamanLife){
                        megamanLife->takeDamage(10000);
                    }
                }
            }
        }

        void render(EntityManager& entityManager,SDL_Renderer* renderer) override
        {
            //RenderTextManager::getInstance().renderText("selectedFont", std::to_string(camera->getPosX()) + "   " + std::to_string(camera->getPosY()), 0.0f, 0.0f, 4, 4, &entityManager);

            //RenderTextManager::getInstance().renderText("selectedFont", std::to_string(megamanTransform->posX) + "   " + std::to_string(megamanTransform->posY), 0.0f, 0.5f, 4, 4, &entityManager);
        }
    
    private:
        void clampToBounds() {
            // Ensure camera's view edges stay within level bounds by clamping the center

            if(camera->getPosX() < levelBoundsMin.x)
            {
                cameraTransform->posX = levelBoundsMin.x;
            }
            else if(camera->getPosX() > levelBoundsMax.x)
            {
                cameraTransform->posX = levelBoundsMax.x;
            }
            
            if(camera->getPosY() < levelBoundsMin.y)
            {
                cameraTransform->posY = levelBoundsMin.y;
            }
            else if(camera->getPosY() > levelBoundsMax.y)
            {
                cameraTransform->posY = levelBoundsMax.y;
            }
        }
    };

    

    class MegamanCameraLoader {
        public:
    
            static MegamanCameraParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
    
                MegamanCameraParameters params;

                params.entityManager = &entityManager;
                params.followSpeed = j.value("followSpeed", 0.1f);
                params.verticalThreshold = j.value("verticalThreshold", 100);
                params.levelBoundsMin.x = j.value("minX", -100000);
                params.levelBoundsMin.y = j.value("minY", -100000);
                params.levelBoundsMax.x = j.value("maxX", +100000);
                params.levelBoundsMax.y = j.value("maxY", +100000);

                params.bossZone = j.value("bossTrigger", 0);
                params.bossCenter.x= j.value("bossCenterX", 0);
                params.bossCenter.y = j.value("bossCenterY", 0);
                
                params.bossSpawn.x= j.value("bossSpawnX", 0);
                params.bossSpawn.y = j.value("bossSpawnY", 0);
                params.bossName = j.value("bossName", "chillpenguinIA");


                if(OptionsMenuComponent::advancedIAEnabled)
                {
                    if (params.bossName.size() >= 2 && params.bossName.substr(params.bossName.size() - 2) == "IA") {
                        params.bossName.replace(params.bossName.size() - 2, 2, "AdvancedIA");
                    }
                }

                params.cameraMinBounds.clear();
                if (j.contains("cameraMinBounds")) {
                    for (const auto& trigger : j["cameraMinBounds"]) {
                        Vector2D triggerPos;
                        triggerPos.x = trigger.value("x", 0);
                        triggerPos.y = trigger.value("y", 0);
                        params.cameraMinBounds.push_back(triggerPos);
                    }
                }

                params.cameraMaxBounds.clear();
                if (j.contains("cameraMaxBounds")) {
                    for (const auto& trigger : j["cameraMaxBounds"]) {
                        Vector2D triggerPos;
                        triggerPos.x = trigger.value("x", 0);
                        triggerPos.y = trigger.value("y", 0);
                        params.cameraMaxBounds.push_back(triggerPos);
                    }
                }

                params.cameraTriggers.clear();
                if (j.contains("cameraTriggers")) {
                    for (const auto& trigger : j["cameraTriggers"]) {
                        CameraTrigger cTrigger;
                        cTrigger.greaterX  = trigger.value("gX", -100000);
                        cTrigger.greaterY = trigger.value("gY", -100000);
                        cTrigger.lessX = trigger.value("lX", -100000);
                        cTrigger.lessY = trigger.value("lY", -100000);
                        cTrigger.x = trigger.value("x", -100000);
                        cTrigger.y = trigger.value("y", -100000);
                        cTrigger.texturePath = ""; // Load texture if needed
                        if (trigger.contains("texturePath")) {
                            cTrigger.texturePath = "resources/" + trigger.value("texturePath", "");
                        }
                        params.cameraTriggers.push_back(cTrigger);
                    }
                }
        
                return params;
            }
        
            // Helper function to directly create a MegamanControlelr component from JSON
            static MegamanCamera createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
                return MegamanCamera(fromJSON(j, entityManager));
            }
    };
