#ifndef SCENELOADER_H
#define SCENELOADER_H

class Scene;
class PrefabManager;


#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include "EntityManager.h"
#include <nlohmann/json.hpp>
#include "Components.h"
#include "AnimatorComponent.h"
#include <utility>
#include <typeindex>
#include <fstream>
#include "PrefabManager.h"

using json = nlohmann::ordered_json;


/**
 * @brief Clase ComponentLoader para cargar componentes desde datos JSON.
 */
class ComponentLoader {
public:
    using LoaderFunction = std::function<Component*(const nlohmann::json&, EntityManager&)>; /**< Tipo de función para cargar componentes. */

    /**
     * @brief Establece la función de carga para el ComponentLoader.
     * @param func Función de carga a asignar.
     */
    void setLoaderFunction(LoaderFunction func);

    /**
     * @brief Carga un componente desde datos JSON.
     * @param data Datos JSON del componente.
     * @param em Instancia de EntityManager.
     * @return Puntero al componente cargado.
     */
    Component* load(const nlohmann::json& data, EntityManager& em);

private:
    LoaderFunction loaderFunc; /**< Función de carga asignada al ComponentLoader. */
};

/**
 * @brief Clase SceneLoader para cargar escenas desde archivos JSON.
 */
class SceneLoader {
public:
    SceneLoader(const SceneLoader&) = delete;
    SceneLoader& operator=(const SceneLoader&) = delete;

    /**
     * @brief Obtiene la instancia única de SceneLoader (singleton).
     * @return Referencia a la instancia de SceneLoader.
     */
    static SceneLoader& getInstance() {
        static SceneLoader instance;
        return instance;
    }

    /**
     * @brief Carga una escena desde un archivo.
     * @param filePath Ruta del archivo JSON de la escena.
     * @return Puntero a la escena cargada.
     */
    Scene* loadScene(const std::string& filePath);

    /**
     * @brief Registra una función de carga para un componente específico.
     * @tparam T Tipo del componente.
     * @tparam T2 Tipo de retorno de la función de carga.
     * @param componentName Nombre del componente.
     * @param loaderFunc Función de carga para el componente.
     */
    template<typename T, typename T2>
    void registerComponentLoader(
        const std::string& componentName,
        std::function<Component*(const nlohmann::json&, EntityManager&)> loaderFunc) {

        ComponentLoader* loader = new ComponentLoader();

        loader->setLoaderFunction([loaderFunc](const nlohmann::json& data, EntityManager& em) -> Component* {
            return loaderFunc(data, em); 
        });
        componentsClasses.emplace(componentName, std::type_index(typeid(T)));
        componentLoaders[componentName] = loader;
    }

    /**
     * @brief Carga un componente usando su nombre y datos JSON.
     * @tparam T Tipo del componente.
     * @tparam T2 Tipo del componente cargado.
     * @tparam Args Tipos de los argumentos adicionales.
     * @param componentName Nombre del componente.
     * @param componentData Datos JSON del componente.
     * @param args Argumentos adicionales para la carga.
     * @return Instancia del componente cargado.
     */
    template<typename T, typename T2, typename... Args>
    T2* loadComponent(const std::string& componentName, const json& componentData, Args&&... args) {
        auto loaderIt = componentLoaders.find(componentName);
        if (loaderIt == componentLoaders.end()) {
            throw std::runtime_error("Cargador de componente no encontrado: " + componentName);
        }

        ComponentLoader* loader = loaderIt->second;
        if (!loader) {
            throw std::runtime_error("Tipo de cargador de componente inválido para: " + componentName);
        }

        return static_cast<T2*>(loader->load(componentData, std::forward<Args>(args)...));
    }

private:
    friend class PrefabManager;

    /** @brief Constructor privado de SceneLoader para el singleton. */
    SceneLoader();

    std::unordered_map<std::string, std::type_index> componentsClasses; /**< Mapa de nombres de componentes a sus índices de tipo. */
    std::unordered_map<std::string, ComponentLoader*> componentLoaders;  /**< Mapa de nombres de componentes a sus cargadores. */
};

#endif