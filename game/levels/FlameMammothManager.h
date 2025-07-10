#include <string>
#include <stdexcept>
#include "EntityManager.h"
#include "Components.h"
#include "megaman/MegamanController.h"
#include "megaman/MegamanCamera.h"
#include "AudioManager.h"
#include "Debug.h"
#pragma once

struct FlameMammothManagerParameters {
    EntityManager* entityManager;
};

class FlameMammothManager : public Component {
    private:
        EntityManager* entityManager;
        std::shared_ptr<MegamanController> megamanController;
        std::shared_ptr<MegamanCamera> megaCamera;
    
    public:
        
    
    FlameMammothManager(FlameMammothManagerParameters params) 
            : entityManager(params.entityManager) {
            if (!entityManager) {
                throw std::runtime_error("FlameMammothManager requires a valid EntityManager");
            }
    
        }
    
        void start() override {
    
            // Initialize Mega Man's transform
            entityManager->findComponent<MegamanController>(megamanController);
            entityManager->findComponent<MegamanCamera>(megaCamera);
            if (!megaCamera || !megamanController) {
                throw std::runtime_error("FlameMammothManager requires a MegamanCamera component on MegaMan's entity");
            }


            AudioManager::getInstance()->loadMusic("FlameMammoth", "music/13. Flame Mammoth.mp3");
            AudioManager::getInstance()->playMusic("FlameMammoth", -1, 0);

            Debug::Log("Playing Flame Mammoth music");

        }
    
        void update(float deltaTime) override {

            
        }
    
    private:
        
    };

    class FlameMammothManagerLoader {
        public:
    
            static FlameMammothManagerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
    
                FlameMammothManagerParameters params;

                params.entityManager = &entityManager;
        
                return params;
            }
        
            // Helper function to directly create a MegamanControlelr component from JSON
            static FlameMammothManager createFromJSON( nlohmann::json& j, EntityManager& entityManager) {
                return FlameMammothManager(fromJSON(j, entityManager));
            }
    };
