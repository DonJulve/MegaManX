#ifndef RAYCAST_MANAGER_H
#define RAYCAST_MANAGER_H

#include <vector>
#include <optional>
#include <SDL2/SDL.h>
#include "Components.h"
#include "EntityManager.h"
#include "Debug.h" // Para registro de depuración

/**
 * @struct Ray
 * @brief Representa un rayo con un origen y una dirección.
 */
struct Ray {
    SDL_FPoint origin;    /**< Punto de inicio del rayo (x, y). */
    SDL_FPoint direction; /**< Vector de dirección normalizado (x, y). */

    Ray(const SDL_FPoint& orig, const SDL_FPoint& dir);
};

/**
 * @struct RaycastHit
 * @brief Almacena información sobre una colisión con un rayo.
 */
struct RaycastHit {
    Collider* collider;     /**< El Collider que fue impactado. */
    SDL_FPoint point;       /**< Punto de intersección. */
    float distance;         /**< Distancia desde el origen del rayo hasta el punto de impacto. */
    Entity entity;          /**< La entidad propietaria del Collider impactado. */

    RaycastHit(Collider* c, const SDL_FPoint& p, float d, Entity e)
        : collider(c), point(p), distance(d), entity(e) {}
};

/**
 * @class RayCastManager
 * @brief Administra las operaciones de raycasting para detectar colisiones con Colliders.
 */
class RayCastManager {
public:
    /**
     * @brief Realiza un raycast y devuelve el primer Collider impactado con etiquetas especificadas.
     * @param ray El rayo a lanzar.
     * @param entityManager Referencia al EntityManager para acceder a los Colliders.
     * @param maxDistance Distancia máxima que el rayo debe recorrer (predeterminado: infinito).
     * @param tags Vector de etiquetas para filtrar impactos (predeterminado: vacío, impacta todo).
     * @return Un std::optional<RaycastHit> si se detecta una colisión con una etiqueta coincidente, std::nullopt en caso contrario.
     */
    static std::optional<RaycastHit> raycast(const Ray& ray, EntityManager* entityManager, 
                                             float maxDistance = std::numeric_limits<float>::infinity(), 
                                             const std::vector<std::string>& tags = {});

    /**
     * @brief Realiza un raycast y devuelve todos los Colliders impactados con etiquetas especificadas.
     * @param ray El rayo a lanzar.
     * @param entityManager Referencia al EntityManager para acceder a los Colliders.
     * @param maxDistance Distancia máxima que el rayo debe recorrer (predeterminado: infinito).
     * @param tags Vector de etiquetas para filtrar impactos (predeterminado: vacío, impacta todo).
     * @return Un vector de RaycastHits con todas las colisiones detectadas con etiquetas coincidentes.
     */
    static std::vector<RaycastHit> raycastAll(const Ray& ray, EntityManager* entityManager, 
                                              float maxDistance = std::numeric_limits<float>::infinity(), 
                                              const std::vector<std::string>& tags = {});

    /**
     * @brief Renderiza un rayo utilizando el SDL_Renderer proporcionado.
     * @param renderer El SDL_Renderer con el que se dibujará.
     * @param ray El rayo a renderizar.
     * @param length Longitud del rayo a dibujar (predeterminado: 1000).
     * @param color Color del rayo (predeterminado: blanco).
     */
    static void renderRay(SDL_Renderer* renderer, const Ray& ray,const Camera& camera, float length = 1000.0f, 
                          SDL_Color color = {255, 255, 255, 255});

private:
    /**
     * @brief Verifica si un rayo intersecta con un Rect y calcula el punto de impacto y la distancia.
     * @param ray El rayo a comprobar.
     * @param rect El cuadro delimitador contra el cual se realiza la prueba.
     * @param maxDistance Distancia máxima a considerar.
     * @param hitPoint Parámetro de salida para el punto de intersección.
     * @param distance Parámetro de salida para la distancia al impacto.
     * @return True si ocurre una intersección dentro de maxDistance, false en caso contrario.
     */
    static bool rayIntersectsRect(const Ray& ray, const Rect& rect, float maxDistance, SDL_FPoint& hitPoint, float& distance);

    static bool rayIntersectsTriangle(const Ray& ray, const Triangle& triangle, float maxDistance, SDL_FPoint& hitPoint, float& distance);

    static bool rayIntersectsSegment(const Ray& ray, const Vector2D& p1, const Vector2D& p2, float maxDistance, SDL_FPoint& hitPoint, float& distance);
};

#endif // RAYCAST_MANAGER_H
