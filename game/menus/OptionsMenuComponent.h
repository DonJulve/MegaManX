#ifndef OPTIONSMENUCOMPONENT_H
#define OPTIONSMENUCOMPONENT_H
#include "Components.h"
#include "RenderTextManager.h"
#include "Debug.h"
#include <cctype> // for std::tolower
#include "InputManager.h"
#include "DynamicInputManager.h"
#include <string>
#include "MegamanLevelsManager.h"

/*
 *  Parametros
 */
struct OptionsMenuComponentParameters
{
    Camera *camera;
    EntityManager *entityManager;

    OptionsMenuComponentParameters(Camera *c, EntityManager *m) : camera(c), entityManager(m) {}
};
/*
 * Menu de Opciones. Maneja la gestion de los contoles
 */
struct OptionsMenuComponent : public Component
{
    static bool advancedIAEnabled;
private:
    // -------------------------------------------
    //  Menu text
    // -------------------------------------------
    const std::vector<std::string> controlsText = {"jump", "shoot", "dash", "weapon next", "weapon prev", "move left", "move right", "exit armor", "pause"};
    const std::vector<std::string> optionKeys = {"Jump", "Shoot", "Dash", "WeaponNext", "WeaponPrev", "MoveLeft", "MoveRight", "ExitArmor", "Pause"};

    const unsigned int NUM_CONTROLS = 9;
    const unsigned int NUM_OPTIONS = 14;
    // -------------------------------------------
    //  Aux variables
    // -------------------------------------------
    unsigned int cursorPosition = 0;
    bool inputEnabled = true; // Para gestionar solo un movimiento cada vez que le das
    bool waitingKey = false;
    bool waitingKey2 = false;
    

    // -------------------------------------------
    //  Cursor moving functions
    // -------------------------------------------
    void moveUp()
    {
        cursorPosition = (cursorPosition == 0) ? NUM_OPTIONS - 1 : cursorPosition - 1;
        inputEnabled = false;
    }
    void moveDown()
    {
        cursorPosition = (cursorPosition + 1) % NUM_OPTIONS;
        inputEnabled = false;
    }

    void moveRight()
    {
        if (NUM_OPTIONS - 4 == cursorPosition)
        {
            int volume = AudioManager::getInstance()->getMusicVolume();
            if (InputManager::isKeyHeld(SDL_SCANCODE_LCTRL))
                volume = std::min(volume + 10, 128);
            else
                volume = std::min(volume + 1, 128);
            AudioManager::getInstance()->setMusicVolume(volume);
        }

        if (NUM_OPTIONS - 3 == cursorPosition)
        {
            int volume = AudioManager::getInstance()->getSoundVolume();
            if (InputManager::isKeyHeld(SDL_SCANCODE_LCTRL))
                volume = std::min(volume + 10, 128);
            else
                volume = std::min(volume + 1, 128);
            AudioManager::getInstance()->setSoundVolume(volume);
        }

        if (NUM_OPTIONS - 5 == cursorPosition)
        {
            advancedIAEnabled = true;
        }

        inputEnabled = false;
    }

    void moveLeft()
    {

        if (NUM_OPTIONS - 4 == cursorPosition)
        {
            int volume = AudioManager::getInstance()->getMusicVolume();
            volume = std::max(volume - 1, 0);
            AudioManager::getInstance()->setMusicVolume(volume);
        }

        if (NUM_OPTIONS - 3 == cursorPosition)
        {
            int volume = AudioManager::getInstance()->getSoundVolume();
            volume = std::max(volume - 1, 0);
            AudioManager::getInstance()->setSoundVolume(volume);
        }

        if (NUM_OPTIONS - 5 == cursorPosition)
        {
            advancedIAEnabled = false;
        }

        inputEnabled = false;
    }
    // TODO
    void select()
    {
        inputEnabled = false;
        if (cursorPosition < NUM_CONTROLS)
            waitingKey = true;
        if (NUM_OPTIONS - 2 == cursorPosition)
        {
            DynamicInputManager::getInstance().clearBindings();
            DynamicInputManager::getInstance().loadFromJson("default_controls.config");
        }

        if (NUM_OPTIONS - 1 == cursorPosition)
        {
            DynamicInputManager::getInstance().saveToJson("controls.config");
            AudioManager::getInstance()->save();
            MegamanLevelsManager::getInstance()->loadLevel("mainMenu", entityManager->getSceneManager()->getSceneNameByEntityManager(entityManager));
        }
        if (cursorPosition == NUM_OPTIONS - 5)
        {
            advancedIAEnabled = !advancedIAEnabled;
        }
    }

public:
    /// -------------------------------------------
    //  Constructor
    // -------------------------------------------
    Camera *camera;
    EntityManager *entityManager;
    OptionsMenuComponent(OptionsMenuComponentParameters params) : camera(params.camera), entityManager(params.entityManager) {}

    float numtiempoSonido = 0.12f;
    bool puedeSubirVolumen = true;

    // -------------------------------------------
    //  Main functions
    // -------------------------------------------
    void start() override
    {
        // Cargar fuentes con tamaño 7x7
        RenderTextManager::getInstance().loadFont("goldTitle", "sprites/letters/goldTitle", 7, 7);
        RenderTextManager::getInstance().loadFont("selectedFont", "sprites/letters/lettersSelected", 7, 7);
        RenderTextManager::getInstance().loadFont("unselectedFont", "sprites/letters/lettersUnselected", 7, 7);

        DynamicInputManager::getInstance().loadFromJson("controls.config");
    }

    void update(float deltaTime) override
    {
        numtiempoSonido -= deltaTime;
        if (numtiempoSonido <= 0)
        {
            puedeSubirVolumen = true;
            numtiempoSonido = 0.12f;
        }
        else
        {
            puedeSubirVolumen = false;
        }

        if (InputManager::isKeyPressed(SDL_SCANCODE_W) && inputEnabled && !waitingKey)
        {
            moveUp();
        }
        if (InputManager::isKeyPressed(SDL_SCANCODE_S) && inputEnabled && !waitingKey)
        {
            moveDown();
        }
        if (InputManager::isKeyPressed(SDL_SCANCODE_D) && puedeSubirVolumen)
        {
            moveRight();
        }
        if (InputManager::isKeyPressed(SDL_SCANCODE_A) && puedeSubirVolumen)
        {
            moveLeft();
        }
        if (InputManager::isKeyPressed(SDL_SCANCODE_SPACE) && inputEnabled && !waitingKey)
        {
            select();
        }

        // Gestion inputenabled
        if (!InputManager::isKeyHeld(SDL_SCANCODE_W) &&
            !InputManager::isKeyHeld(SDL_SCANCODE_S) &&
            !InputManager::isKeyHeld(SDL_SCANCODE_A) &&
            !InputManager::isKeyHeld(SDL_SCANCODE_D) &&
            !InputManager::isKeyHeld(SDL_SCANCODE_SPACE))
        {
            inputEnabled = true; // Re-enable input when the key is released
        }

        SDL_Scancode key = InputManager::getLastKey();

        if (waitingKey && waitingKey2 && key != SDL_SCANCODE_UNKNOWN)
        {
            DynamicInputManager::getInstance().bindAction(optionKeys[cursorPosition], key);
            waitingKey = false;
            waitingKey2 = false;
            inputEnabled = false; // Re-enable input when the key is released
        }
        else if (waitingKey && key != SDL_SCANCODE_SPACE)
        {
            waitingKey2 = true;
        }
    }

    void render(EntityManager &entityManager, SDL_Renderer *renderer) override
    {
        // Variable auxiliar para gestionar seleccion del texto
        std::string mode;

        // ------------ Titulo del menu ---------- //
        RenderTextManager::getInstance().renderText("goldTitle", "option mode", 0.2, 0.05, 6, 6, &entityManager);

        // ------------ "key config" text ---------- //
        RenderTextManager::getInstance().renderText("unselectedFont", "key config", 0.35, 0.15, 3, 3, &entityManager);

        // ------------ Texto Controles ---------- //
        float inicioControles_x = 0.25f;
        float inicioControles_y = 0.2f;

        int i = 0;
        for (const auto &control : controlsText)
        {
            mode = (i == cursorPosition) ? "selectedFont" : "unselectedFont";
            float posY_control = inicioControles_y + i * 0.05; // Pequeña separacion
            RenderTextManager::getInstance().renderText(mode, control, inicioControles_x, posY_control, 3, 3, &entityManager);

            i++;
        }

        // ------------- Mappeo Controles ---------- //
        float inicioMapping_x = 0.55f;
        float inicioMapping_y = 0.2f;

        i = 0;
        for (const auto &mapKey : optionKeys)
        {
            float posY_mapping = inicioMapping_y + i * 0.05; // Pequeña separacion

            std::string value(SDL_GetScancodeName(DynamicInputManager::getInstance().getBind(mapKey)));
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c)
                           { return std::tolower(c); });

            if (i == cursorPosition && waitingKey)
                RenderTextManager::getInstance().renderText("selectedFont", "press any key", inicioMapping_x, posY_mapping, 3, 3, &entityManager);
            else
                RenderTextManager::getInstance().renderText("unselectedFont", value, inicioMapping_x, posY_mapping, 3, 3, &entityManager);
            i++;
        }
        // ------------- Nueva opción personalizada ---------- //
        mode = (NUM_OPTIONS - 5 == cursorPosition) ? "selectedFont" : "unselectedFont";
        RenderTextManager::getInstance().renderText(mode, "advanced ia", 0.25, 0.73, 3, 3, &entityManager);
        RenderTextManager::getInstance().renderText(
            "selectedFont",
            advancedIAEnabled ? "on" : "off", // Mostrar el estado
            0.6, 0.73, 3, 3, &entityManager);
        // ------------- Background Music ---------- //
        mode = (NUM_OPTIONS - 4 == cursorPosition) ? "selectedFont" : "unselectedFont";
        RenderTextManager::getInstance().renderText(mode, "music", 0.3, 0.78, 3, 3, &entityManager);
        RenderTextManager::getInstance().renderText("selectedFont", std::to_string(AudioManager::getInstance()->getMusicVolume()), 0.6, 0.78, 3, 3, &entityManager);

        // ------------- Sound effects ---------- //
        mode = (NUM_OPTIONS - 3 == cursorPosition) ? "selectedFont" : "unselectedFont";
        RenderTextManager::getInstance().renderText(mode, "sounds", 0.3, 0.83, 3, 3, &entityManager);
        RenderTextManager::getInstance().renderText("selectedFont", std::to_string(AudioManager::getInstance()->getSoundVolume()), 0.6, 0.83, 3, 3, &entityManager);

        // ------------- "exit" text ---------- //
        mode = (NUM_OPTIONS - 2 == cursorPosition) ? "selectedFont" : "unselectedFont";
        RenderTextManager::getInstance().renderText(mode, "reset controls", 0.32, 0.9, 3, 3, &entityManager);

        mode = (NUM_OPTIONS - 1 == cursorPosition) ? "selectedFont" : "unselectedFont";
        RenderTextManager::getInstance().renderText(mode, "exit", 0.45, 0.95, 3, 3, &entityManager);
    }
};

/*
 * Cargador de la clase
 */
class OptionsMenuComponentLoader
{
public:
    static OptionsMenuComponentParameters fromJSON(const nlohmann::json &j, EntityManager &entityManager)
    {
        std::shared_ptr<Camera> cameraPtr;
        entityManager.findComponent<Camera>(cameraPtr);
        Camera *camera = cameraPtr.get();
        if (!camera)
        {
            throw std::runtime_error("PauseMenuComponent requires a Camera component");
        }
        OptionsMenuComponentParameters params(camera, &entityManager);
        return params;
    }

    // Helper function to directly create a Camera component from JSON
    static OptionsMenuComponent createFromJSON(nlohmann::json &j, EntityManager &entityManager)
    {
        return OptionsMenuComponent(fromJSON(j, entityManager));
    }
};

bool OptionsMenuComponent::advancedIAEnabled = false; // Initialize static variable

#endif
