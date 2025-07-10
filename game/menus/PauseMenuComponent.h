#include "Components.h"
#include "RenderTextManager.h"
#include "Debug.h"
#include "InputManager.h"
#include "menus/uiComponents/PowerMenuComponent.h"
#include "menus/uiComponents/MiniBarMenuComponent.h"
#pragma once


struct PauseMenuComponentParameters {
    Camera* camera;
    EntityManager* entityManager;

    PauseMenuComponentParameters(Camera* c, EntityManager *m) : camera(c) , entityManager(m) {}
};



struct PauseMenuComponent : public Component {

    Camera* camera;
    EntityManager* entityManager;
    MiniBarMenuComponent* miniBarMenuComponent;
    inline static std::string levelName;

    //**********Variables que posiblemente haya que cambiar */
    int numVida = 2;
    int vida = 6;
    int maxVidaMegaman = 10;

    // Estructura para almacenar la posición de los power-ups
    struct posicionPowerUps {
        float x;
        float y;
    };

    // Estructura para almacenar los power-ups
    struct PowerUps{
        posicionPowerUps posicion; // Posición del power-up en la pantalla
        int mode; // 0: no lo tiene, 1: lo tiene, 2: lo tiene pero está deshabilitado
        int levelPower; // Cuanto power up tiene el power-up
        int maxLevelPower; // Maximo power up tiene el power-up 
        Entity entity; // Entidad del power-up
        PowerMenuComponent* powerMenuComponent; // Componente del power-up
    };

    PowerUps powerUps[10]{
        {0.183f, 0.184f, 1, 10, 10, 0, nullptr}, // X.Buster
        {0.527f, 0.184f, 0, 10, 10, 0, nullptr}, // Strom.T
        {0.183f, 0.255f, 0, 10, 10, 0, nullptr}, // Homming.T (no lo vamos a usar)
        {0.527f, 0.255f, 0, 10, 10, 0, nullptr}, // E.Spart (no lo vamos a usar)
        {0.183f, 0.326f, 0, 10, 10, 0, nullptr}, // C.String
        {0.527f, 0.326f, 0, 10, 10, 0, nullptr}, // B.Cutter (no lo vamos a usar)
        {0.183f, 0.397f, 0, 10, 10, 0, nullptr}, // R.Shield (no lo vamos a usar)
        {0.527f, 0.397f, 0, 10 , 10 , 0 , nullptr}, // S.Ice
        {0.183f , 0.468f ,0 ,10 ,10 ,2 ,nullptr}, // Fire.W
        {0.527f , 0.468f ,2 ,10 ,10 ,2 ,nullptr} // Escape.U
    };

    // // Vector de posiciones de los power-ups
    // posicionPowerUps posicionPowerUps[10] = {
    //     {0.183f, 0.184f},
    //     {0.527f, 0.184f},
    //     {0.183f, 0.255f},
    //     {0.527f, 0.255f},
    //     {0.183f, 0.326f},
    //     {0.527f, 0.326f},
    //     {0.183f, 0.397f},
    //     {0.527f, 0.397f},
    //     {0.183f, 0.468f},
    //     {0.527f, 0.468f}
    // };

    
    // variables basura
    bool inputEnabled = true; // Flag para habilitar/deshabilitar la entrada
    int powerUpsRow = 2; // numero de power-ups por fila
    int powerUpsColumn = 5; // numero de power-ups por columna
    struct posicionCursor{
        int row;
        int column;
    };
    posicionCursor posicionCursor ={0,0}; // Posición del cursor

    PauseMenuComponent(PauseMenuComponentParameters params) : camera(params.camera), entityManager(params.entityManager) {}

    void moveUp(){
        inputEnabled = false; // Disable input while moving
        int powerUp;
        for(int i = 5; i >= 0; i--) {
            powerUp = ((i+posicionCursor.row)%5) * powerUpsRow + posicionCursor.column;
            if(powerUps[powerUp].mode == 2) {
                // habilitar el power-up
                powerUps[powerUp].mode = 1;
                powerUps[powerUp].powerMenuComponent->actualizarPower(powerUp, powerUps[powerUp].mode, powerUps[powerUp].levelPower, powerUps[powerUp].maxLevelPower);
                // deshabilitar el power-up
                powerUp= posicionCursor.row* powerUpsRow + posicionCursor.column;
                powerUps[powerUp].mode = 2;
                powerUps[powerUp].powerMenuComponent->actualizarPower(powerUp, powerUps[powerUp].mode, powerUps[powerUp].levelPower, powerUps[powerUp].maxLevelPower);
                // cambiar el cursor
                posicionCursor.row = ((i+posicionCursor.row)%5);
                Debug::Log("row: " + std::to_string(posicionCursor.row) + " column: " + std::to_string(posicionCursor.column));
                break;
            }
        }
    }

    void moveDown(){
        inputEnabled = false; // Disable input while moving
        int powerUp;
        for(int i = 0; i <powerUpsColumn  ; i++) {
            powerUp = ((i+posicionCursor.row)%5) * powerUpsRow + posicionCursor.column;
            if(powerUps[powerUp].mode == 2) {
                // habilitar el power-up
                powerUps[powerUp].mode = 1;
                powerUps[powerUp].powerMenuComponent->actualizarPower(powerUp, powerUps[powerUp].mode, powerUps[powerUp].levelPower, powerUps[powerUp].maxLevelPower);
                // deshabilitar el power-up
                powerUp= posicionCursor.row* powerUpsRow + posicionCursor.column;
                powerUps[powerUp].mode = 2;
                powerUps[powerUp].powerMenuComponent->actualizarPower(powerUp, powerUps[powerUp].mode, powerUps[powerUp].levelPower, powerUps[powerUp].maxLevelPower);
                // cambiar el cursor
                posicionCursor.row = ((i+posicionCursor.row)%5);
                Debug::Log("row: " + std::to_string(posicionCursor.row) + " column: " + std::to_string(posicionCursor.column));

                break;
            }
        }
    }

    void moveLeft(){
        inputEnabled = false; // Disable input while moving
        int powerUp;
        if (posicionCursor.column != 0){
            Debug::Log("row: " + std::to_string(posicionCursor.row) + " column: " + std::to_string(posicionCursor.column));

            posicionCursor.column--;
            for(int i = 0; i <powerUpsColumn; i++) {
                powerUp = ((i+posicionCursor.row)%5) * powerUpsRow + posicionCursor.column;
                if(powerUps[powerUp].mode == 2) {
                    // habilitar el power-up
                    powerUps[powerUp].mode = 1;
                    powerUps[powerUp].powerMenuComponent->actualizarPower(powerUp, powerUps[powerUp].mode, powerUps[powerUp].levelPower, powerUps[powerUp].maxLevelPower);
                    // deshabilitar el power-up
                    powerUp= posicionCursor.row* powerUpsRow + posicionCursor.column +1;
                    powerUps[powerUp].mode = 2;
                    powerUps[powerUp].powerMenuComponent->actualizarPower(powerUp, powerUps[powerUp].mode, powerUps[powerUp].levelPower, powerUps[powerUp].maxLevelPower);
                    // cambiar el cursor
                    posicionCursor.row = ((i+posicionCursor.row)%5);

                    break;
                }
            }
        }
    }

    void moveRight(){
        inputEnabled = false; // Disable input while moving
        int powerUp;
        if (posicionCursor.column == 0){
            Debug::Log("row: " + std::to_string(posicionCursor.row) + " column: " + std::to_string(posicionCursor.column));
            posicionCursor.column++;
            for(int i = 0; i <powerUpsColumn; i++) {
                powerUp = ((i+posicionCursor.row)%5) * powerUpsRow + posicionCursor.column;
                Debug::Log("row: " + std::to_string(powerUp));

                if(powerUps[powerUp].mode == 2) {
                    // habilitar el power-up
                    powerUps[powerUp].mode = 1;
                    powerUps[powerUp].powerMenuComponent->actualizarPower(powerUp, powerUps[powerUp].mode, powerUps[powerUp].levelPower, powerUps[powerUp].maxLevelPower);
                    // deshabilitar el power-up
                    powerUp= posicionCursor.row* powerUpsRow + posicionCursor.column - 1;
                    powerUps[powerUp].mode = 2;
                    powerUps[powerUp].powerMenuComponent->actualizarPower(powerUp, powerUps[powerUp].mode, powerUps[powerUp].levelPower, powerUps[powerUp].maxLevelPower);
                    // cambiar el cursor
                    posicionCursor.row = ((i+posicionCursor.row)%5);
                    Debug::Log("row: " + std::to_string(powerUp));
                    break;
                }
            }
        }
    }
    
    void start() override
    {
        Debug::Log("PENE");

        if (!PrefabManager::getInstance().hasPrefab("powerMenu")) {
            PrefabManager::getInstance().loadPrefab("powerMenu");
        }

        for (int i = 0; i < 10; i++) {
            if(powerUps[i].mode == 0) {
                continue;
            }
            powerUps[i].entity = PrefabManager::getInstance().instantiatePrefab("powerMenu", *entityManager);
            Transform* transform = entityManager->getComponent<Transform>(powerUps[i].entity);
            powerUps[i].powerMenuComponent = entityManager->getComponent<PowerMenuComponent>(powerUps[i].entity);
            transform->posX = powerUps[i].posicion.x;
            transform->posY = powerUps[i].posicion.y;
            powerUps[i].powerMenuComponent->actualizarPower(i, powerUps[i].mode, powerUps[i].levelPower, powerUps[i].maxLevelPower);
            powerUps[i].powerMenuComponent->start();
        }
        if (!PrefabManager::getInstance().hasPrefab("minilifeBarMenu")) {
            PrefabManager::getInstance().loadPrefab("minilifeBarMenu");
        }
        Entity entity = PrefabManager::getInstance().instantiatePrefab("minilifeBarMenu", *entityManager);
        miniBarMenuComponent = entityManager->getComponent<MiniBarMenuComponent>(entity);
        miniBarMenuComponent->actualizarBarraVida(vida, maxVidaMegaman);
        miniBarMenuComponent->start();
        RenderTextManager::getInstance().loadFont("selectedFont", "sprites/letters/lettersSelected", 7, 7);
        entityManager->getSceneManager()->getScene("pauseMenu")->deactivate();
    }

    void update(float deltaTime) override
    {
        Debug::Log("PENEUpdate");
        if(InputManager::isKeyPressed(SDL_SCANCODE_A) && inputEnabled) {
            moveLeft();
        }
        if(InputManager::isKeyPressed(SDL_SCANCODE_D) && inputEnabled) {
            moveRight();
        }
        if(InputManager::isKeyPressed(SDL_SCANCODE_W) && inputEnabled) {
            moveUp();
        }
        if(InputManager::isKeyPressed(SDL_SCANCODE_S) && inputEnabled) {
            moveDown();
        }

        if(InputManager::isKeyPressed(SDL_SCANCODE_ESCAPE) && inputEnabled) {
            entityManager->getSceneManager()->getScene(levelName)->activate();
            entityManager->getSceneManager()->getScene("pauseMenu")->deactivate();
        }

        if(!InputManager::isKeyHeld(SDL_SCANCODE_A) &&
            !InputManager::isKeyHeld(SDL_SCANCODE_D) &&
            !InputManager::isKeyHeld(SDL_SCANCODE_W) &&
            !InputManager::isKeyHeld(SDL_SCANCODE_S)&&
            !InputManager::isKeyHeld(SDL_SCANCODE_ESCAPE)) {
            inputEnabled = true; // Re-enable input when the key is released
        }

        MegamanController* mc = MegamanController::getInstance();
        if (mc) {
            HealthComponent* healthComponent = entityManager->getComponent<HealthComponent>(mc->parent);
            //vida = healthComponent->getCurrentHealth();
            //numVida = healthComponent->getMaxHealth();
        }


    }

    void render(EntityManager& entityManager, SDL_Renderer* renderer) override 
    {
        RenderTextManager::getInstance().renderText("selectedFont",std::to_string(numVida), 0.72f, 0.78f, 7, 7, &entityManager);
    };
};

class PauseMenuComponentLoader {
    public:

        static PauseMenuComponentParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
            std::shared_ptr<Camera> cameraPtr ;
            entityManager.findComponent<Camera>(cameraPtr);
            Camera* camera = cameraPtr.get();
            if (!camera) {
                throw std::runtime_error("PauseMenuComponent requires a Camera component");
            }
    
            PauseMenuComponentParameters params(camera, &entityManager);
    
            return params;
        }
    
        // Helper function to directly create a Camera component from JSON
        static PauseMenuComponent createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
            return PauseMenuComponent(fromJSON(j, entityManager));
        }
};