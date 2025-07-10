#include "PrefabManager.h"
#include "SceneLoader.h"
#include <fstream>
#include <stdexcept>
#include "Debug.h"

void PrefabManager::loadPrefab(const std::string& prefabName) {

    std::string filePath = "resources/prefabs/" + prefabName + ".prefab";
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open prefab file: " + filePath);
    }

    json prefabData;

    try {
        file >> prefabData;
    } catch (const json::parse_error& e) {
        Debug::Log(std::string("JSON Prefab Parse Error: ") + e.what());
        throw std::runtime_error(std::string("JSON Prefab Parse Error: ") + e.what());
    } catch (const std::exception& e) {
        Debug::Log(std::string("Unexpected Error: ") + e.what());
        throw std::runtime_error(std::string("Unexpected Error: ") + e.what());
    }

    prefabTemplates[prefabName] = std::move(prefabData);
    Debug::Log("Loaded prefab: " + prefabName + " from " + filePath);
}

Entity PrefabManager::instantiatePrefab(const std::string& prefabName, EntityManager& entityManager) {
    auto it = prefabTemplates.find(prefabName);
    if (it == prefabTemplates.end()) {
        throw std::runtime_error("Prefab not found: " + prefabName);
    }

    return createEntityFromPrefab(it->second, entityManager);
}

Entity PrefabManager::createEntityFromPrefab(const json& prefabData, EntityManager& entityManager) {
    Entity entity = entityManager.createEntity();
    SceneLoader* sceneLoader = &(SceneLoader::getInstance());

    if (prefabData.contains("components")) {
        for (const auto& [componentName, componentData] : prefabData["components"].items()) {
            auto loaderIt = sceneLoader->componentLoaders.find(componentName);
            auto classIt = sceneLoader->componentsClasses.find(componentName);
            if (loaderIt == sceneLoader->componentLoaders.end()) {
                throw std::runtime_error("Component loader not found in prefab: " + componentName);
            }
            ComponentLoader* loader = loaderIt->second;
            if (!loader) {
                throw std::runtime_error("Invalid component loader for: " + componentName);
            }
            Component* rawComponent = loader->load(componentData, entityManager);
            std::shared_ptr<Component> component(rawComponent);

            entityManager.addComponent(entity, classIt->second, component);
            Debug::Log("Component " + componentName + " added to prefab entity " + std::to_string(entity));
        }
    }

    auto& components = entityManager.getEntityComponents(entity);

    for (auto& [type, component] : components) {
        if (component) {
            component->start();
        }
    }

    return entity;
}

bool PrefabManager::hasPrefab(const std::string& prefabName) const {
    return prefabTemplates.find(prefabName) != prefabTemplates.end();
}