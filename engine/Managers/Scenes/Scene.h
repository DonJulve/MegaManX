#ifndef SCENE_H
#define SCENE_H

#include <memory> // Para std::shared_ptr
#include <string>
#include "EntityManager.h"
#include "Systems.h"
#include "Engine.h"
#include "SceneLoader.h"

/**
 * @brief Clase Scene que representa una escena en el juego.
 */
class Scene {
private:
    std::string name;              /**< Nombre de la escena. */
    bool active;                   /**< Indica si la escena está activa. */

    EntityManager entityManager;   /**< Instancia de EntityManager para gestionar entidades. */
    PhysicsSystem physicsSystem;   /**< Instancia de PhysicsSystem para manejar física. */
    CollisionSystem collisionSystem; /**< Instancia de CollisionSystem para detectar colisiones. */
    RenderSystem renderSystem;     /**< Instancia de RenderSystem para renderizar entidades. */

public:
    /**
     * @brief Constructor de Scene.
     * @param sceneName Nombre de la escena.
     */
    Scene(const std::string& sceneName) : name(sceneName), active(true), entityManager(sceneName) {}

    /**
     * @brief Carga una escena desde un archivo usando Engine.
     * @param filename Nombre del archivo de la escena.
     * @return Puntero a la escena cargada.
     */
    static Scene* loadFromFile(const std::string& filename);

    /** @brief Activa la escena. */
    void activate();

    /** @brief Desactiva la escena. */
    void deactivate();

    /**
     * @brief Verifica si la escena está activa.
     * @return Verdadero si la escena está activa, falso en caso contrario.
     */
    bool isActive();

    /**
     * @brief Obtiene el nombre de la escena.
     * @return Referencia constante al nombre de la escena.
     */
    const std::string& getName() const { return name; }

    /**
     * @brief Obtiene la instancia de EntityManager asociada a la escena.
     * @return Referencia a la instancia de EntityManager.
     */
    EntityManager& getEntityManager() { return entityManager; }

    void setupStaticColliders(std::vector<Collider*> colliders) {
        collisionSystem.staticEntities = new BVH(2);

        collisionSystem.staticEntities->Build(colliders);
    }

    /**
     * @brief Actualiza los sistemas de la escena.
     * @param deltaTime Tiempo transcurrido desde la última actualización.
     */
    void start();


    /**
     * @brief Actualiza los sistemas de la escena.
     * @param deltaTime Tiempo transcurrido desde la última actualización.
     */
    void update(float deltaTime);

    /** @brief Renderiza la escena y sus entidades. */
    void render();
};

#endif