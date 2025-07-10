#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <functional>
#include "Components.h"
#include <memory>  // Para std::shared_ptr
#include "Debug.h"
#include "EntityManager.h"
#include <nlohmann/json.hpp>
#pragma once

struct ParallaxLayerParameters {
    EntityManager* entityManager; // EntityManager instance
    float x = 0.0f;          // Base x position (optional, for offsets)
    float speed = 0.5f;      // Speed relative to camera (0.0 to 1.0)
                             // ¡IMPORTANTE! Para efecto parallax, usar valores MENORES que 1.0
};

struct ParallaxLayer : public Component {
    EntityManager* entityManager; // EntityManager instance
    SpriteRenderer* spriteRenderer = nullptr; // Pointer to SpriteRenderer component
    float x = 0.0f;          // Base x position (optional, for offsets)
    float speed = 0.5f;      // Speed relative to camera (0.0 to 1.0)
                             // ¡IMPORTANTE! Para efecto parallax, usar valores MENORES que 1.0
    int width = 0;           // Texture width
    int height = 0;          // Texture height
    bool initialized = false;
    float rotZ = 0.0f;       // Rotation in degrees
    float currentOffset = 0.0f; // Para acumular el desplazamiento de forma suave

    ParallaxLayer(ParallaxLayerParameters plp);
    
    void start() override;
    void render(EntityManager& entityManager, SDL_Renderer* renderer) override;
};

class ParallaxLayerLoader {
    public:
        /**
         * @brief Convierte un objeto JSON en una instancia de ParallaxLayerParameters.
         * @param j Objeto JSON con los datos.
         * @param entityManager Instancia de EntityManager.
         * @return Instancia de ParallaxLayerParameters generada desde JSON.
         */
        static ParallaxLayerParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager);
    
        /**
         * @brief Crea un componente ParallaxLayer directamente desde JSON.
         * @param j Objeto JSON con los datos.
         * @param entityManager Instancia de EntityManager.
         * @return Componente ParallaxLayer creado.
         */
        static ParallaxLayer createFromJSON(const nlohmann::json& j, EntityManager& entityManager);
    };