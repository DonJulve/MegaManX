#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <SDL2/SDL.h>
#include "Components.h"
#include <unordered_map>
#include <array>

/**
 * @brief Clase InputManager para gestionar la entrada del usuario (teclado y ratón).
 */
class InputManager {
public:
    /**
     * @brief Maneja eventos SDL relacionados con la entrada del ratón.
     * @param event Evento SDL a procesar.
     */
    static void handleEvents(SDL_Event& event);

    /** @brief Procesa el estado del teclado en tiempo real. */
    static void handleKeyboard();

    /**
     * @brief Verifica si una tecla está presionada en el frame actual.
     * @param key Código de la tecla SDL (SDL_Scancode).
     * @return Verdadero si la tecla está presionada, falso en caso contrario.
     */
    static bool isKeyPressed(SDL_Scancode key);

    /**
     * @brief Verifica si una tecla está mantenida presionada.
     * @param key Código de la tecla SDL (SDL_Scancode).
     * @return Verdadero si la tecla está mantenida, falso en caso contrario.
     */
    static bool isKeyHeld(SDL_Scancode key);

    /**
     * @brief Verifica si una tecla ha sido soltada en el frame actual.
     * @param key Código de la tecla SDL (SDL_Scancode).
     * @return Verdadero si la tecla fue soltada, falso en caso contrario.
     */
    static bool isKeyReleased(SDL_Scancode key);

    /**
     * @brief Verifica si un botón del ratón está presionado en el frame actual.
     * @param button Identificador del botón del ratón (Uint8).
     * @return Verdadero si el botón está presionado, falso en caso contrario.
     */
    static bool isMouseButtonPressed(Uint8 button);

    /**
     * @brief Verifica si un botón del ratón está mantenido presionado.
     * @param button Identificador del botón del ratón (Uint8).
     * @return Verdadero si el botón está mantenido, falso en caso contrario.
     */
    static bool isMouseButtonHeld(Uint8 button);

    /**
     * @brief Verifica si un botón del ratón ha sido soltado en el frame actual.
     * @param button Identificador del botón del ratón (Uint8).
     * @return Verdadero si el botón fue soltado, falso en caso contrario.
     */
    static bool isMouseButtonReleased(Uint8 button);

    /**
     * @brief Obtiene la posición X del ratón.
     * @return Coordenada X del ratón.
     */
    static int getMouseX();

    /**
     * @brief Obtiene la posición Y del ratón.
     * @return Coordenada Y del ratón.
     */
    static int getMouseY();

    static int getScrollWheelX();
    
    static int getScrollWheelY();
    

    /** @brief Guarda el estado previo de las entradas. */
    static void updatePreviousState();

    /** @brief Reinicia el estado de las entradas. */
    static void resetInputState();

    static SDL_FPoint screenToWorld(float mouseX, float mouseY, Camera& camera);

    static SDL_FPoint worldToScreen(float worldX, float worldY, Camera& camera);

    static void resetLastKey();
    static SDL_Scancode getLastKey();

private:
    static std::array<Uint8, SDL_NUM_SCANCODES> keyStates;         /**< Estados actuales de las teclas. */
    static std::array<Uint8, SDL_NUM_SCANCODES> prevKeyStates;     /**< Estados previos de las teclas. */

    static std::unordered_map<Uint8, bool> mouseButtonStates;      /**< Estados actuales de los botones del ratón. */
    static std::unordered_map<Uint8, bool> prevMouseButtonStates;  /**< Estados previos de los botones del ratón. */

    static int mouseX, mouseY;                                     /**< Coordenadas actuales del ratón. */
    static int scrollWheelX, scrollWheelY;

    static SDL_Scancode lastKeyPressed;
};

#endif