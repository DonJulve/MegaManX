#include "SceneLoader.h"
#include <stdexcept>
#include <fstream>
#include "Scene.h"
#include "IAComponents.h"
#include "ReinforcementIAComponent.h"
#include "ParallaxLayer.h"
#include "Debug.h"

// ComponentLoader Methods
void ComponentLoader::setLoaderFunction(LoaderFunction func) {
    loaderFunc = std::move(func);
}

Component* ComponentLoader::load(const nlohmann::json& data, EntityManager& em) {
    return loaderFunc(data, em);
}

// SceneLoader Methods
SceneLoader::SceneLoader() {
    registerComponentLoader<Transform, TransformParameters>(
        "Transform",
        [](const nlohmann::json& j, EntityManager& entityManager) -> Transform* {
            return new Transform(TransformLoader::fromJSON(j, entityManager));
        }
    );

    registerComponentLoader<Rigidbody, RigidbodyParameters>(
        "Rigidbody",
        [](const nlohmann::json& j, EntityManager& entityManager) -> Rigidbody* {
            return new Rigidbody(RigidbodyLoader::fromJSON(j, entityManager));
        }
    );

    registerComponentLoader<Collider, ColliderParameters>(
        "Collider",
        [](const nlohmann::json& j, EntityManager& entityManager) -> Collider* {
            return new Collider(ColliderLoader::fromJSON(j, entityManager));
        }
    );

    registerComponentLoader<SpriteRenderer, SpriteRendererParameters>(
        "Sprite",
        [](const nlohmann::json& j, EntityManager& entityManager) -> SpriteRenderer* {
            return new SpriteRenderer(SpriteRendererLoader::fromJSON(j, entityManager));
        }
    );

    registerComponentLoader<Camera, CameraParameters>(
        "Camera",
        [](const nlohmann::json& j, EntityManager& entityManager) -> Camera* {
            return new Camera(CameraLoader::fromJSON(j, entityManager));
        }
    );

    registerComponentLoader<AnimatorComponent, AnimatorParameters>(
        "Animator",
        [](const nlohmann::json& j, EntityManager& entityManager) -> AnimatorComponent* {
            return new AnimatorComponent(AnimatorLoader::fromJSON(j, entityManager));
        }
    );

    registerComponentLoader<UIComponent, UIParameters>(
        "UIComponent",
        [](const nlohmann::json& j, EntityManager& entityManager) -> UIComponent* {
            return new UIComponent(UILoader::fromJSON(j, entityManager));
        }
    );

    registerComponentLoader<ClassicIAComponent, ClassicIAParameters>(
        "ClassicIA",
        [](const nlohmann::json& j, EntityManager& entityManager) -> ClassicIAComponent* {
            return new ClassicIAComponent(ClassicIALoader::fromJSON(j, entityManager));
        }
    );

    registerComponentLoader<ParallaxLayer, ParallaxLayerParameters>(
        "ParallaxLayer",
        [](const nlohmann::json& j, EntityManager& entityManager) -> ParallaxLayer* {
            return new ParallaxLayer(ParallaxLayerLoader::fromJSON(j, entityManager));
        }
    );

    registerComponentLoader<ReinforcementIAComponent, ReinforcementIAParameters>(
        "ReinforcementIA",
        [](const nlohmann::json& j, EntityManager& entityManager) -> ReinforcementIAComponent* {
            return new ReinforcementIAComponent(ReinforcementIALoader::fromJSON(j, entityManager));
        }
    );
}

Scene* SceneLoader::loadScene(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open scene file: " + filePath);
    }

    json sceneData;

    try {
        file >> sceneData;
    } catch (const json::parse_error& e) {
        Debug::Log(std::string("JSON Scene Parse Error: ") + e.what());
        throw std::runtime_error(std::string("JSON Scene Parse Error: ") + e.what());
    } catch (const std::exception& e) {
        Debug::Log(std::string("Unexpected Error: ") + e.what());
        throw std::runtime_error(std::string("Unexpected Error: ") + e.what());
    }

    std::string sceneName = sceneData["name"].get<std::string>();
    Scene* scene = new Scene(sceneName);
    EntityManager& entityManager = scene->getEntityManager();

    // Load entities, loading prefabs on-demand if needed
    if (sceneData.contains("entities")) {
        for (const auto& entityData : sceneData["entities"]) {
            Entity entity;

            if (entityData.contains("prefab")) {
                std::string prefabName = entityData["prefab"].get<std::string>();
                // Load prefab if not already loaded (assume prefab files are in "prefabs/" directory)
                if (!PrefabManager::getInstance().hasPrefab(prefabName)) {
                    PrefabManager::getInstance().loadPrefab(prefabName);
                }
                entity = PrefabManager::getInstance().instantiatePrefab(prefabName, entityManager);

                Transform* transform = entityManager.getComponent<Transform>(entity);
    
                transform->posX = entityData.value("posX", transform->posX);
                transform->posY = entityData.value("posY", transform->posY);

            } else {
                entity = entityManager.createEntity();
                if (entityData.contains("components")) {
                    for (const auto& [componentName, componentData] : entityData["components"].items()) {
                        auto loaderIt = componentLoaders.find(componentName);
                        auto classIt = componentsClasses.find(componentName);
                        if (loaderIt == componentLoaders.end()) {
                            throw std::runtime_error("Component loader not found: " + componentName);
                        }
                        ComponentLoader* loader = loaderIt->second;
                        if (!loader) {
                            throw std::runtime_error("Invalid component loader for: " + componentName);
                        }
                        Component* rawComponent = loader->load(componentData, entityManager);
                        std::shared_ptr<Component> component(rawComponent);
                        entityManager.addComponent(entity, classIt->second, component);
                        Debug::Log("Component " + componentName + " added to entity " + std::to_string(entity));
                    }
                }
            }
        }
    }

    return scene;
}