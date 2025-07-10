#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "EntityManager.h"
#include "Components.h"
#include "UIComponent.h"
#include "ParallaxLayer.h"
#include <SDL2/SDL.h>
#include "BVH.h"

/**
 * @brief Clase PhysicsSystem para manejar la lógica de física en las entidades.
 */
class PhysicsSystem {
public:
    /**
     * @brief Actualiza la física de las entidades gestionadas por EntityManager.
     * @param manager Instancia de EntityManager con las entidades a actualizar.
     * @param deltaTime Tiempo transcurrido desde la última actualización.
     */
    void update(EntityManager& manager, float deltaTime);
};

/**
 * @brief Clase CollisionSystem para detectar y manejar colisiones entre entidades.
 */
class CollisionSystem {
public:

    BVH* staticEntities;
    BVH* dynamicEntities;

    CollisionSystem()
    {
        staticEntities = nullptr;
        dynamicEntities = nullptr;
    }
    

    static void ResolveCollision(Collider* col1, Rigidbody* rb1, Transform* t1, Collider* col2, Rigidbody* rb2, Transform* t2, const CollisionResult& collisionResult);
    
    /**
     * @brief Actualiza las colisiones de las entidades gestionadas por EntityManager.
     * @param manager Instancia de EntityManager con las entidades a verificar.
     * @param deltaTime Tiempo transcurrido desde la última actualización.
     */
    void update(EntityManager& manager, float deltaTime);
};

/**
 * @brief Clase RenderSystem para renderizar entidades en pantalla.
 */
class RenderSystem {
public:
    /**
     * @brief Renderiza las entidades gestionadas por EntityManager usando un renderizador SDL.
     * @param manager Instancia de EntityManager con las entidades a renderizar.
     * @param renderer Puntero al renderizador SDL.
     */
    void render(EntityManager& manager, SDL_Renderer* renderer);

    void largest_4_3_resolution(int x, int y, int* out_width, int* out_height, int* excess_width, int* excess_height);
};

#endif