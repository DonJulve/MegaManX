#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include <unordered_map>
#include <memory>
#include <vector>
#include <typeindex>
#include <set>
#include <SDL2/SDL.h>
#include "SceneManager.h"

/** @brief Alias para el tipo de identificador de entidad. */
using Entity = std::size_t;

class EntityManager;

/**
 * @brief Clase base para todos los componentes en el sistema ECS.
 */
class Component {
public:
    /** @brief La entidad a la que pertenece este componente. */
    Entity parent;

    /** @brief Destructor virtual para la limpieza adecuada de clases derivadas. */
    virtual ~Component() = default;

    /** @brief Se llama cuando el componente se inicializa. */
    virtual void start() {};

    /**
     * @brief Actualiza el estado del componente.
     * @param deltaTime Tiempo transcurrido desde la última actualización (en segundos).
     */
    virtual void update(float deltaTime) {};

    /**
     * @brief Actualiza el estado del componente.
     * @param deltaTime Tiempo transcurrido desde la última actualización (en segundos).
     */
    virtual void render(EntityManager& entityManager, SDL_Renderer* renderer) {};
};

/**
 * @brief Gestiona entidades y sus componentes asociados en el sistema ECS.
 */
class EntityManager {
private:
    /** @brief Mapa de entidades a sus componentes, indexado por tipo. */
    std::unordered_map<Entity, std::unordered_map<std::type_index, std::shared_ptr<Component>>> entities;

    /** @brief Conjunto de entidades actualmente activas. */
    std::set<Entity> activeEntities;

    /** @brief El próximo ID de entidad disponible. */
    Entity nextEntity = 0;

    /** @brief El último ID de entidad creado. */
    Entity lastEntity;

    /** @brief Conjunto de entidades marcadas para eliminación. */
    std::set<Entity> entitiesToDelete;

    

    SceneManager* sceneManager;
    

public:

    std::string scenaName;
    
    /** @brief Constructor de EntityManager con name . */
    EntityManager(const std::string& name) : scenaName(name), sceneManager(nullptr) {}
    

    void setSceneManager(SceneManager* sm){sceneManager = sm;}

    SceneManager* getSceneManager(){return sceneManager;}

    /**
     * @brief Crea una nueva entidad y devuelve su ID.
     * @return El ID de la entidad recién creada.
     */
    Entity createEntity();

    /**
     * @brief Marca una entidad para eliminación al final del marco.
     * @param entity La entidad a eliminar.
     */
    void deleteEntity(Entity entity);

    /**
     * @brief Agrega un componente a una entidad usando shared_ptr.
     * @param entity La entidad a la que se agrega el componente.
     * @param type El índice de tipo del componente.
     * @param component shared_ptr del componente a agregar.
     */
    void addComponent(Entity entity, std::type_index type, std::shared_ptr<Component> component);

    /**
     * @brief Agrega un componente a una entidad usando punteros estandar.
     * @param entity La entidad a la que se agrega el componente.
     * @param type El índice de tipo del componente.
     * @param component Puntero al componente a agregar.
     */
    void addComponent(Entity entity, std::type_index type, Component* component);

    /**
     * @brief Obtiene un componente de tipo T de una entidad.
     * @tparam T El tipo de componente a recuperar.
     * @param entity La entidad a consultar.
     * @return Puntero al componente si se encuentra, nullptr en caso contrario.
     */
    template <typename T>
    T* getComponent(Entity entity) const;

    /**
     * @brief Obtiene un componente de tipo T de una entidad.
     * @tparam T El tipo de componente a recuperar.
     * @param entity La entidad a consultar.
     * @return Puntero al componente si se encuentra, nullptr en caso contrario.
     */
    template <typename T>
    std::shared_ptr<T> getComponentShared(Entity entity) const;

    /**
     * @brief Obtiene el conjunto de todas las entidades activas.
     * @return Referencia al conjunto de entidades activas.
     */
    std::set<Entity>& getEntities();

    /**
     * @brief Obtiene el ID de la última entidad creada.
     * @return El último ID de entidad.
     */
    Entity getLast() const { return lastEntity; }

    /**
     * @brief Obtiene todos los componentes asociados con una entidad.
     * @param entity La entidad a consultar.
     * @return Referencia al mapa de componentes de la entidad.
     */
    std::unordered_map<std::type_index, std::shared_ptr<Component>>& getEntityComponents(Entity entity);

    /**
     * @brief Elimina todas las entidades marcadas para eliminación al final del marco.
     */
    void processDeletion();

    template <typename T>
    Entity findComponent(std::shared_ptr<T>& outComponent);

    bool hasComponentPointer(Component* ptr) const;
};

template <typename T>
T* EntityManager::getComponent(Entity entity) const {
    auto entityIt = entities.find(entity);
    if (entityIt != entities.end()) {
        auto& entityComponents = entityIt->second;
        auto componentIt = entityComponents.find(std::type_index(typeid(T)));
        if (componentIt != entityComponents.end()) {
            return static_cast<T*>(componentIt->second.get());
        }
    }
    return nullptr;
}

template <typename T>
std::shared_ptr<T> EntityManager::getComponentShared(Entity entity) const {
    auto entityIt = entities.find(entity);
    if (entityIt != entities.end()) {
        auto& entityComponents = entityIt->second;
        auto componentIt = entityComponents.find(std::type_index(typeid(T)));
        if (componentIt != entityComponents.end()) {
            return std::static_pointer_cast<T>(componentIt->second);
        }
    }
    return nullptr; // Implicitly converts to std::shared_ptr<T>
}

template <typename T>
Entity EntityManager::findComponent(std::shared_ptr<T>& outComponent) {
    for (const auto& [entity, components] : entities) {
        auto componentIt = components.find(std::type_index(typeid(T)));
        if (componentIt != components.end()) {
            outComponent = std::dynamic_pointer_cast<T>(componentIt->second); // Cast to the correct type
            return entity; // Return the entity that has this component
        }
    }
    outComponent = nullptr; // If not found, set to nullptr
    return 0; // Return an invalid entity (assuming 0 is invalid)
}


#endif
