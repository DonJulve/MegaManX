#include "SceneManager.h"
#include <iostream>
#include "Scene.h" 
#include "EntityManager.h"

std::string SceneManager::getSceneNameByEntityManager(const EntityManager* targetEntityManager) const {
    for (const auto& [sceneName, scenePtr] : scenes) {
        if (scenePtr && &(scenePtr->getEntityManager()) == targetEntityManager) {
            return sceneName;
        }
    }
    return ""; // Return empty string if no match is found
}

void SceneManager::addScene(const std::string& name) {
    if (isSceneLoaded(name)) {
        std::cerr << "Scene " << name << " is already loaded!" << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(scenesMutex);

    scenes.emplace(name,Scene::loadFromFile("resources/scenes/" + name + ".scene"));

    scenes[name]->getEntityManager().setSceneManager(this);
    pendingStarts.push_back(name);
}

bool SceneManager::isSceneLoaded(const std::string& name) const {
    return scenes.find(name) != scenes.end();
}

bool SceneManager::isSceneLoading(const std::string& name) const {
    auto it = loadingFlags.find(name);
    return it != loadingFlags.end() && it->second;
}

void SceneManager::loadSceneAsync(const std::string& name) {
    if (isSceneLoaded(name)) {
        std::cerr << "Scene " << name << " is already loaded!" << std::endl;
        return;
    }

    // Set loading flag
    loadingFlags[name] = true;

    // Launch async loading
    pendingLoads.push_back(std::async(std::launch::async, [this, name]() {
        std::lock_guard<std::mutex> lock(scenesMutex);
        scenes.emplace(name, Scene::loadFromFile("resources/scenes/" + name + ".scene"));
        loadingFlags[name] = false; // Mark as complete when done
        scenes[name]->getEntityManager().setSceneManager(this);
        pendingStarts.push_back(name);
    }));
}

Scene* SceneManager::getScene(const std::string& name)
{
    if (!isSceneLoaded(name)) {
        return nullptr;
    }

    return scenes[name];
}

void SceneManager::removeScene(const std::string& name) {
    if (!isSceneLoaded(name)) {
        std::cerr << "Scene " << name << " is not loaded!" << std::endl;
        return;
    }
    
    pendingRemoves.push_back(name);
}

void SceneManager::update(float deltaTime) {

    deltaTime = deltaTime - delayInputs;

    int time1 = SDL_GetTicks();

    // Remove scenes marked for deletion
    for (std::string& name : pendingRemoves) {
        scenes.erase(name);
    }
    pendingRemoves.clear();

    // Clean up completed futures
    pendingLoads.erase(
        std::remove_if(pendingLoads.begin(), pendingLoads.end(),
            [](std::future<void>& f) {
                bool isReady = f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                return isReady;
            }),
        pendingLoads.end()
    );

    // Lock and update scenes
    std::lock_guard<std::mutex> lock(scenesMutex);



    for (std::string& name : pendingStarts) {
        scenes[name]->start();
    }

    pendingStarts.clear();

    int time2 = SDL_GetTicks();

    delayInputs = (time2 - time1) / 1000.0f;  // Calculate delay for input processing

    //deltaTime = deltaTime - delayInputs;  // Adjust deltaTime based on input delay

    //Debug::Log("Delay for inputs: " + std::to_string(time2 - time1) + " ticks");

    for (auto& [name, scene] : scenes) {
        scene->update(deltaTime);
    }

}

void SceneManager::render() {
    std::lock_guard<std::mutex> lock(scenesMutex);
    for (auto& [name, scene] : scenes) {
        scene->render();
    }
}
