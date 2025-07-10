#include "Components.h"
#include "RenderTextManager.h"
#include "Debug.h"
#include "InputManager.h"
#include "menus/uiComponents/MegamanMainMenuComponent.h"
#include "menus/uiComponents/BulletMainMenuComponent.h"
#include "MegamanLevelsManager.h"

struct MainMenuComponentParameters {
    Camera* camera;
    EntityManager* entityManager;

    MainMenuComponentParameters(Camera* c, EntityManager *m) : camera(c) , entityManager(m) {}
};



struct MainMenuComponent : public Component {

    Camera* camera;
    Transform* transformMegaman;
    Transform* transformBullet;
    EntityManager* entityManager;
    AnimatorComponent* animatorMegaman;
    AnimatorComponent* animatorBullet;
    int selectedOption = 0; // 0: Start Game, 1: Options, 2: Exit
    bool inputEnabled = true;
    struct position {
        float posX;
        float posY;
    };
    position posicionMegaman[3] = {
        {0.1f, 0.65f},
        {0.1f, 0.70f},
        {0.1f, 0.75f}
    };

    position posicionBullet[3] = {
        {0.2f, 0.66f},
        {0.2f, 0.71f},
        {0.2f, 0.76f}
    };
    std::shared_ptr<MegamanMainMenuComponent> megamanMenu;
    std::shared_ptr<BulletMainMenuComponent> bulletMenu;
    

    MainMenuComponent(MainMenuComponentParameters params) : camera(params.camera), entityManager(params.entityManager) {}

    void start() override
    {
        RenderTextManager::getInstance().loadFont("selectedFont", "sprites/letters/lettersSelected", 7, 7);
        RenderTextManager::getInstance().loadFont("unselectedFont", "sprites/letters/lettersUnselected", 7, 7);

        entityManager->findComponent<MegamanMainMenuComponent>(megamanMenu);
        entityManager->findComponent<BulletMainMenuComponent>(bulletMenu);

        transformMegaman = entityManager->getComponent<Transform>(megamanMenu->parent);
        transformBullet = entityManager->getComponent<Transform>(bulletMenu->parent);

        animatorMegaman = entityManager->getComponent<AnimatorComponent>(megamanMenu->parent);
        animatorBullet = entityManager->getComponent<AnimatorComponent>(bulletMenu->parent);
        
        animatorMegaman->addFrameTrigger("shoot", 1, [this](){
            animatorMegaman->setBool("isShooting", false);
        });
        
        animatorBullet->addFrameTrigger("shoot", 1, [this](){
            animatorBullet->setBool("isShooting", false);
            animatorBullet->setBool("isMovShooting", true);
        });

        animatorBullet->addFrameTrigger("charge", 1, [this](){
            animatorBullet->setBool("isCharging", false);
            animatorBullet->setBool("isShooting", true);
        });
    }

    void update(float deltaTime) override
    {
        // Check for key presses A
        if (InputManager::isKeyPressed(SDL_SCANCODE_W) && inputEnabled) {
            selectedOption = (selectedOption - 1 + 3) % 3; // Move down the menu
            inputEnabled = false; // Disable input for a short time
            transformMegaman->posY= posicionMegaman[selectedOption].posY;
            transformMegaman->posX= posicionMegaman[selectedOption].posX; 
        }
        else if(InputManager::isKeyPressed(SDL_SCANCODE_S) && inputEnabled) {
            selectedOption = (selectedOption + 1 ) % 3; // Move up the menu
            inputEnabled = false; // Disable input for a short time
            transformMegaman->posY= posicionMegaman[selectedOption].posY;
            transformMegaman->posX= posicionMegaman[selectedOption].posX;
        }
        else if (!InputManager::isKeyHeld(SDL_SCANCODE_W) && !InputManager::isKeyHeld(SDL_SCANCODE_S) && !InputManager::isKeyHeld(SDL_SCANCODE_SPACE)) {
            inputEnabled = true; // Re-enable input when the key is released
        }
        
        if (InputManager::isKeyPressed(SDL_SCANCODE_SPACE) && inputEnabled) {
            transformBullet->posY= posicionBullet[selectedOption].posY;
            transformBullet->posX= posicionBullet[selectedOption].posX;
            animatorBullet->restart_current_state();
            animatorMegaman->setBool("isShooting", true);
            
            if(selectedOption==0)
            {
                MegamanLevelsManager::getInstance()->loadLevel("levelSelectorMenu",entityManager->getSceneManager()->getSceneNameByEntityManager(entityManager));
            }
            else if(selectedOption==1)
            {
                MegamanLevelsManager::getInstance()->loadLevel("optionsMenu",entityManager->getSceneManager()->getSceneNameByEntityManager(entityManager));
            }
            else if(selectedOption==2)
            {
                Engine::getInstance()->close(); // Close the game
            }
        }

        if (animatorBullet->currentState() == "movShoot") {
            transformBullet->posX += 0.4f * deltaTime ;
        }

    }

    void render(EntityManager& entityManager, SDL_Renderer* renderer) override 
    {
        if (selectedOption == 0) {
            RenderTextManager::getInstance().renderText("selectedFont", "game  start", 0.3f, 0.70f, 4, 4, &entityManager);
            RenderTextManager::getInstance().renderText("unselectedFont", "option  mode", 0.3f, 0.75f, 4, 4, &entityManager);
            RenderTextManager::getInstance().renderText("unselectedFont", "exit", 0.3f, 0.80f, 4, 4, &entityManager);
        } else if (selectedOption == 1) {
            RenderTextManager::getInstance().renderText("unselectedFont", "game  start", 0.3f, 0.70f, 4, 4, &entityManager);
            RenderTextManager::getInstance().renderText("selectedFont", "option  mode", 0.3f, 0.75f, 4, 4, &entityManager);
            RenderTextManager::getInstance().renderText("unselectedFont", "exit", 0.3f, 0.80f, 4, 4, &entityManager);
        } else if (selectedOption == 2) {
            RenderTextManager::getInstance().renderText("unselectedFont", "game  start", 0.3f, 0.70f, 4, 4, &entityManager);
            RenderTextManager::getInstance().renderText("unselectedFont", "option  mode", 0.3f, 0.75f, 4, 4, &entityManager);
            RenderTextManager::getInstance().renderText("selectedFont", "exit", 0.3f, 0.80f, 4, 4, &entityManager);
        }
    };
};

class MainMenuComponentLoader {
    public:

        static MainMenuComponentParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            std::shared_ptr<Camera> cameraPtr ;
            entityManager.findComponent<Camera>(cameraPtr);
            Camera* camera = cameraPtr.get();
            if (!camera) {
                throw std::runtime_error("MainMenuComponent requires a Camera component");
            }
    
            MainMenuComponentParameters params(camera, &entityManager);
    
            return params;
        }
    
        // Helper function to directly create a Camera component from JSON
        static MainMenuComponent createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
            return MainMenuComponent(fromJSON(j, entityManager));
        }
    };