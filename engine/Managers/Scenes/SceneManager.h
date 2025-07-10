#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

class Scene;
class EntityManager;

#include <string>
#include <unordered_map>
#include <vector>
#include <future>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <memory>
#include <mutex>

/**
 * @brief Clase SceneManager para gestionar las escenas del juego.
 */
class SceneManager {
private:
    std::unordered_map<std::string, Scene*> scenes; /**< Mapa de nombres de escenas a punteros de Scene. */
    std::unordered_map<std::string, bool> loadingFlags; // Tracks loading status
    std::vector<std::future<void>> pendingLoads;        // Stores active async operations
    std::vector<std::string> pendingStarts;        // Stores active async operations
    std::vector<std::string> pendingRemoves;        // Stores active async operations
    float delayInputs = 0.0f; // Delay for input processing
    mutable std::mutex scenesMutex;

public:

    std::string getSceneNameByEntityManager(const EntityManager* targetEntityManager) const;

    /**
     * @brief Agrega una escena al SceneManager.
     * @param name Nombre de la escena a agregar.
     */
    void addScene(const std::string& name);

    /**
     * @brief Verifica si una escena ya está cargada.
     * @param name Nombre de la escena a verificar.
     * @return Verdadero si la escena está cargada, falso en caso contrario.
     */
    bool isSceneLoaded(const std::string& name) const;

    /**
     * @brief Verifica si una escena está siendo cargada.
     * @param name Nombre de la escena a verificar.
     * @return Verdadero si la escena está siendo cargada, falso en caso contrario.
     */
    bool isSceneLoading(const std::string& name) const;

    /**
     * @brief Carga una escena de forma asíncrona.
     * @param name Nombre de la escena a cargar.
     */
    void loadSceneAsync(const std::string& name);

    /**
     * @brief Obtiene una escena por su nombre.
     * @param name Nombre de la escena.
     * @return Puntero a la instancia de Scene, o nullptr si no existe.
     */
    Scene* getScene(const std::string& name);

    /**
     * @brief Obtiene el mapa de todas las escenas cargadas.
     * @return Puntero al mapa de escenas.
     */
    std::unordered_map<std::string, Scene*>* getScenes() { return &scenes; }

    /**
     * @brief Elimina una escena del SceneManager.
     * @param name Nombre de la escena a eliminar.
     */
    void removeScene(const std::string& name);

    /**
     * @brief Actualiza todas las escenas activas.
     * @param deltaTime Tiempo transcurrido desde la última actualización.
     */
    void update(float deltaTime);

    /** @brief Renderiza todas las escenas activas. */
    void render();
};

#endif