#ifndef PREFAB_MANAGER_H
#define PREFAB_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <nlohmann/json.hpp>
#include "EntityManager.h"
#include "Scene.h"

using json = nlohmann::ordered_json;

class SceneLoader;

/**
 * @brief Clase para gestionar y cargar prefabs en el sistema.
 */
class PrefabManager {
public:

    /**
     * @brief Obtiene la instancia única de PrefabManager (singleton).
     * @return Referencia a la instancia de PrefabManager.
     */
    static PrefabManager& getInstance() {
        static PrefabManager instance;
        return instance;
    }

    /**
     * @brief Carga un prefab desde un archivo.
     * @param prefabName Nombre del prefab a cargar.
     */
    void loadPrefab(const std::string& prefabName);

    /**
     * @brief Instancia un prefab en el EntityManager proporcionado.
     * @param prefabName Nombre del prefab a instanciar.
     * @param entityManager Instancia de EntityManager donde se creará la entidad.
     * @return ID de la entidad creada.
     */
    Entity instantiatePrefab(const std::string& prefabName, EntityManager& entityManager);

    /**
     * @brief Verifica si un prefab está cargado.
     * @param prefabName Nombre del prefab a verificar.
     * @return Verdadero si el prefab existe, falso en caso contrario.
     */
    bool hasPrefab(const std::string& prefabName) const;

private:
    /**
     * @brief Constructor privado para el singleton.
     */
    PrefabManager() = default;

    PrefabManager(const PrefabManager&) = delete;
    PrefabManager& operator=(const PrefabManager&) = delete;

    /**
     * @brief Mapa de nombres de prefabs a sus datos JSON.
     */
    std::unordered_map<std::string, json> prefabTemplates;

    /**
     * @brief Crea una entidad a partir de los datos de un prefab.
     * @param prefabData Datos JSON del prefab.
     * @param entityManager Instancia de EntityManager donde se creará la entidad.
     * @return ID de la entidad creada.
     */
    Entity createEntityFromPrefab(const json& prefabData, EntityManager& entityManager);
};

#endif // PREFAB_MANAGER_H