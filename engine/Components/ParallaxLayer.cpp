#include "ParallaxLayer.h"
#include <SDL2/SDL_image.h>
#include <string>
#include <iostream>
#include <cmath>

ParallaxLayer::ParallaxLayer(ParallaxLayerParameters plp) : 
    entityManager(plp.entityManager), 
    x(plp.x), 
    speed(plp.speed),
    rotZ(0.0f),
    currentOffset(0.0f) {
}

const float offset = 1000.0f; // Offset para el ancho de la textura

void ParallaxLayer::start() {
    spriteRenderer = entityManager->getComponent<SpriteRenderer>(this->parent);
    
    if (spriteRenderer) {
        width = spriteRenderer->width + offset;
        height = spriteRenderer->height;
        initialized = true;
    } else {
        Debug::Log("ParallaxLayer: No SpriteRenderer component found!");
    }
    
    // Asegúrate de que el valor speed es apropiado para el efecto parallax
    // Para un verdadero efecto parallax, speed debe ser menor que 1.0
    // Speed = 0: fondo estático
    // Speed = 0.2-0.5: efecto parallax para fondos lejanos
    // Speed = 0.6-0.9: efecto parallax para elementos intermedios
    // Speed = 1.0: mismo movimiento que la cámara (sin efecto parallax)
    if (speed == 1.0f) {
        Debug::Log("ParallaxLayer: Warning - Speed=1.0 no producirá efecto parallax visible");
    } else if (speed > 1.0f) {
        Debug::Log("ParallaxLayer: Warning - Speed>1.0 producirá un efecto parallax 'invertido'");
    }
}

void ParallaxLayer::render(EntityManager& entityManager, SDL_Renderer* renderer) {
    // 1. Verificar los componentes necesarios
    if (!spriteRenderer || !spriteRenderer->texture) return;

    // 2. Obtener la cámara
    Camera* camera = nullptr;
    for (Entity entity : entityManager.getEntities()) {
        camera = entityManager.getComponent<Camera>(entity);
        if (camera) break;
    }
    if (!camera) return;

    // 3. Asegurarnos de que tenemos las dimensiones de la textura
    if (width == 0 && height == 0) {
        width = spriteRenderer->width + offset;
        height = spriteRenderer->height;
    }

    // CLAVE: Para el efecto parallax, multiplicamos por un factor menor a 1.0 para que 
    // se mueva más lentamente que la cámara. El valor speed controla ese factor.
    // - Si speed = 0.0: La capa no se mueve (fondo estático)
    // - Si speed = 0.5: La capa se mueve a la mitad de velocidad que la cámara (efecto parallax medio)
    // - Si speed = 1.0: La capa se mueve a la misma velocidad que la cámara (sin efecto parallax)
    
    // 4. Calcular el offset real (multiplicamos por un factor menor a 1 para efecto parallax)
    float trueOffset = camera->getPosX() * speed;
    
    // 5. Calcular el offset visual (para desplazar la textura dentro de su ancho)
    float visualOffset = fmodf(trueOffset, width);
    if (visualOffset < 0) visualOffset += width;
    
    // 6. Definir rectángulo fuente (imagen completa)
    SDL_Rect srcRect = {0, 0, width, height};
    
    // 7. Calcular cuántas copias necesitamos para cubrir toda la pantalla
    int tilesNeeded = ceil(camera->windowWidth / (float)width) + 2;
    
    // 8. Calcular la escala para que ocupe toda la altura de la ventana
    float scaleY = (float)camera->windowHeight / height;
    int scaledHeight = camera->windowHeight;
    
    // 9. Renderizar las imágenes en mosaico
    for (int i = 0; i < tilesNeeded; i++) {
        // Posición X de esta copia, considerando el offset de parallax
        int tileX = (i * width) - visualOffset + camera->excessWidth / 2;
        
        // Rectángulo destino para esta copia (ajustando la altura para llenar la pantalla)
        SDL_Rect destRect = {
            tileX,                  // Posición X con parallax
            0,                      // Alineado con la parte superior de la ventana
            width,                  // Ancho original (sin estirar)
            scaledHeight            // Alto escalado para llenar la ventana
        };
        
        // Renderizar esta copia
        SDL_RenderCopyEx(
            renderer, 
            spriteRenderer->texture, 
            &srcRect, 
            &destRect, 
            rotZ,     // Rotación
            nullptr,  // Sin punto de rotación específico (centro por defecto)
            spriteRenderer->flip
        );
    }
}

ParallaxLayerParameters ParallaxLayerLoader::fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
    ParallaxLayerParameters params;

    params.x = j.value("x", 0.0f);
    params.speed = j.value("speed", 0.5f);  // Valor por defecto cambiado a 0.5 para asegurar efecto parallax
    params.entityManager = &entityManager;
    return params;
}

ParallaxLayer ParallaxLayerLoader::createFromJSON(const nlohmann::json& j, EntityManager& entityManager) {
    return ParallaxLayer(fromJSON(j, entityManager));
}