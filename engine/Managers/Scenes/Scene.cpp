#include "Scene.h"
#include "Debug.h"


void Scene::activate() {active=true;}
void Scene::deactivate() {active=false;}
bool Scene::isActive() {return active;}

void Scene::start(){
    std::vector<Collider*> colliders;

    for (auto& entity : getEntityManager().getEntities()) {
        // Retrieve the component map for the current entity
        auto& componentMap = getEntityManager().getEntityComponents(entity);

        // Iterate over all components of the entity
        for (auto& [typeIndex, componentPtr] : componentMap) {
            if (componentPtr) {
                componentPtr->start();  // Call the start method of the component
            }
        }

        Collider* collider = getEntityManager().getComponent<Collider>(entity);
        if (collider) {
            if(collider->getTag()=="Terrain") colliders.push_back(collider);
        }
    }

    setupStaticColliders(colliders);
}

void Scene::update(float deltaTime) {

    if(!isActive()) return;

    // Update each active entity in the entityManager
    for (auto& entity : entityManager.getEntities()) {
        // Retrieve the component map for the current entity
        auto& componentMap = entityManager.getEntityComponents(entity);

        // Iterate over all components of the entity
        for (auto& [typeIndex, componentPtr] : componentMap) {
            if(componentPtr) {
                componentPtr->update(deltaTime);  // Call the update method of the component
            }
        }
    }
    physicsSystem.update(entityManager, deltaTime);
    collisionSystem.update(entityManager, deltaTime);

    entityManager.processDeletion();
}

void Scene::render() {
    if(!isActive()) return;
    
    renderSystem.render(entityManager,Engine::getInstance()->getRenderer());

    for (auto& entity : getEntityManager().getEntities()) {
        // Retrieve the component map for the current entity
        auto& componentMap = getEntityManager().getEntityComponents(entity);

        // Iterate over all components of the entity
        for (auto& [typeIndex, componentPtr] : componentMap) {
            if (componentPtr) {
                // Skip UIComponent and ParallaxLayer to avoid duplicate rendering
                if (typeIndex == std::type_index(typeid(UIComponent)) || 
                    typeIndex == std::type_index(typeid(ParallaxLayer))) {
                    continue;
                }
                componentPtr->render(entityManager, Engine::getInstance()->getRenderer());
            }
        }
    }
}

Scene* Scene::loadFromFile(const std::string& filename) {
    // Make sure Engine is properly initialized to call the sceneManager
    return SceneLoader::getInstance().loadScene(filename);
}