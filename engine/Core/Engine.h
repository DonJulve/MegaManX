#ifndef ENGINE_H
#define ENGINE_H

class Scene;

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "EntityManager.h"
#include "SceneManager.h"
#include "SceneLoader.h"
#include "Systems.h"
#include <string>
#include <iostream>

/**
 * @brief Clase Engine que representa el núcleo del motor del juego.
 */
class Engine {
private:
    bool running;                    /**< Indica si el motor está en ejecución. */
    SDL_Window* window;              /**< Ventana SDL del juego. */
    SDL_Renderer* renderer;          /**< Renderizador SDL asociado. */
    SceneManager sceneManager;       /**< Instancia de SceneManager para gestionar escenas. */
    int delayInputs;           /**< Tiempo de espera para entradas. */

    /**
     * @brief Constructor privado de Engine para evitar instancias externas.
     * @param title Título de la ventana.
     * @param width Ancho de la ventana.
     * @param height Alto de la ventana.
     */
    Engine(const char* title, int width, int height);

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    static Engine* instance;         /**< Puntero a la instancia única de Engine (singleton). */

public:
    /** @brief Destructor de Engine para liberar recursos. */
    ~Engine();

    /**
     * @brief Inicializa SDL, la ventana y el renderizador.
     * @param title Título de la ventana.
     * @param width Ancho de la ventana.
     * @param height Alto de la ventana.
     */
    void init(const char* title, int width, int height);

    /** @brief Maneja todos los eventos SDL (teclado, ratón, ventana, etc.). */
    void handleEvents();

    /** @brief Inicia el motor del juego. */
    void start();

    /**
     * @brief Actualiza los sistemas (física, colisiones, etc.).
     * @param deltaTime Tiempo transcurrido desde la última actualización.
     */
    void update(float deltaTime);

    /** @brief Renderiza todas las entidades y sistemas. */
    void render();

    /** @brief Limpia los recursos SDL utilizados. */
    void cleanUp();

    /** @brief Bucle principal del juego. */
    void run();

    void close() { running = false; } /**< Cierra el motor del juego. */

    /**
     * @brief Obtiene el renderizador SDL.
     * @return Puntero al renderizador SDL.
     */
    SDL_Renderer* getRenderer() { return renderer; }

    /**
     * @brief Obtiene el SceneManager.
     * @return Puntero a la instancia de SceneManager.
     */
    SceneManager* getSceneManager() { return &sceneManager; }

    /**
     * @brief Método estático para acceder a la instancia única de Engine (singleton).
     * @param title Título de la ventana (por defecto nullptr).
     * @param width Ancho de la ventana (por defecto 800).
     * @param height Alto de la ventana (por defecto 600).
     * @return Puntero a la instancia de Engine.
     */
    static Engine* getInstance(const char* title = nullptr, int width = 800, int height = 600);
};

#endif