#include "Components.h"
#include "Debug.h"
#include "InputManager.h"

const float Collider::COLLISION_DIST_THRESHOLD = 0.001f; // Threshold for collision distance

ColliderParameters::ColliderParameters(Transform *t, float sX, float sY, bool trigger,
                                       const std::string &tTag, const std::vector<std::string> &cTags,
                                       bool cT, bool cL, bool cR, bool cB)
    : transform(t), scaleX(sX), scaleY(sY), isTrigger(trigger), tag(tTag), collisionTags(cTags),
      checkTop(cT), checkLeft(cL), checkRight(cR), checkBottom(cB) {}

Collider::Collider(ColliderParameters cp)
    : transform(cp.transform), scaleX(cp.scaleX), scaleY(cp.scaleY),
      isTrigger(cp.isTrigger), tag(cp.tag), collisionTags(cp.collisionTags),
      checkTop(cp.checkTop), checkLeft(cp.checkLeft), checkRight(cp.checkRight), checkBottom(cp.checkBottom)
{
    updateBoundingBox();
}

bool Collider::pointInTriangle(float px, float py, Triangle triangle, CollisionResult &result, const Vector2D &velocity1, const Vector2D &velocity2)
{
    // Calculate barycentric coordinates
    float denominator = ((triangle.p2.y - triangle.p3.y) * (triangle.p1.x - triangle.p3.x) +
                         (triangle.p3.x - triangle.p2.x) * (triangle.p1.y - triangle.p3.y));

    // Check for degenerate triangle
    if (std::abs(denominator) < 0.0001f)
    {
        result.isColliding = false;
        result.collisionPoint = {px, py};
        result.normal = {0, 0};
        result.penetrationDepth = 0.0f;
        return false;
    }

    float alpha = ((triangle.p2.y - triangle.p3.y) * (px - triangle.p3.x) +
                   (triangle.p3.x - triangle.p2.x) * (py - triangle.p3.y)) /
                  denominator;
    float beta = ((triangle.p3.y - triangle.p1.y) * (px - triangle.p3.x) +
                  (triangle.p1.x - triangle.p3.x) * (py - triangle.p3.y)) /
                 denominator;
    float gamma = 1.0f - alpha - beta;

    const float EPSILON = 0.001f; // Slightly larger for robustness
    result.isColliding = (alpha >= -EPSILON) && (beta >= -EPSILON) && (gamma >= -EPSILON);

    if (result.isColliding)
    {
        result.collisionPoint = {px, py};

        auto distanceToEdge = [&](Vector2D p1, Vector2D p2) -> float
        {
            // Vector AB and AP
            Vector2D ab = {p2.x - p1.x, p2.y - p1.y};
            Vector2D ap = {px - p1.x, py - p1.y};

            // Compute projection factor t
            float ab_norm = ab.x * ab.x + ab.y * ab.y; // Squared length of AB
            float t = (ap.x * ab.x + ap.y * ab.y) / ab_norm;
            t = std::max(0.0f, std::min(1.0f, t)); // Clamp t between 0 and 1

            // Compute closest point on segment
            Vector2D closest = {p1.x + t * ab.x, p1.y + t * ab.y};

            // Compute distance
            return std::sqrt((px - closest.x) * (px - closest.x) +
                             (py - closest.y) * (py - closest.y));
        };

        float d1 = distanceToEdge(triangle.p1, triangle.p2);
        float d2 = distanceToEdge(triangle.p2, triangle.p3);
        float d3 = distanceToEdge(triangle.p3, triangle.p1);

        float minDist = d1;
        Vector2D edgeStart = triangle.p1;
        Vector2D edgeEnd = triangle.p2;

        if (d2 < minDist)
        {
            minDist = d2;
            edgeStart = triangle.p2;
            edgeEnd = triangle.p3;
        }
        if (d3 < minDist)
        {
            minDist = d3;
            edgeStart = triangle.p3;
            edgeEnd = triangle.p1;
        }

        result.penetrationDepth = minDist;
        float dx = edgeEnd.x - edgeStart.x;
        float dy = edgeEnd.y - edgeStart.y;
        result.normal = normalizeVector(dy, -dx);

        float absX = std::abs(result.normal.x);
        float absY = std::abs(result.normal.y);

        /*if (absX > absY) {
            // X-axis dominates: snap to left or right
            result.normal = result.normal.x > 0 ? Vector2D{1.0f, 0.0f} : Vector2D{-1.0f, 0.0f};
        } else {
            // Y-axis dominates: snap to up or down
            result.normal = result.normal.y > 0 ? Vector2D{0.0f, 1.0f} : Vector2D{0.0f, -1.0f};
        }*/
    }
    else
    {
        result.collisionPoint = {px, py};
        result.normal = {0, 0};
        result.penetrationDepth = 0.0f;
    }

    // if(result.isColliding) Debug::Log("AAA collision, normal: (" + std::to_string(result.normal.x) + ", " +
    // std::to_string(result.normal.y) + "), penetration: " +
    // std::to_string(result.penetrationDepth) + ", collisionPoint: (" +
    // std::to_string(result.collisionPoint.x) + ", " +
    // std::to_string(result.collisionPoint.y) + ")");

    return result.isColliding;
}

bool Collider::pointInRect(float px, float py, Rect rect, CollisionResult &result, const Vector2D &velocity1, const Vector2D &velocity2)
{
    // Adjust rectangle coordinates with transform position if needed
    float rectLeft = rect.x;
    float rectRight = rectLeft + rect.w;
    float rectTop = rect.y;
    float rectBottom = rectTop + rect.h;

    // Check if point is inside rectangle bounds
    bool inside = (px >= rectLeft && px <= rectRight &&
                   py >= rectTop && py <= rectBottom);

    result.isColliding = inside;

    if (inside)
    {
        result.collisionPoint = {px, py};

        // Calculate distances to each edge
        float distLeft = std::abs(px - rectLeft);
        float distRight = std::abs(rectRight - px);
        float distTop = std::abs(py - rectTop);
        float distBottom = std::abs(rectBottom - py);

        // Find minimum distance to determine closest edge
        float minDist = distLeft;
        result.normal = {-1.0f, 0.0f}; // Left edge normal

        if (distRight < minDist)
        {
            minDist = distRight;
            result.normal = {1.0f, 0.0f}; // Right edge normal
        }
        if (distTop < minDist)
        {
            minDist = distTop;
            result.normal = {0.0f, 1.0f}; // Top edge normal
        }
        if (distBottom < minDist)
        {
            minDist = distBottom;
            result.normal = {0.0f, -1.0f}; // Bottom edge normal
        }

        result.penetrationDepth = minDist;
    }
    else
    {
        result.collisionPoint = {0, 0};
        result.normal = {0, 0};
        result.penetrationDepth = 0.0f;
    }

    // if(result.isColliding) Debug::Log("pInRect collision, normal: (" + std::to_string(result.normal.x) + ", " +
    // std::to_string(result.normal.y) + "), penetration: " +
    // std::to_string(result.penetrationDepth) + ", collisionPoint: (" +
    // std::to_string(result.collisionPoint.x) + ", " +
    // std::to_string(result.collisionPoint.y) + ")");

    return result.isColliding;
}

bool Collider::rectIntersectsTriangle(Rect rect, Triangle triangle, CollisionResult &result, const Vector2D &velocity1, const Vector2D &velocity2)
{
    result.isColliding = false;
    result.penetrationDepth = 0;
    result.collisionPoint = {0, 0};
    result.normal = {0, 0};

    // 1. Primero verificar si estamos en una situación de plataforma (desde arriba)
    Vector2D baseEdgeP1 = triangle.p1;
    Vector2D baseEdgeP2 = triangle.p3;

    // Ordenar puntos de izquierda a derecha
    if (baseEdgeP1.x > baseEdgeP2.x)
    {
        std::swap(baseEdgeP1, baseEdgeP2);
    }

    Vector2D playerBottomLeft = {rect.x, rect.y + rect.h};
    Vector2D playerBottomRight = {rect.x + rect.w, rect.y + rect.h};

    // Verificación específica para plataforma
    bool isPlatformCollision = false;
    if (velocity1.y >= 0)
    { // Solo si no estamos saltando hacia arriba
        if (playerBottomLeft.y >= baseEdgeP1.y && playerBottomLeft.y <= baseEdgeP1.y + 10.0f)
        {
            if (!(playerBottomRight.x < baseEdgeP1.x || playerBottomLeft.x > baseEdgeP2.x))
            {
                isPlatformCollision = true;
                result.isColliding = true;
                result.normal = {0.0f, -1.0f};
                result.collisionPoint = {
                    std::max(baseEdgeP1.x, std::min(playerBottomRight.x, baseEdgeP2.x)),
                    baseEdgeP1.y};
                result.penetrationDepth = playerBottomLeft.y - baseEdgeP1.y + 7;

                // Prioridad absoluta a colisiones desde arriba
                return true;
            }
        }
    }

    // 2. Verificación de colisión frontal con diagonales
    if (std::abs(velocity1.x) > 0.1f)
    { // Si hay movimiento horizontal significativo
        Vector2D playerFrontBottom = (velocity1.x > 0) ? playerBottomRight : playerBottomLeft;
        Vector2D playerFrontTop = (velocity1.x > 0) ? Vector2D{rect.x + rect.w, rect.y} : Vector2D{rect.x, rect.y};

        // Verificar ambas diagonales
        Vector2D diagonals[2][2] = {
            {triangle.p1, triangle.p2}, // Diagonal izquierda
            {triangle.p2, triangle.p3}  // Diagonal derecha
        };

        for (int i = 0; i < 2; i++)
        {
            Vector2D intersection;
            if (lineSegmentsIntersect(playerFrontTop, playerFrontBottom,
                                      diagonals[i][0], diagonals[i][1], intersection))
            {
                // Solo considerar si no hay ya una colisión de plataforma
                if (!isPlatformCollision || intersection.y < baseEdgeP1.y)
                {
                    result.isColliding = true;

                    // Calcular normal de la diagonal
                    Vector2D edgeVec = {diagonals[i][1].x - diagonals[i][0].x,
                                        diagonals[i][1].y - diagonals[i][0].y};
                    result.normal = normalizeVector(edgeVec.y, -edgeVec.x);

                    // Asegurar que la normal apunte hacia el jugador
                    Vector2D centerToEdge = {intersection.x - (rect.x + rect.w / 2),
                                             intersection.y - (rect.y + rect.h / 2)};
                    if (result.normal.x * centerToEdge.x + result.normal.y * centerToEdge.y > 0)
                    {
                        result.normal.x = -result.normal.x;
                        result.normal.y = -result.normal.y;
                    }

                    result.collisionPoint = intersection;
                    result.penetrationDepth = std::sqrt(
                        (playerFrontBottom.x - intersection.x) * (playerFrontBottom.x - intersection.x) +
                        (playerFrontBottom.y - intersection.y) * (playerFrontBottom.y - intersection.y));
                    return true;
                }
            }
        }
    }

    // 3. Verificación por puntos internos (solo si no hay otras colisiones)
    if (!result.isColliding)
    {
        Vector2D rectPoints[4] = {
            {rect.x, rect.y},
            {rect.x + rect.w, rect.y},
            {rect.x + rect.w, rect.y + rect.h},
            {rect.x, rect.y + rect.h}};

        CollisionResult tempResult;
        for (int i = 0; i < 4; i++)
        {
            if (pointInTriangle(rectPoints[i].x, rectPoints[i].y, triangle, tempResult, velocity1, velocity2))
            {
                if (tempResult.penetrationDepth > result.penetrationDepth)
                {
                    result = tempResult;
                }
            }
        }

        Vector2D triPoints[3] = {triangle.p1, triangle.p2, triangle.p3};
        for (int i = 0; i < 3; i++)
        {
            if (pointInRect(triPoints[i].x, triPoints[i].y, rect, tempResult, velocity1, velocity2))
            {
                if (tempResult.penetrationDepth > result.penetrationDepth)
                {
                    result = tempResult;
                    result.normal.x = -result.normal.x;
                    result.normal.y = -result.normal.y;
                }
            }
        }
    }

    return result.isColliding;
}

Vector2D Collider::normalizeVector(float x, float y)
{
    float length = sqrt(x * x + y * y);
    // Debug::Log("Length: " + std::to_string(length));
    if (length < 0.0001f)
        return {0, 0}; // Avoid division by zero
    return {x / length, y / length};
}

bool Collider::lineSegmentsIntersect(Vector2D p1, Vector2D p2, Vector2D p3, Vector2D p4, Vector2D &intersection)
{
    float denominator = (p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y);
    if (abs(denominator) < 0.0001f)
        return false; // Parallel lines

    float ua = ((p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x)) / denominator;
    float ub = ((p2.x - p1.x) * (p1.y - p3.y) - (p2.y - p1.y) * (p1.x - p3.x)) / denominator;

    if (ua >= 0 && ua <= 1 && ub >= 0 && ub <= 1)
    {
        intersection.x = (p1.x + ua * (p2.x - p1.x));
        intersection.y = (p1.y + ua * (p2.y - p1.y));
        return true;
    }
    return false;
}

Collider::Collider()
    : transform(nullptr), scaleX(1), scaleY(1), isTrigger(false), tag("Untagged"),
      collisionTags({""}), wasCollidingLastFrame(false),
      checkTop(true), checkLeft(true), checkRight(true), checkBottom(true) {}

void Collider::updateBoundingBox()
{
    if (!transform)
        return;
    float width = (transform->sizeX * scaleX);
    float height = (transform->sizeY * scaleY);
    float x = (transform->posX);
    float y = (transform->posY);
    if (transform->centerMode == AnchorPoint::CENTER)
    {
        boundingBox = {x - width / 2, y - height / 2, width, height};
    }
    else
    {
        boundingBox = {x, y, width, height};
    }

    if (checkTop)
    {
        topTriangle.p1 = {(boundingBox.x), (boundingBox.y)};
        topTriangle.p2 = {(boundingBox.x + boundingBox.w), (boundingBox.y)};
        topTriangle.p3 = {(boundingBox.x + boundingBox.w / 2), (boundingBox.y + boundingBox.h / 2)};
    }

    if (checkLeft)
    {
        leftTriangle.p1 = {(boundingBox.x), (boundingBox.y)};
        leftTriangle.p3 = {(boundingBox.x), (boundingBox.y + boundingBox.h)};
        leftTriangle.p2 = {(boundingBox.x + boundingBox.w / 2), (boundingBox.y + boundingBox.h / 2)};
    }

    if (checkRight)
    {
        rightTriangle.p1 = {(boundingBox.x + boundingBox.w), (boundingBox.y)};
        rightTriangle.p2 = {(boundingBox.x + boundingBox.w), (boundingBox.y + boundingBox.h)};
        rightTriangle.p3 = {(boundingBox.x + boundingBox.w / 2), (boundingBox.y + boundingBox.h / 2)};
    }

    if (checkBottom)
    {
        bottomTriangle.p1 = {(boundingBox.x), (boundingBox.y + boundingBox.h)};
        bottomTriangle.p3 = {(boundingBox.x + boundingBox.w), (boundingBox.y + boundingBox.h)};
        bottomTriangle.p2 = {(boundingBox.x + boundingBox.w / 2), (boundingBox.y + boundingBox.h / 2)};
    }
}

void Collider::start() {}

void Collider::update(float deltaTime)
{
    updateBoundingBox();
}

CollisionResult Collider::checkRectRectCollision(const Rect &rect1, const Rect &rect2, const Vector2D &velocity1, const Vector2D &velocity2)
{
    CollisionResult result = {false, {0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f};

    if (!Rect::hasIntersection(rect1, rect2))
        return result;

    result.isColliding = true;

    Rect intersection = Rect::intersectRect(rect1, rect2);
    Vector2D velocity = {velocity1.x - velocity2.x, velocity1.y - velocity2.y};

    float overlapX = intersection.w;
    float overlapY = intersection.h;

    float center1X = rect1.x + rect1.w / 2.0f;
    float center1Y = rect1.y + rect1.h / 2.0f;
    float center2X = rect2.x + rect2.w / 2.0f;
    float center2Y = rect2.y + rect2.h / 2.0f;

    float dx = center2X - center1X;
    float dy = center2Y - center1Y; // Note: Corrected y-axis direction (positive dy means rect2 below rect1)

    if (overlapX < overlapY)
    {
        // Horizontal collision (left or right edge)
        result.penetrationDepth = overlapX;
        result.normal = normalizeVector(-dx, 0.0f);

        // x is on the left or right edge
        result.collisionPoint.x = (dx > 0) ? intersection.x : (intersection.x + intersection.w);

        // y is the closest point *within* the intersection to the vertical center of the edge
        float centerY = rect1.y + rect1.h / 2.0f;
        result.collisionPoint.y = std::clamp(centerY, intersection.y, intersection.y + intersection.h);
    }
    else
    {
        // Vertical collision (top or bottom edge)
        result.penetrationDepth = overlapY;
        result.normal = normalizeVector(0.0f, -dy);

        // y is on the top or bottom edge
        result.collisionPoint.y = (dy > 0) ? intersection.y : (intersection.y + intersection.h);

        // x is the closest point *within* the intersection to the horizontal center of the edge
        float centerX = rect1.x + rect1.w / 2.0f;
        result.collisionPoint.x = std::clamp(centerX, intersection.x, intersection.x + intersection.w);
    }
    /*
        if (overlapX < overlapY) {
            // Horizontal collision (left or right edge)
            result.penetrationDepth = overlapX;
            result.normal = {dx > 0 ? 1.0f : -1.0f, 0.0f}; // Right or left normal

            // Collision point x is on the intersection edge
            result.collisionPoint.x = (dx > 0) ? intersection.x : (intersection.x + intersection.w);
            // Y is the center of the intersection
            result.collisionPoint.y = std::clamp(center1Y, intersection.y, intersection.y + intersection.h);

            // Adjust normal based on relative velocity
            if (result.normal.x * velocity.x > 0) {

                if (result.normal.x > 0) { // Normal points right
                    result.penetrationDepth = (rect2.x + rect2.w) - result.collisionPoint.x; // To right edge of rect2
                } else { // Normal points left
                    result.penetrationDepth = result.collisionPoint.x - rect2.x; // To left edge of rect2
                }

                result.normal.x = -result.normal.x;
            }
        } else {
            // Vertical collision (top or bottom edge)
            result.penetrationDepth = overlapY;
            result.normal = {0.0f, dy > 0 ? 1.0f : -1.0f}; // Down or up normal

            // Collision point y is on the intersection edge
            result.collisionPoint.y = (dy > 0) ? intersection.y : (intersection.y + intersection.h);
            // X is the center of the intersection
            result.collisionPoint.x = std::clamp(center1X, intersection.x, intersection.x + intersection.w);

            // Adjust normal based on relative velocity
            if (result.normal.y * velocity.y > 0) {

                if (result.normal.y > 0) { // Normal points down
                    result.collisionPoint.y = intersection.y; // Top edge of intersection
                    // Maximum distance: from collision point to bottom edge of rect2
                    result.penetrationDepth = (rect2.y + rect2.h) - result.collisionPoint.y;
                } else { // Normal points up
                    result.collisionPoint.y = intersection.y + intersection.h; // Bottom edge of intersection
                    // Maximum distance: from collision point to top edge of rect1 (or adjust based on rect2)
                    result.penetrationDepth = result.collisionPoint.y - rect2.y;
                }
                result.normal.y = -result.normal.y;
            }
        }
    */

    return result;
}

CollisionResult Collider::checkRectTrianglesCollision(const Rect &rect, const std::vector<Triangle> &triangles, const Vector2D &velocity1, const Vector2D &velocity2)
{
    // Initialize the result with default values: no collision, zero vectors, and a large penetration depth
    CollisionResult result = {false, {0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f};

    // Flag to track if any collision occurs
    bool anyCollision = false;
    // Initialize the minimum penetration depth to the maximum possible float value
    float minPenetrationDepth = std::numeric_limits<float>::max();
    // Accumulators for normal and collision point (not used in this version but present for potential future use)
    float accumNormalX = 0.0f;
    float accumNormalY = 0.0f;
    float accumPointX = 0.0f;
    float accumPointY = 0.0f;
    // Counter for the number of collisions (not used in this version but present for potential future use)
    int collisionCount = 0;

    // Index to track the current triangle being processed
    int i = 0;
    // Index of the selected triangle for collision resolution (-1 if none selected)
    int selectedTriangle = -1;

    // Iterate through each triangle in the vector
    for (const Triangle &tri : triangles)
    {
        // Temporary result for the current triangle's collision check
        CollisionResult triResult;

        // Check if the rectangle intersects the current triangle
        if (rectIntersectsTriangle(rect, tri, triResult, velocity1, velocity2))
        {
            // Log the collision details: penetration depth and normal

            // Skip this triangle if its penetration depth is larger than the current smallest
            if (abs(triResult.penetrationDepth) < abs(result.penetrationDepth))
            {
                i++;
                continue;
            }

            result = triResult;
            anyCollision = true;
            selectedTriangle = i;
        }
        // Increment the triangle index
        i++;
    }

    // If any collision occurred, set the isColliding flag to true
    if (anyCollision)
    {
        result.isColliding = true;
    }
    else
    {
    }

    // if(result.isColliding) Debug::Log("Final result collision, normal: (" + std::to_string(result.normal.x) + ", " +
    //            std::to_string(result.normal.y) + "), penetration: " +
    //            std::to_string(result.penetrationDepth) + ", collisionPoint: (" +
    //            std::to_string(result.collisionPoint.x) + ", " +
    //            std::to_string(result.collisionPoint.y) + ")");

    // Return the final collision result
    return result;
}

CollisionResult Collider::checkTrianglesTrianglesCollision(const std::vector<Triangle> &triangles1,
                                                           const std::vector<Triangle> &triangles2, const Vector2D &velocity1, const Vector2D &velocity2)
{
    CollisionResult result = {false, {0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f};

    bool anyCollision = false;
    float minPenetrationDepth = std::numeric_limits<float>::max();
    float accumNormalX = 0.0f;
    float accumNormalY = 0.0f;
    float accumPointX = 0.0f;
    float accumPointY = 0.0f;
    int collisionCount = 0;

    CollisionResult triResult;
    for (const Triangle &tri1 : triangles1)
    {
        for (const Triangle &tri2 : triangles2)
        {
            // Simplified triangle-triangle check using bounding box intersection first
            Rect tri1Bounds = {
                (std::min({tri1.p1.x, tri1.p2.x, tri1.p3.x})),
                (std::min({tri1.p1.y, tri1.p2.y, tri1.p3.y})),
                (std::max({tri1.p1.x, tri1.p2.x, tri1.p3.x}) - std::min({tri1.p1.x, tri1.p2.x, tri1.p3.x})),
                (std::max({tri1.p1.y, tri1.p2.y, tri1.p3.y}) - std::min({tri1.p1.y, tri1.p2.y, tri1.p3.y}))};
            Rect tri2Bounds = {
                (std::min({tri2.p1.x, tri2.p2.x, tri2.p3.x})),
                (std::min({tri2.p1.y, tri2.p2.y, tri2.p3.y})),
                (std::max({tri2.p1.x, tri2.p2.x, tri2.p3.x}) - std::min({tri2.p1.x, tri2.p2.x, tri2.p3.x})),
                (std::max({tri2.p1.y, tri2.p2.y, tri2.p3.y}) - std::min({tri2.p1.y, tri2.p2.y, tri2.p3.y}))};

            if (Rect::hasIntersection(tri1Bounds, tri2Bounds))
            {
                // For simplicity, use rectIntersectsTriangle with a temporary rect
                // This is a placeholder; a true triangle-triangle check would be more complex
                if (rectIntersectsTriangle(tri1Bounds, tri2, triResult, velocity1, velocity2))
                {
                    anyCollision = true;
                    collisionCount++;
                    accumNormalX += triResult.normal.x;
                    accumNormalY += triResult.normal.y;
                    accumPointX += triResult.collisionPoint.x;
                    accumPointY += triResult.collisionPoint.y;
                    minPenetrationDepth = std::min(minPenetrationDepth, triResult.penetrationDepth);
                }
            }
        }
    }

    if (anyCollision)
    {
        result.isColliding = true;
        if (collisionCount > 0)
        {
            result.normal = normalizeVector(accumNormalX, accumNormalY);
            result.penetrationDepth = minPenetrationDepth;
            result.collisionPoint = {accumPointX / collisionCount, accumPointY / collisionCount};
        }

        /*Debug::Log("Triangle triangle correction, normal: (" + std::to_string(result.normal.x) + ", " +
                   std::to_string(result.normal.y) + "), penetration: " +
                   std::to_string(result.penetrationDepth) + ", collisionPoint: (" +
                   std::to_string(result.collisionPoint.x) + ", " +
                   std::to_string(result.collisionPoint.y) + ")");*/
    }

    return result;
}

CollisionResult Collider::checkCollision(Collider *other, const Vector2D &velocity1, const Vector2D &velocity2, float deltaTime)
{
    CollisionResult result = {false, {0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f};

    // Check collision tags
    bool tagMatch = collisionTags.empty() ||
                    std::find(collisionTags.begin(), collisionTags.end(), other->tag) != collisionTags.end();
    if (!tagMatch)
        return result;

    // Determine collider types
    bool thisFullAABB = checkTop && checkLeft && checkRight && checkBottom;
    bool otherFullAABB = other->checkTop && other->checkLeft && other->checkRight && other->checkBottom;

    // Collect triangles for this collider
    std::vector<Triangle> thisTriangles;
    if (checkTop)
        thisTriangles.push_back(topTriangle);
    if (checkBottom)
        thisTriangles.push_back(bottomTriangle);
    if (checkLeft)
        thisTriangles.push_back(leftTriangle);
    if (checkRight)
        thisTriangles.push_back(rightTriangle);

    // Collect triangles for other collider
    std::vector<Triangle> otherTriangles;
    if (other->checkTop)
        otherTriangles.push_back(other->topTriangle);
    if (other->checkBottom)
        otherTriangles.push_back(other->bottomTriangle);
    if (other->checkLeft)
        otherTriangles.push_back(other->leftTriangle);
    if (other->checkRight)
        otherTriangles.push_back(other->rightTriangle);

    // Delegate to appropriate collision check
    if (thisFullAABB && otherFullAABB)
    {
        result = checkRectRectCollision(boundingBox, other->boundingBox, velocity1, velocity2);
    }
    else if (thisFullAABB && !otherFullAABB)
    {
        result = checkRectTrianglesCollision(boundingBox, otherTriangles, velocity1, velocity2);
    }
    else if (!thisFullAABB && otherFullAABB)
    {
        result = checkRectTrianglesCollision(other->boundingBox, thisTriangles, velocity2, velocity1);
    }
    else
    {
        result = checkTrianglesTrianglesCollision(thisTriangles, otherTriangles, velocity1, velocity2);
    }

    if (other->tag == "Terrain")
    {
        // Calculate min distance between collision point and corners
        // Create a lambda function to calculate distance between 2 Vector2D points
        auto distance = [](Vector2D a, Vector2D b)
        {
            return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
        };

        float dTopLeft = distance(result.collisionPoint, Vector2D{boundingBox.x, boundingBox.y});
        float dTopRight = distance(result.collisionPoint, Vector2D{boundingBox.x + boundingBox.w, boundingBox.y});
        float dBottomLeft = distance(result.collisionPoint, Vector2D{boundingBox.x, boundingBox.y + boundingBox.h});
        float dBottomRight = distance(result.collisionPoint, Vector2D{boundingBox.x + boundingBox.w, boundingBox.y + boundingBox.h});

        float minDistance = std::min({dTopLeft, dTopRight, dBottomLeft, dBottomRight});

        if (minDistance < COLLISION_DIST_THRESHOLD && std::abs(result.normal.x) >= 0.98)
        {
            if (minDistance == dTopLeft)
            {
                result.collisionPoint = Vector2D{boundingBox.x, boundingBox.y};
                result.normal = Vector2D{0.0f, -1.0f};
            }
            else if (minDistance == dTopRight)
            {
                result.collisionPoint = Vector2D{boundingBox.x + boundingBox.w, boundingBox.y};
                result.normal = Vector2D{0.0f, -1.0f};
            }
            else if (minDistance == dBottomLeft)
            {
                result.collisionPoint = Vector2D{boundingBox.x, boundingBox.y + boundingBox.h};
                result.normal = Vector2D{0.0f, 1.0f};
            }
            else if (minDistance == dBottomRight)
            {
                result.collisionPoint = Vector2D{boundingBox.x + boundingBox.w, boundingBox.y + boundingBox.h};
                result.normal = Vector2D{0.0f, 1.0f};
            }

            Debug::Log("Correction applied");
        }
    }

    static float collisionTimeStep = 0.1f;

    // Update timer for colliding objects
    if (activeColliders.find(other->parent) != activeColliders.end())
    {
        if (colliderTimers[other->parent] != 0.0f)
            colliderTimers[other->parent] -= deltaTime;
        if (colliderTimers[other->parent] < 0.0f)
            colliderTimers[other->parent] = 0.0f;
    }

    if (result.isColliding)
    {
        // Only trigger onCollisionEnter if the collider is not already in the active set
        if (activeColliders.find(other->parent) == activeColliders.end())
        {
            if (onCollisionEnter != nullptr)
            {
                onCollisionEnter(other, result);
            }
            activeColliders.insert(other->parent);             // Add the collider to the set after entering
            colliderTimers[other->parent] = collisionTimeStep; // Initialize timer for the new collision
        }
        else
        {
            // If collider is already in the active set and enough time has passed, trigger onCollisionStay
            colliderTimers[other->parent] = collisionTimeStep; // Reset timer after triggering onCollisionStay
            if (onCollisionStay != nullptr)
            {
                onCollisionStay(other, result);
            }
        }
    }
    else
    {
        // If the collision ends and the collider is in the set, trigger onCollisionExit
        if (activeColliders.find(other->parent) != activeColliders.end() && colliderTimers[other->parent] == 0.0f)
        {
            if (onCollisionExit != nullptr)
            {
                onCollisionExit(other);
            }
            activeColliders.erase(other->parent); // Remove the collider from the set after exiting
            colliderTimers.erase(other->parent);  // Remove timer when collider exits
        }

        colPoint = {100000.0f, 100000.0f};
    }

    return result;
}

std::string Collider::getTag() const
{
    return tag;
}

void Collider::setTag(std::string newTag)
{
    tag = newTag;
}

void Collider::addCollisionTag(const std::string &collisionTag)
{
    if (std::find(collisionTags.begin(), collisionTags.end(), collisionTag) == collisionTags.end())
    {
        collisionTags.push_back(collisionTag);
    }
}

void Collider::removeCollisionTag(const std::string &collisionTag)
{
    auto it = std::find(collisionTags.begin(), collisionTags.end(), collisionTag);
    if (it != collisionTags.end())
    {
        collisionTags.erase(it);
    }
}

std::vector<std::string> Collider::getCollisionTags() const
{
    return collisionTags;
}

void Collider::render(EntityManager &entityManager, SDL_Renderer *renderer)
{

    return; // Disable rendering for now

    Camera *camera = nullptr;
    for (Entity entity : entityManager.getEntities())
    {
        camera = entityManager.getComponent<Camera>(entity);
        if (camera)
            break;
    }
    if (!camera)
        return;

    // Calculate scaling factor for visual elements (reference: 800x600)
    float scaleFactor = static_cast<float>(camera->windowWidth) / 800.0f;

    // Convert colPoint to screen space
    SDL_FPoint screenPos = InputManager::worldToScreen(colPoint.x, colPoint.y, *camera);
    // Ensure screenPos is adjusted for excess space (if not done in worldToScreen)
    screenPos.x += camera->excessWidth / 2.0f;
    screenPos.y += camera->excessHeight / 2.0f;

    // Render the colPoint dot, scaled for the viewport
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderDrawPoint(renderer, (int)screenPos.x, (int)screenPos.y);
    int dotSize = static_cast<int>(3 * scaleFactor); // Scale the 3x3 dot
    SDL_Rect dot = {(int)screenPos.x - dotSize / 2, (int)screenPos.y - dotSize / 2, dotSize, dotSize};
    SDL_RenderFillRect(renderer, &dot);

    // Set color for the collider box
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    if (tag == "Terrain")
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

    // Camera properties
    float camX = camera->getPosX();
    float camY = camera->getPosY();
    float zoom = camera->zoom;
    int screenW = camera->windowWidth;
    int screenH = camera->windowHeight;

    // Render the bounding box
    Rect renderBox = boundingBox;
    if (transform->centerMode == AnchorPoint::CENTER)
    {
        renderBox.x = ((boundingBox.x - camX) * zoom + screenW / 2) + camera->excessWidth / 2;
        renderBox.y = ((boundingBox.y - camY) * zoom + screenH / 2) + camera->excessHeight / 2;
    }
    else
    {
        renderBox.x = ((boundingBox.x - camX) * zoom + screenW / 2) + camera->excessWidth / 2;
        renderBox.y = ((boundingBox.y - camY) * zoom + screenH / 2) + camera->excessHeight / 2;
    }
    renderBox.w = (boundingBox.w * zoom);
    renderBox.h = (boundingBox.h * zoom);

    if (checkTop && checkLeft && checkRight && checkBottom)
    {
        int x = static_cast<int>(std::floor(renderBox.x));                  // Left (inclusive start)
        int y = static_cast<int>(std::floor(renderBox.y));                  // Top (inclusive start)
        int w = static_cast<int>(std::ceil(renderBox.x + renderBox.w)) - x; // Width to exclusive end
        int h = static_cast<int>(std::ceil(renderBox.y + renderBox.h)) - y; // Height to exclusive end

        SDL_Rect toRender = {x, y, w, h};
        SDL_RenderDrawRect(renderer, &toRender);
        return;
    }

    // Render individual sides of the collider
    int x1 = boundingBox.x;
    int y1 = boundingBox.y;
    int x2 = boundingBox.x + boundingBox.w;
    int y2 = boundingBox.y;
    int x3 = boundingBox.x;
    int y3 = boundingBox.y + boundingBox.h;
    int x4 = boundingBox.x + boundingBox.w;
    int y4 = boundingBox.y + boundingBox.h;
    int midX = x1 + boundingBox.w / 2;
    int midY = y1 + boundingBox.h / 2;

    int renderX1 = static_cast<int>((x1 - camX) * zoom + screenW / 2 + camera->excessWidth / 2);
    int renderY1 = static_cast<int>((y1 - camY) * zoom + screenH / 2 + camera->excessHeight / 2);
    int renderX2 = static_cast<int>((x2 - camX) * zoom + screenW / 2 + camera->excessWidth / 2);
    int renderY2 = static_cast<int>((y2 - camY) * zoom + screenH / 2 + camera->excessHeight / 2);
    int renderX3 = static_cast<int>((x3 - camX) * zoom + screenW / 2 + camera->excessWidth / 2);
    int renderY3 = static_cast<int>((y3 - camY) * zoom + screenH / 2 + camera->excessHeight / 2);
    int renderX4 = static_cast<int>((x4 - camX) * zoom + screenW / 2 + camera->excessWidth / 2);
    int renderY4 = static_cast<int>((y4 - camY) * zoom + screenH / 2 + camera->excessHeight / 2);
    int renderMidX = static_cast<int>((midX - camX) * zoom + screenW / 2 + camera->excessWidth / 2);
    int renderMidY = static_cast<int>((midY - camY) * zoom + screenH / 2 + camera->excessHeight / 2);

    if (checkTop)
    {
        SDL_RenderDrawLine(renderer, renderX1, renderY1, renderX2, renderY2);
        SDL_RenderDrawLine(renderer, renderX2, renderY2, renderMidX, renderMidY);
        SDL_RenderDrawLine(renderer, renderMidX, renderMidY, renderX1, renderY1);
    }
    if (checkLeft)
    {
        SDL_RenderDrawLine(renderer, renderX1, renderY1, renderMidX, renderMidY);
        SDL_RenderDrawLine(renderer, renderMidX, renderMidY, renderX3, renderY3);
        SDL_RenderDrawLine(renderer, renderX3, renderY3, renderX1, renderY1);
    }
    if (checkRight)
    {
        SDL_RenderDrawLine(renderer, renderX2, renderY2, renderMidX, renderMidY);
        SDL_RenderDrawLine(renderer, renderMidX, renderMidY, renderX4, renderY4);
        SDL_RenderDrawLine(renderer, renderX4, renderY4, renderX2, renderY2);
    }
    if (checkBottom)
    {
        SDL_RenderDrawLine(renderer, renderX3, renderY3, renderX4, renderY4);
        SDL_RenderDrawLine(renderer, renderX4, renderY4, renderMidX, renderMidY);
        SDL_RenderDrawLine(renderer, renderMidX, renderMidY, renderX3, renderY3);
    }
}

void Collider::setTriangleCheck(bool top, bool left, bool right, bool bottom)
{
    checkTop = top;
    checkLeft = left;
    checkRight = right;
    checkBottom = bottom;
}

ColliderParameters ColliderLoader::fromJSON(const nlohmann::json &j, const EntityManager &entityManager)
{
    Transform *transform = entityManager.getComponent<Transform>(entityManager.getLast());
    if (!transform)
    {
        throw std::runtime_error("Collider requiere un componente Transform");
    }
    ColliderParameters params(transform);
    params.scaleX = j.value("scaleX", 1.0f);
    params.scaleY = j.value("scaleY", 1.0f);
    params.isTrigger = j.value("isTrigger", false);
    params.tag = j.value("tag", "Untagged");
    if (j.contains("collisionTags") && j["collisionTags"].is_array())
    {
        params.collisionTags.clear();
        for (const auto &tag : j["collisionTags"])
        {
            if (tag.is_string())
            {
                params.collisionTags.push_back(tag.get<std::string>());
            }
        }
    }
    params.checkTop = j.value("checkTop", true);
    params.checkLeft = j.value("checkLeft", true);
    params.checkRight = j.value("checkRight", true);
    params.checkBottom = j.value("checkBottom", true);
    return params;
}

Collider ColliderLoader::createFromJSON(const nlohmann::json &j, const EntityManager &entityManager)
{
    return Collider(fromJSON(j, entityManager));
}
