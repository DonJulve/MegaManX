#include "RaycastManager.h"
#include <algorithm>
#include <cmath>
#include <limits>

Ray::Ray(const SDL_FPoint& orig, const SDL_FPoint& dir) : origin(orig) {
    // Normalize the direction vector
    float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (length > 0) {
        direction.x = dir.x / length;
        direction.y = dir.y / length;
    } else {
        direction = {0, 0}; // Default to no direction if zero vector
        //Debug::Log("Ray: Warning - Direction vector is zero, defaulting to (0, 0).");
    }
}

std::optional<RaycastHit> RayCastManager::raycast(const Ray& ray, EntityManager* entityManager, float maxDistance, const std::vector<std::string>& tags) {

    std::optional<RaycastHit> closestHit = std::nullopt;
    float closestDistance = maxDistance;

    for (const Entity entity : entityManager->getEntities()) {
        Collider* collider = entityManager->getComponent<Collider>(entity);
        if (!collider || collider->isTriggerCol()) continue; // Skip triggers

        // Check if collider's tag matches any specified tags (if tags are provided)
        if (!tags.empty() && std::find(tags.begin(), tags.end(), collider->getTag()) == tags.end()) {
            continue; // Skip if no tag match
        }

        SDL_FPoint hitPoint;
        float distance;
        if(collider->checkBottom&& collider->checkLeft&&collider->checkRight&&collider->checkTop){
            if (rayIntersectsRect(ray, collider->getBoundingBox(), maxDistance, hitPoint, distance) && distance < closestDistance) {
                closestHit = RaycastHit(collider, hitPoint, distance, entity);
                closestDistance = distance;
            }
        }else
        {
            if(rayIntersectsTriangle(ray, collider->topTriangle, maxDistance, hitPoint, distance) && distance < closestDistance){
                closestHit = RaycastHit(collider, hitPoint, distance, entity);
                closestDistance = distance;
            }

            if(rayIntersectsTriangle(ray, collider->leftTriangle, maxDistance, hitPoint, distance) && distance < closestDistance){
                closestHit = RaycastHit(collider, hitPoint, distance, entity);
                closestDistance = distance;
            }

            if(rayIntersectsTriangle(ray, collider->rightTriangle, maxDistance, hitPoint, distance) && distance < closestDistance){
                closestHit = RaycastHit(collider, hitPoint, distance, entity);
                closestDistance = distance;
            }

            if(rayIntersectsTriangle(ray, collider->bottomTriangle, maxDistance, hitPoint, distance) && distance < closestDistance){
                closestHit = RaycastHit(collider, hitPoint, distance, entity);
                closestDistance = distance;
            }
        }
    }
    return closestHit;
}

std::vector<RaycastHit> RayCastManager::raycastAll(const Ray& ray, EntityManager* entityManager, float maxDistance, const std::vector<std::string>& tags) {
    std::vector<RaycastHit> hits;

    for (const Entity entity : entityManager->getEntities()) {
        Collider* collider = entityManager->getComponent<Collider>(entity);
        if (!collider || collider->isTriggerCol()) continue; // Skip triggers

        // Check if collider's tag matches any specified tags (if tags are provided)
        if (!tags.empty() && std::find(tags.begin(), tags.end(), collider->getTag()) == tags.end()) {
            continue; // Skip if no tag match
        }

        SDL_FPoint hitPoint;
        float distance;
        if (rayIntersectsRect(ray, collider->getBoundingBox(), maxDistance, hitPoint, distance)) {
            hits.emplace_back(collider, hitPoint, distance, entity);
        }
    }

    // Sort hits by distance (closest first)
    std::sort(hits.begin(), hits.end(), [](const RaycastHit& a, const RaycastHit& b) {
        return a.distance < b.distance;
    });

    return hits;
}

bool RayCastManager::rayIntersectsRect(const Ray& ray, const Rect& rect, float maxDistance, SDL_FPoint& hitPoint, float& distance) {
    // Convert rect to float coordinates
    float left = static_cast<float>(rect.x);
    float right = static_cast<float>(rect.x + rect.w);
    float top = static_cast<float>(rect.y);
    float bottom = static_cast<float>(rect.y + rect.h);

    float tMin = 0.0f;
    float tMax = maxDistance;

    // Check X axis
    if (std::abs(ray.direction.x) < 1e-6f) { // Ray parallel to Y axis
        if (ray.origin.x < left || ray.origin.x > right) return false;
    } else {
        float t1 = (left - ray.origin.x) / ray.direction.x;
        float t2 = (right - ray.origin.x) / ray.direction.x;
        tMin = std::max(tMin, std::min(t1, t2));
        tMax = std::min(tMax, std::max(t1, t2));
        if (tMin > tMax) return false;
    }

    // Check Y axis
    if (std::abs(ray.direction.y) < 1e-6f) { // Ray parallel to X axis
        if (ray.origin.y < top || ray.origin.y > bottom) return false;
    } else {
        float t1 = (top - ray.origin.y) / ray.direction.y;
        float t2 = (bottom - ray.origin.y) / ray.direction.y;
        tMin = std::max(tMin, std::min(t1, t2));
        tMax = std::min(tMax, std::max(t1, t2));
        if (tMin > tMax) return false;
    }

    if (tMin < 0 || tMin > maxDistance) return false; // Hit is behind ray origin or beyond max distance

    // Calculate hit point and distance
    distance = tMin;
    hitPoint.x = ray.origin.x + ray.direction.x * tMin;
    hitPoint.y = ray.origin.y + ray.direction.y * tMin;
    return true;
}

bool RayCastManager::rayIntersectsSegment(const Ray& ray, const Vector2D& p1, const Vector2D& p2, float maxDistance, SDL_FPoint& hitPoint, float& distance) {
    Vector2D r = {ray.direction.x, ray.direction.y};
    Vector2D s = { p2.x - p1.x, p2.y - p1.y };

    float rxs = r.x * s.y - r.y * s.x;
    if (std::abs(rxs) < 1e-6f) return false; // Parallel

    Vector2D qp = { p1.x - ray.origin.x, p1.y - ray.origin.y };
    float t = (qp.x * s.y - qp.y * s.x) / rxs;
    float u = (qp.x * r.y - qp.y * r.x) / rxs;

    if (t >= 0 && t <= maxDistance && u >= 0 && u <= 1) {
        distance = t;
        hitPoint.x = ray.origin.x + t * r.x;
        hitPoint.y = ray.origin.y + t * r.y;
        return true;
    }

    return false;
}

bool RayCastManager::rayIntersectsTriangle(const Ray& ray, const Triangle& triangle, float maxDistance, SDL_FPoint& hitPoint, float& distance) {
    bool hit = false;
    float closestDist = maxDistance;
    SDL_FPoint tempHit;
    float tempDist;

    if (rayIntersectsSegment(ray, triangle.p1, triangle.p2, closestDist, tempHit, tempDist)) {
        closestDist = tempDist;
        hitPoint = tempHit;
        hit = true;
    }
    if (rayIntersectsSegment(ray, triangle.p2, triangle.p3, closestDist, tempHit, tempDist)) {
        closestDist = tempDist;
        hitPoint = tempHit;
        hit = true;
    }
    if (rayIntersectsSegment(ray, triangle.p3, triangle.p1, closestDist, tempHit, tempDist)) {
        closestDist = tempDist;
        hitPoint = tempHit;
        hit = true;
    }

    if (hit) distance = closestDist;
    return hit;
}


void RayCastManager::renderRay(SDL_Renderer* renderer, const Ray& ray, const Camera& camera, float length, SDL_Color color) {
    if (!renderer) {
        Debug::Log("RayCastManager: Cannot render ray - SDL_Renderer is null.");
        return;
    }
    
    // Validate camera transform
    if (!camera.transform) {
        Debug::Log("RayCastManager: Camera transform is null!");
        return;
    }

    // Get camera properties
    SDL_FPoint camPos = {camera.transform->posX, camera.transform->posY};
    float camZoom = camera.zoom;
    float camRotation = camera.transform->rotZ;

    // Prevent division by zero
    if (camZoom == 0.0f) {
        camZoom = 0.0001f;  // Small non-zero value
        //Debug::Log("RayCastManager: Camera zoom was 0, clamped to small value");
    }

    // Calculate world coordinates
    SDL_FPoint worldStart = ray.origin;
    SDL_FPoint worldEnd = {
        ray.origin.x + ray.direction.x * length,
        ray.origin.y + ray.direction.y * length
    };

    // Convert to camera-relative coordinates
    SDL_FPoint relStart = {worldStart.x - camPos.x, worldStart.y - camPos.y};
    SDL_FPoint relEnd = {worldEnd.x - camPos.x, worldEnd.y - camPos.y};

    // Apply inverse rotation if present
    if (camRotation != 0.0f) {
        float cosRot = std::cos(-camRotation);
        float sinRot = std::sin(-camRotation);
        
        // Rotate start point
        float startX = relStart.x * cosRot + relStart.y * sinRot;
        float startY = -relStart.x * sinRot + relStart.y * cosRot;
        relStart = {startX, startY};
        
        // Rotate end point
        float endX = relEnd.x * cosRot + relEnd.y * sinRot;
        float endY = -relEnd.x * sinRot + relEnd.y * cosRot;
        relEnd = {endX, endY};
    }

    // Convert to screen coordinates
    float screenWidth = camera.windowWidth;
    float screenHeight = camera.windowHeight;
    float halfWidth = screenWidth / 2.0f;
    float halfHeight = screenHeight / 2.0f;

    SDL_FPoint screenStart = {
        halfWidth + (relStart.x * camZoom),
        halfHeight + (relStart.y * camZoom)  // Y not inverted - higher Y is down
    };
    SDL_FPoint screenEnd = {
        halfWidth + (relEnd.x * camZoom),
        halfHeight + (relEnd.y * camZoom)
    };

    // Set render color and draw
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLineF(renderer, screenStart.x, screenStart.y, screenEnd.x, screenEnd.y);
}