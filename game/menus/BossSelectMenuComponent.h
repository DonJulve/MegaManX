#include "Components.h"
#include "RenderTextManager.h"
#include "menus/uiComponents/SelectorMenuComponent.h"
#include "menus/uiComponents/BossMenuComponent.h"
#include "menus/uiComponents/PointMapMenuComponent.h"
#include "menus/uiComponents/MapMenuComponent.h"
#include "menus/uiComponents/StageMenuComponent.h"
#include "menus/uiComponents/SpecMenuComponent.h"
#include "menus/uiComponents/MapBgMenuComponent.h"
#include "InputManager.h"
#include "MegamanLevelsManager.h"
#include "Debug.h"

struct BossSelectComponentParameters
{
    Camera *camera;
    EntityManager *entityManager;

    BossSelectComponentParameters(Camera *c, EntityManager *em) : camera(c), entityManager(em) {}
};

struct BossSelectComponent : public Component
{

    Camera *camera;
    Transform *transformSelector;
    Transform *transformPointMap;
    EntityManager *entityManager;
    SpriteRenderer *renderMapMenu;
    SpriteRenderer *renderStageMenu;
    SpriteRenderer *renderSpecMenu;
    SpriteRenderer *renderMapBgMenu;

    bool inputEnabled = true;      // Flag para habilitar/deshabilitar la entrada
    int selecPosition[2] = {0, 0}; // Variable para almacenar la posicion del selector actual
    int bossSelected = 0;          // Variable para almacenar el boss seleccionado
    int mode = 0;                  // Variable para almacenar el modo actual (0 = stage, 1 = mapa, 2 = spec)

    // struct para almacenar posiciones
    struct position
    {
        float posX;
        float posY;
    };

    // Posiciones de los elementos  selector del menu
    position posicionSelector[4][4] = {
        {{0.19f, 0.09f}, {0.35f, 0.09f}, {0.51f, 0.09f}, {0.67f, 0.09f}},
        {{0.19f, 0.30f}, {0.355f, 0.3f}, {0.355f, 0.3f}, {0.67f, 0.3f}},
        {{0.19f, 0.51f}, {0.355f, 0.3f}, {0.355f, 0.3f}, {0.67f, 0.51f}},
        {{0.19f, 0.72f}, {0.35f, 0.72f}, {0.51f, 0.72f}, {0.67f, 0.72f}}};

    // varible para almacenar la posicion del punto del mapa cuando estoy en modo map
    position posicionPoint[4][4] = {
        {{0.2f, 0.2f}, {0.37f, 0.41f}, {0.51f, 0.33f}, {0.2f, 0.2f}},
        {{0.58f, 0.36f}, {0.2f, 0.2f}, {0.2f, 0.2f}, {0.39f, 0.56f}},
        {{0.6f, 0.57f}, {0.2f, 0.2f}, {0.2f, 0.2f}, {0.6f, 0.42f}},
        {{0.2f, 0.2f}, {0.52f, 0.57f}, {0.46f, 0.38f}, {0.2f, 0.2f}}};

    // Variable para saber si el selector esta en el centro
    // si es 1, el selector esta en el centro
    // si es 0, el selector no esta en el centro
    int estaCentro[4][4] = {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}};

    // Variable para almacenar el tamaño del selector
    struct size
    {
        float sizeX;
        float sizeY;
    };

    // Variable para el size dependiendo de si esta en el centro o no
    // posicion 0 es el tamaño del selector cuando no esta en el centro
    // posicion 1 es el tamaño del selector cuando esta en el centro
    size sizeSelector[2] = {
        {2.66f, 2.66f},
        {5.5f, 5.5f}};

    std::shared_ptr<SelectorMenuComponent> selector;
    std::shared_ptr<PointMapMenuComponent> pointMap;
    std::shared_ptr<MapMenuComponent> mapMenu;
    std::shared_ptr<StageMenuComponent> stageMenu;
    std::shared_ptr<SpecMenuComponent> specMenu;
    std::shared_ptr<MapBgMenuComponent> mapBgMenu;

    BossSelectComponent(BossSelectComponentParameters params) : camera(params.camera), entityManager(params.entityManager) {}

    // Funcion que guarda el boss seleccionado
    // dependiendo de la posicion del selector
    // para que en el caso de que no este en ningun boss, se guarde el anterior
    void getBossSelected()
    {
        if (selecPosition[0] == 0 && selecPosition[1] == 1)
        {
            bossSelected = 0;
        }
        else if (selecPosition[0] == 0 && selecPosition[1] == 2)
        {
            bossSelected = 1;
        }
        else if (selecPosition[0] == 1 && selecPosition[1] == 0)
        {
            bossSelected = 2;
        }
        else if (selecPosition[0] == 1 && selecPosition[1] == 3)
        {
            bossSelected = 3;
        }
        else if (selecPosition[0] == 2 && selecPosition[1] == 0)
        {
            bossSelected = 4;
        }
        else if (selecPosition[0] == 2 && selecPosition[1] == 3)
        {
            bossSelected = 5;
        }
        else if (selecPosition[0] == 3 && selecPosition[1] == 1)
        {
            bossSelected = 6;
        }
        else if (selecPosition[0] == 3 && selecPosition[1] == 2)
        {
            bossSelected = 7;
        }
    }

    // Funcion que actualice la foto del centro dependiendo del modo en el que estemos
    void updateBg()
    {
        if (mode == 1)
        {
            transformPointMap->posX = posicionPoint[selecPosition[0]][selecPosition[1]].posX;
            transformPointMap->posY = posicionPoint[selecPosition[0]][selecPosition[1]].posY;
        }
        else if (mode == 0)
        {
            // Actualizar el boss seleccionado
            getBossSelected();
            std::string bg = "resources/sprites/menu/menuStageBg/menuStageBg_" + std::to_string(bossSelected) + ".png";
            renderMapBgMenu->texture = TextureManager::getInstance()->getTexture(bg);
        }
        else if (mode == 2)
        {
            // Actualizar el boss seleccionado
            getBossSelected();
            std::string bg = "resources/sprites/menu/menuSpecBg/menuSpecBg_" + std::to_string(bossSelected) + ".png";
            renderMapBgMenu->texture = TextureManager::getInstance()->getTexture(bg);
        }
    }

    // Funcion para mover el selector hacia arriba
    void moveSelectorUp(int eje)
    {
        if (selecPosition[eje] < 3)
        {
            if (selecPosition[eje] == 1 && (selecPosition[(eje + 1) % 2] == 1 || selecPosition[(eje + 1) % 2] == 2))
            {
                selecPosition[eje] = 2;
            }
            selecPosition[eje] = (selecPosition[eje] + 1) % 4;
            // Si esta en el centro, cambiar el tamaño del +
            if (estaCentro[selecPosition[0]][selecPosition[1]] == 1)
            {
                transformSelector->sizeX = sizeSelector[1].sizeX;
                transformSelector->sizeY = sizeSelector[1].sizeY;
            }
            else
            {
                // Si no esta en el centro, cambiar el tamaño del selector
                transformSelector->sizeX = sizeSelector[0].sizeX;
                transformSelector->sizeY = sizeSelector[0].sizeY;
            }

            // Actualizar la posicion del selector
            transformSelector->posX = posicionSelector[selecPosition[0]][selecPosition[1]].posX;
            transformSelector->posY = posicionSelector[selecPosition[0]][selecPosition[1]].posY;

            // Actualizar la posicion del punto del mapa si estoy en modo mapa
            updateBg();
        }
        inputEnabled = false; // Disable input until the key is released
    }

    // Funcion para mover el selector hacia abajo
    void moveSelectorDown(int eje)
    {
        if (selecPosition[eje] > 0)
        {

            if (selecPosition[eje] == 2 && (selecPosition[(eje + 1) % 2] == 1 || selecPosition[(eje + 1) % 2] == 2))
            {
                selecPosition[eje] = 1;
            }

            selecPosition[eje] = (selecPosition[eje] - 1) % 4;
            // Si esta en el centro, cambiar el tamaño del selector
            if (estaCentro[selecPosition[0]][selecPosition[1]] == 1)
            {
                transformSelector->sizeX = sizeSelector[1].sizeX;
                transformSelector->sizeY = sizeSelector[1].sizeY;
            }
            else
            {
                // Si no esta en el centro, cambiar el tamaño del selector
                transformSelector->sizeX = sizeSelector[0].sizeX;
                transformSelector->sizeY = sizeSelector[0].sizeY;
            }

            // Actualizar la posicion del selector
            transformSelector->posX = posicionSelector[selecPosition[0]][selecPosition[1]].posX;
            transformSelector->posY = posicionSelector[selecPosition[0]][selecPosition[1]].posY;

            // Actualizar la posicion del punto del mapa si estoy en modo mapa
            updateBg();
        }
        inputEnabled = false; // Disable input until the key is released
    }

    // Funcion para activar un modo o desactivar un modo
    // activa = 1 para activar el modo
    // activa = 0 para desactivar el modo
    void activateMode(int mode, int activa)
    {
        std::string ruta;
        if (mode == 0)
        {
            ruta = "resources/sprites/menu/menuStage/menuStage_" + std::to_string(activa) + ".png";
            renderStageMenu->texture = TextureManager::getInstance()->getTexture(ruta);
        }
        else if (mode == 1)
        {
            ruta = "resources/sprites/menu/menuMap/menuMap_" + std::to_string(activa) + ".png";
            renderMapMenu->texture = TextureManager::getInstance()->getTexture(ruta);
        }
        else if (mode == 2)
        {
            ruta = "resources/sprites/menu/menuSpec/menuSpec_" + std::to_string(activa) + ".png";
            renderSpecMenu->texture = TextureManager::getInstance()->getTexture(ruta);
        }
    }
    void start() override
    {
        RenderTextManager::getInstance().loadFont("unselectedFont", "sprites/letters/lettersUnselected", 7, 7);
        entityManager->findComponent<SelectorMenuComponent>(selector);
        entityManager->findComponent<PointMapMenuComponent>(pointMap);
        entityManager->findComponent<MapMenuComponent>(mapMenu);
        entityManager->findComponent<StageMenuComponent>(stageMenu);
        entityManager->findComponent<SpecMenuComponent>(specMenu);
        entityManager->findComponent<MapBgMenuComponent>(mapBgMenu);

        transformSelector = entityManager->getComponent<Transform>(selector->parent);
        transformPointMap = entityManager->getComponent<Transform>(pointMap->parent);
        renderMapMenu = entityManager->getComponent<SpriteRenderer>(mapMenu->parent);
        renderStageMenu = entityManager->getComponent<SpriteRenderer>(stageMenu->parent);
        renderSpecMenu = entityManager->getComponent<SpriteRenderer>(specMenu->parent);
        renderMapBgMenu = entityManager->getComponent<SpriteRenderer>(mapBgMenu->parent);
    }

    void update(float deltaTime) override
    {
        if (InputManager::isKeyPressed(SDL_SCANCODE_A) && inputEnabled)
        {
            moveSelectorDown(1);
        }
        if (InputManager::isKeyPressed(SDL_SCANCODE_D) && inputEnabled)
        {
            moveSelectorUp(1);
        }
        if (InputManager::isKeyPressed(SDL_SCANCODE_W) && inputEnabled)
        {
            moveSelectorDown(0);
        }
        if (InputManager::isKeyPressed(SDL_SCANCODE_S) && inputEnabled)
        {
            moveSelectorUp(0);
        }

        if (InputManager::isKeyPressed(SDL_SCANCODE_SPACE) && inputEnabled)
        {
            std::string bg;
            if (selecPosition[0] == 0 && selecPosition[1] == 3)
            {
                // desactivar el modo anterior
                activateMode(mode, 0);
                // activar el nuevo modo
                mode = 1;
                activateMode(mode, 1);
                bg = "resources/sprites/menu/menuMapBg/menuMapBg_0.png";
                renderMapBgMenu->texture = TextureManager::getInstance()->getTexture(bg);
            }
            else if (selecPosition[0] == 0 && selecPosition[1] == 0)
            {
                // desactivar el modo anterior
                activateMode(mode, 0);
                // activar el nuevo modo
                mode = 0;
                activateMode(mode, 1);
                bg = "resources/sprites/menu/menuStageBg/menuStageBg_0.png";
                renderMapBgMenu->texture = TextureManager::getInstance()->getTexture(bg);
            }
            else if (selecPosition[0] == 3 && selecPosition[1] == 0)
            {
                // desactivar el modo anterior
                activateMode(mode, 0);
                // activar el nuevo modo
                mode = 2;
                activateMode(mode, 1);
                bg = "resources/sprites/menu/menuSpecBg/menuSpecBg_0.png";
                renderMapBgMenu->texture = TextureManager::getInstance()->getTexture(bg);
            }
            else if (selecPosition[0] == 0 && selecPosition[1] == 2)
            {
                getBossSelected();
                BossMenuComponent::BossSeleccionado = bossSelected;
                MegamanLevelsManager::getInstance()->loadLevel("bossSelectMenu", entityManager->getSceneManager()->getSceneNameByEntityManager(entityManager));
            }
            else if (selecPosition[0] == 1 && selecPosition[1] == 3)
            {
                getBossSelected();
                BossMenuComponent::BossSeleccionado = bossSelected;
                MegamanLevelsManager::getInstance()->loadLevel("bossSelectMenu", entityManager->getSceneManager()->getSceneNameByEntityManager(entityManager));
            }
        }

        if (!InputManager::isKeyHeld(SDL_SCANCODE_S) &&
            !InputManager::isKeyHeld(SDL_SCANCODE_W) &&
            !InputManager::isKeyHeld(SDL_SCANCODE_A) &&
            !InputManager::isKeyHeld(SDL_SCANCODE_D) &&
            !InputManager::isKeyHeld(SDL_SCANCODE_SPACE))
        {
            inputEnabled = true; // Re-enable input when the key is released
        }
    }
};

class BossSelectComponentLoader
{
public:
    static BossSelectComponentParameters fromJSON(const nlohmann::json &j, EntityManager &entityManager)
    {
        std::shared_ptr<Camera> cameraPtr;
        entityManager.findComponent<Camera>(cameraPtr);
        Camera *camera = cameraPtr.get();
        if (!camera)
        {
            throw std::runtime_error("BossSelectComponent requires a Camera component");
        }

        BossSelectComponentParameters params(camera, &entityManager);

        return params;
    }

    // Helper function to directly create a Camera component from JSON
    static BossSelectComponent createFromJSON(nlohmann::json &j, EntityManager &entityManager)
    {
        return BossSelectComponent(fromJSON(j, entityManager));
    }
};