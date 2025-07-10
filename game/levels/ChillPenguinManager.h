#include <string>
#include <stdexcept>
#include "EntityManager.h"
#include "Components.h"
#include "megaman/MegamanController.h"
#include "megaman/MegamanCamera.h"
#include "AudioManager.h"
#include "Debug.h"
#pragma once

struct ChillPenguinManagerParameters {
    EntityManager* entityManager;
};

class ChillPenguinManager : public Component {
    private:
        EntityManager* entityManager;
        std::shared_ptr<MegamanController> megamanController;
        std::shared_ptr<MegamanCamera> megaCamera;
    
    public:
        
    
    ChillPenguinManager(ChillPenguinManagerParameters params) 
            : entityManager(params.entityManager) {
            if (!entityManager) {
                throw std::runtime_error("ChillPenguinManager requires a valid EntityManager");
            }
    
        }
    
        void start() override {
    
            // Initialize Mega Man's transform
            entityManager->findComponent<MegamanController>(megamanController);
            entityManager->findComponent<MegamanCamera>(megaCamera);
            if (!megaCamera || !megamanController) {
                throw std::runtime_error("ChillPenguinManager requires a MegamanCamera component on MegaMan's entity");
            }


            AudioManager::getInstance()->loadMusic("chillPenguin", "music/12. Chill Penguin.mp3");
            AudioManager::getInstance()->playMusic("chillPenguin", -1, 0);

            Debug::Log("Playing Chill Penguin music");

        }
    
        void update(float deltaTime) override {

            
        }
    
    private:
        
    };

    class ChillPenguinManagerLoader {
        public:
    
            static ChillPenguinManagerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
    
                ChillPenguinManagerParameters params;

                params.entityManager = &entityManager;
        
                return params;
            }
        
            // Helper function to directly create a MegamanControlelr component from JSON
            static ChillPenguinManager createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
                return ChillPenguinManager(fromJSON(j, entityManager));
            }
    };
