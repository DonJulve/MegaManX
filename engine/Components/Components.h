#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <functional>
#include <memory>  // Para std::shared_ptr
#include "Debug.h"
#include "EntityManager.h"
#include <nlohmann/json.hpp>

/**
 * @brief Enumeración para los puntos de anclaje de los componentes.
 */
enum class AnchorPoint { TOP_LEFT, CENTER };

NLOHMANN_JSON_SERIALIZE_ENUM(AnchorPoint, {
    {AnchorPoint::CENTER, "CENTER"},
    {AnchorPoint::TOP_LEFT, "TOP_LEFT"}
});

/**
 * @struct Vector2D
 * @brief Estructura que representa un vector 2D con componentes X e Y.
 */
struct Vector2D{
    float x, y; ///< Coordenadas X e Y del vector
};



/**
 * @brief Estructura que contiene parámetros para el componente Transform.
 */
struct TransformParameters {
    float posX, posY;          /**< Posición en X e Y. */
    float rotZ;                /**< Rotación en el eje Z. */
    float sizeX, sizeY;        /**< Tamaño en X e Y. */
    AnchorPoint centerMode;    /**< Modo de centrado (TOP_LEFT o CENTER). */

    /**
     * @brief Constructor de TransformParameters.
     * @param pX Posición en X (por defecto 0).
     * @param pY Posición en Y (por defecto 0).
     * @param rZ Rotación en Z (por defecto 0).
     * @param sX Tamaño en X (por defecto 0).
     * @param sY Tamaño en Y (por defecto 0).
     * @param aP Punto de anclaje (por defecto CENTER).
     */
    TransformParameters(float pX = 0, float pY = 0, float rZ = 0, float sX = 0, float sY = 0, AnchorPoint aP = AnchorPoint::CENTER)
        : posX(pX), posY(pY), rotZ(rZ), sizeX(sX), sizeY(sY), centerMode(aP) {}
};

/**
 * @brief Componente Transform que representa la posición, rotación y tamaño de una entidad.
 */
struct Transform : public Component {
    float posX, posY;          /**< Posición en X e Y. */
    float rotZ;                /**< Rotación en el eje Z. */
    float sizeX, sizeY;        /**< Tamaño en X e Y. */
    AnchorPoint centerMode;    /**< Modo de centrado. */

public:
    /**
     * @brief Constructor de Transform con parámetros.
     * @param tp Instancia de TransformParameters.
     */
    Transform(TransformParameters tp);

    /**
     * @brief Constructor por defecto de Transform.
     */
    Transform();

    /** @brief Método de inicio del componente Transform. */
    void start() override;

    /**
     * @brief Actualiza el estado del componente Transform.
     * @param deltaTime Tiempo transcurrido desde la última actualización.
     */
    void update(float deltaTime) override;
};

/**
 * @brief Clase TransformLoader para cargar parámetros de Transform desde JSON.
 */
class TransformLoader {
public:
    /**
     * @brief Convierte un objeto JSON en una instancia de TransformParameters.
     * @param j Objeto JSON con los datos.
     * @param entityManager Instancia de EntityManager.
     * @return Instancia de TransformParameters generada desde JSON.
     */
    static TransformParameters fromJSON(const nlohmann::json& j, const EntityManager& entityManager);

    /**
     * @brief Crea un componente Transform directamente desde JSON.
     * @param j Objeto JSON con los datos.
     * @param entityManager Instancia de EntityManager.
     * @return Componente Transform creado.
     */
    static Transform createFromJSON(const nlohmann::json& j, const EntityManager& entityManager);
};

/**
 * @brief Estructura que contiene parámetros para el componente Rigidbody.
 */
struct RigidbodyParameters {
    float velocityX, velocityY;      /**< Velocidad en X e Y. */
    float accelerationX, accelerationY; /**< Aceleración en X e Y. */
    float mass;                      /**< Masa del objeto. */
    float gravity;                   /**< Fuerza de gravedad aplicada. */
    float drag;                      /**< Resistencia del aire. */
    float bounceFactor;              /**< Factor de rebote. */
    bool kinematic;                  /**< Indica si el objeto es cinemático. */

    /**
     * @brief Constructor de RigidbodyParameters.
     * @param vx Velocidad en X (por defecto 0).
     * @param vy Velocidad en Y (por defecto 0).
     * @param m Masa (por defecto 1.0).
     * @param kin Cinemático (por defecto false).
     * @param g Gravedad (por defecto 9.8).
     * @param d Resistencia (por defecto 0.01).
     * @param b Factor de rebote (por defecto 0.5).
     */
    RigidbodyParameters(float vx = 0, float vy = 0, float m = 1.0f, bool kin = false, float g = 9.8f, float d = 0.01f, float b = 0.5f);
};

/**
 * @brief Componente Rigidbody que simula física básica (velocidad, aceleración, etc.).
 */
struct Rigidbody : public Component {
    float velocityX, velocityY;      /**< Velocidad en X e Y. */
    float accelerationX, accelerationY; /**< Aceleración en X e Y. */
    float mass;                      /**< Masa del objeto. */
    float gravity;                   /**< Fuerza de gravedad aplicada. */
    float drag;                      /**< Resistencia del aire. */
    float bounceFactor;              /**< Factor de rebote. */
    bool kinematic;                  /**< Indica si el objeto es cinemático. */

public:
    /**
     * @brief Constructor de Rigidbody con parámetros.
     * @param rbp Instancia de RigidbodyParameters.
     */
    Rigidbody(RigidbodyParameters rbp);

    /**
     * @brief Constructor por defecto de Rigidbody.
     */
    Rigidbody();

    /**
     * @brief Aplica una fuerza al componente Rigidbody.
     * @param forceX Fuerza en el eje X.
     * @param forceY Fuerza en el eje Y.
     */
    void ApplyForce(float forceX, float forceY);

    /** @brief Método de inicio del componente Rigidbody. */
    void start() override;

    /**
     * @brief Actualiza el estado del componente Rigidbody.
     * @param deltaTime Tiempo transcurrido desde la última actualización.
     */
    void update(float deltaTime) override;
};

/**
 * @brief Clase RigidbodyLoader para cargar parámetros de Rigidbody desde JSON.
 */
class RigidbodyLoader {
public:
    /**
     * @brief Convierte un objeto JSON en una instancia de RigidbodyParameters.
     * @param j Objeto JSON con los datos.
     * @param entityManager Instancia de EntityManager.
     * @return Instancia de RigidbodyParameters generada desde JSON.
     */
    static RigidbodyParameters fromJSON(const nlohmann::json& j, const EntityManager& entityManager);

    /**
     * @brief Crea un componente Rigidbody directamente desde JSON.
     * @param j Objeto JSON con los datos.
     * @param entityManager Instancia de EntityManager.
     * @return Componente Rigidbody creado.
     */
    static Rigidbody createFromJSON(const nlohmann::json& j, const EntityManager& entityManager);
};

/**
 * @brief Estructura que contiene parámetros para el componente Camera.
 */
struct CameraParameters {
    Transform* transform;     /**< Puntero al componente Transform asociado. */
    float zoom;               /**< Factor de zoom de la cámara. */
    int windowWidth, windowHeight; /**< Dimensiones de la ventana. */

    /**
     * @brief Constructor de CameraParameters.
     * @param t Puntero al componente Transform (por defecto nullptr).
     * @param z Factor de zoom (por defecto 1.0).
     * @param windowW Ancho de la ventana (por defecto 800).
     * @param windowH Alto de la ventana (por defecto 600).
     */
    CameraParameters(Transform* t = nullptr, float z = 1.0f, int windowW = 800, int windowH = 600);
};

/**
 * @brief Componente Camera que representa una cámara en el juego.
 */
struct Camera : public Component {
    Transform* transform;     /**< Puntero al componente Transform asociado. */
    float zoom;               /**< Factor de zoom de la cámara. */
    float aspectRatio;        /**< Relación de aspecto para mantener la escala. */
    int windowWidth, windowHeight; /**< Dimensiones de la ventana. */

public:

    int outWidth, outHeight; /**< Dimensiones de salida para la cámara. */
    int excessWidth, excessHeight; /**< Dimensiones excedentes para la cámara. */

    /**
     * @brief Constructor de Camera con parámetros.
     * @param cp Instancia de CameraParameters.
     */
    Camera(CameraParameters cp);

    /**
     * @brief Constructor por defecto de Camera.
     */
    Camera();

    /** @brief Método de inicio del componente Camera. */
    void start() override;

    /**
     * @brief Actualiza el estado del componente Camera.
     * @param deltaTime Tiempo transcurrido desde la última actualización.
     */
    void update(float deltaTime) override;

    /**
     * @brief Actualiza la escala de la cámara según las nuevas dimensiones de la ventana.
     * @param newWindowWidth Nuevo ancho de la ventana.
     * @param newWindowHeight Nuevo alto de la ventana.
     */
    void updateScaling(int newWindowWidth, int newWindowHeight);

    /** @brief Obtiene la posición X de la cámara desde el componente Transform. */
    float getPosX();

    /** @brief Obtiene la posición Y de la cámara desde el componente Transform. */
    float getPosY();
};

/**
 * @brief Clase CameraLoader para cargar parámetros de Camera desde JSON.
 */
class CameraLoader {
public:
    /**
     * @brief Convierte un objeto JSON en una instancia de CameraParameters.
     * @param j Objeto JSON con los datos.
     * @param entityManager Instancia de EntityManager.
     * @return Instancia de CameraParameters generada desde JSON.
     */
    static CameraParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager);

    /**
     * @brief Crea un componente Camera directamente desde JSON.
     * @param j Objeto JSON con los datos.
     * @param entityManager Instancia de EntityManager.
     * @return Componente Camera creado.
     */
    static Camera createFromJSON(nlohmann::json& j, EntityManager& entityManager);
};

/**
 * @brief Estructura que contiene parámetros para el componente Collider.
 */
struct ColliderParameters {
    Transform* transform;          /**< Puntero al componente Transform asociado. */
    float scaleX, scaleY;          /**< Escala en X e Y. */
    bool isTrigger;                /**< Indica si es un disparador (trigger). */
    std::string tag;               /**< Etiqueta para identificar el tipo de collider. */
    std::vector<std::string> collisionTags; /**< Lista de etiquetas con las que colisionar. */
    bool checkTop;                             /**< Toggle for top triangle. */
    bool checkLeft;                            /**< Toggle for left triangle. */
    bool checkRight;                           /**< Toggle for right triangle. */
    bool checkBottom;                          /**< Toggle for bottom triangle. */

    /**
     * @brief Constructor de ColliderParameters.
     * @param t Puntero al componente Transform (requerido).
     * @param sX Escala en X (por defecto 1.0).
     * @param sY Escala en Y (por defecto 1.0).
     * @param trigger Es disparador (por defecto false).
     * @param tTag Etiqueta del collider (por defecto "Untagged").
     * @param cTags Lista de etiquetas de colisión (por defecto vacía, colisiona con todo).
     */
    ColliderParameters(Transform* t, float sX = 1.0f, float sY = 1.0f, bool trigger = false, 
                       const std::string& tTag = "Untagged", 
                       const std::vector<std::string>& cTags = {}, bool cT = true, bool cL = true, bool cR = true, bool cB = true);
};

struct CollisionResult {
    bool isColliding;
    Vector2D collisionPoint;
    Vector2D normal;
    float penetrationDepth;
};



struct Rect {
    public:
    float x;    // x-coordinate of top-left corner
    float y;    // y-coordinate of top-left corner
    float w;    // width
    float h;    // height

    // Default constructor
    Rect() : x(0.0f), y(0.0f), w(0.0f), h(0.0f) {}

    // Parameterized constructor
    Rect(float xPos, float yPos, float width, float height) 
        : x(xPos), y(yPos), w(width), h(height) {}

    // Method to calculate area
    float getArea() const {
        return w * h;
    }

    // Method to check if a point is inside the rectangle
    bool contains(float pointX, float pointY) const {
        return (pointX >= x && pointX <= x + w &&
                pointY >= y && pointY <= y + h);
    }

    // Static method to check if two rectangles intersect
    static bool hasIntersection(const Rect& rect1, const Rect& rect2) {
        // Two rectangles don't intersect if one is to the left of the other
        // or one is above the other
        return !(rect1.x + rect1.w < rect2.x ||
                    rect2.x + rect2.w < rect1.x ||
                    rect1.y + rect1.h < rect2.y ||
                    rect2.y + rect2.h < rect1.y);
    }

    // Static method to get the intersection rectangle of two rectangles
    static Rect intersectRect(const Rect& rect1, const Rect& rect2) {
        // If there's no intersection, return a zero-sized rectangle
        if (!hasIntersection(rect1, rect2)) {
            return Rect();
        }

        // Calculate intersection coordinates
        float left = std::max(rect1.x, rect2.x);
        float right = std::min(rect1.x + rect1.w, rect2.x + rect2.w);
        float top = std::max(rect1.y, rect2.y);
        float bottom = std::min(rect1.y + rect1.h, rect2.y + rect2.h);

        return Rect(left, top, right - left, bottom - top);
    }
};

struct Triangle {
    Vector2D p1;
    Vector2D p2;
    Vector2D p3;
};

/**
 * @brief Componente Collider que maneja colisiones usando un rectángulo delimitador.
 */
struct Collider : public Component {

private: 
    

    

    /**
     * @brief Verifica si un rectángulo intersecta con un triángulo.
     * @param rect Rectángulo a verificar.
     * @param triangle Triángulo a verificar.
     * @param result Resultado de la colisión.
     * @return Verdadero si hay intersección.
     */
    bool rectIntersectsTriangle(Rect rect, Triangle triangle, CollisionResult& result, const Vector2D& velocity1, const Vector2D& velocity2);

    /**
     * @brief Verifica colisión entre dos rectángulos.
     * @param rect1 Primer rectángulo.
     * @param rect2 Segundo rectángulo.
     * @return Resultado de la colisión.
     */
    CollisionResult checkRectRectCollision(const Rect& rect1, const Rect& rect2, const Vector2D& velocity1, const Vector2D& velocity2);

    /**
     * @brief Verifica colisión entre un rectángulo y un conjunto de triángulos.
     * @param rect Rectángulo a verificar.
     * @param triangles Vector de triángulos a verificar.
     * @return Resultado de la colisión.
     */
    CollisionResult checkRectTrianglesCollision(const Rect& rect, const std::vector<Triangle>& triangles, const Vector2D& velocity1, const Vector2D& velocity2);

    /**
     * @brief Verifica colisión entre dos conjuntos de triángulos.
     * @param triangles1 Primer vector de triángulos.
     * @param triangles2 Segundo vector de triángulos.
     * @return Resultado de la colisión.
     */
    CollisionResult checkTrianglesTrianglesCollision(const std::vector<Triangle>& triangles1, 
                                                    const std::vector<Triangle>& triangles2, const Vector2D& velocity1, const Vector2D& velocity2);

    /**
     * @brief Normaliza un vector dado sus componentes X e Y.
     * @param x Componente X del vector.
     * @param y Componente Y del vector.
     * @return Vector2D normalizado.
     */
    Vector2D normalizeVector(float x, float y);

    /**
     * @brief Verifica si dos segmentos de línea se intersectan y calcula el punto de intersección.
     * @param p1 Punto inicial del primer segmento.
     * @param p2 Punto final del primer segmento.
     * @param p3 Punto inicial del segundo segmento.
     * @param p4 Punto final del segundo segmento.
     * @param intersection Punto de intersección (si existe).
     * @return Verdadero si los segmentos se intersectan.
     */
    bool lineSegmentsIntersect(Vector2D p1, Vector2D p2, Vector2D p3, Vector2D p4, Vector2D& intersection);

    /**
     * @brief Verifica colisión con otro Collider, considerando etiquetas y triángulos.
     * @param other Puntero al otro Collider con el que se verifica la colisión.
     * @return Resultado de la colisión.
     */
    CollisionResult checkCollision(Collider* other, const Vector2D& velocity1, const Vector2D& velocity2, float deltaTime);

    friend class PhysicsSystem;
    friend class CollisionSystem;
    friend class RenderSystem;
    friend class RaycastManager;

public:
/**
     * @brief Determina si un punto está dentro de un triángulo.
     * @param px Coordenada X del punto.
     * @param py Coordenada Y del punto.
     * @param triangle Triángulo a verificar.
     * @param result Resultado de la colisión.
     * @return Verdadero si el punto está dentro del triángulo.
     */
    bool pointInTriangle(float px, float py, Triangle triangle, CollisionResult& result, const Vector2D& velocity1, const Vector2D& velocity2);

    bool pointInRect(float px, float py, Rect rect, CollisionResult& result, const Vector2D& velocity1, const Vector2D& velocity2);
    
    Rect boundingBox;                      /**< Rectángulo delimitador para colisiones. */
    Transform* transform;                      /**< Puntero al componente Transform asociado. */
    float scaleX, scaleY;                      /**< Escala en X e Y. */
    bool isTrigger;                            /**< Indica si es un disparador. */
    std::string tag;                           /**< Etiqueta para identificar el tipo de collider. */
    std::vector<std::string> collisionTags;    /**< Lista de etiquetas con las que colisionar. */
    std::function<void(Collider*, CollisionResult)> onCollisionEnter; /**< Evento al iniciar colisión. */
    std::function<void(Collider*, CollisionResult)> onCollisionStay;  /**< Evento mientras colisiona. */
    std::function<void(Collider*)> onCollisionExit;  /**< Evento al salir de colisión. */
    bool wasCollidingLastFrame;                /**< Estado de colisión del frame anterior. */
    bool checkTop;                             /**< Toggle for top triangle. */
    bool checkLeft;                            /**< Toggle for left triangle. */
    bool checkRight;                           /**< Toggle for right triangle. */
    bool checkBottom;                          /**< Toggle for bottom triangle. */

    Triangle topTriangle;
    Triangle leftTriangle;
    Triangle rightTriangle;
    Triangle bottomTriangle;

    Vector2D colPoint;

    std::unordered_map<int, float> colliderTimers; // Timer for each collider
    std::set<int> activeColliders;

    static const float COLLISION_DIST_THRESHOLD; // Threshold for collision distance

    /**
     * @brief Constructor de Collider con parámetros.
     * @param cp Instancia de ColliderParameters.
     */
    Collider(ColliderParameters cp);

    /**
     * @brief Constructor por defecto de Collider.
     */
    Collider();

    /** @brief Actualiza el rectángulo delimitador basado en el componente Transform. */
    void updateBoundingBox();

    /** @brief Método de inicio del componente Collider. */
    void start() override;

    /**
     * @brief Actualiza el estado del componente Collider.
     * @param deltaTime Tiempo transcurrido desde la última actualización.
     */
    void update(float deltaTime) override;

    /**
     * @brief Obtiene la etiqueta del Collider.
     * @return La etiqueta como string.
     */
    std::string getTag() const;

    /**
     * @brief Establece la etiqueta del Collider.
     * @param newTag Nueva etiqueta a asignar.
     */
    void setTag(std::string newTag);

    /**
     * @brief Agrega una etiqueta a la lista de colisiones.
     * @param collisionTag Etiqueta a agregar.
     */
    void addCollisionTag(const std::string& collisionTag);

    /**
     * @brief Elimina una etiqueta de la lista de colisiones.
     * @param collisionTag Etiqueta a eliminar.
     */
    void removeCollisionTag(const std::string& collisionTag);

    /**
     * @brief Obtiene la lista de etiquetas de colisión.
     * @return Vector de etiquetas.
     */
    std::vector<std::string> getCollisionTags() const;

    /**
     * @brief Renderiza el componente Collider.
     * @param entityManager Referencia al EntityManager que contiene los componentes.
     * @param renderer Puntero al SDL_Renderer usado para dibujar.
     */
    void render(EntityManager& entityManager, SDL_Renderer* renderer) override;

    /** @brief Setter for triangle toggles with specified names. */
    void setTriangleCheck(bool top, bool left, bool right, bool bottom);

    Rect getBoundingBox() { return boundingBox; }

    bool isTriggerCol() { return isTrigger; }
};

/**
 * @brief Clase ColliderLoader para cargar parámetros de Collider desde JSON.
 */
class ColliderLoader {
public:
    /**
     * @brief Convierte un objeto JSON en una instancia de ColliderParameters.
     * @param j Objeto JSON con los datos.
     * @param entityManager Instancia de EntityManager.
     * @return Instancia de ColliderParameters generada desde JSON.
     */
    static ColliderParameters fromJSON(const nlohmann::json& j, const EntityManager& entityManager);

    /**
     * @brief Crea un componente Collider directamente desde JSON.
     * @param j Objeto JSON con los datos.
     * @param entityManager Instancia de EntityManager.
     * @return Componente Collider creado.
     */
    static Collider createFromJSON(const nlohmann::json& j, const EntityManager& entityManager);
};

/**
 * @brief Estructura que contiene parámetros para el componente SpriteRenderer.
 */
struct SpriteRendererParameters {
    SDL_Texture* texture;      /**< Textura del sprite. */
    AnchorPoint centerMode;    /**< Modo de centrado. */
    Transform* transform;      /**< Puntero al componente Transform asociado. */
    int alpha;

    /**
     * @brief Constructor de SpriteRendererParameters.
     * @param tex Puntero a la textura SDL.
     * @param center Modo de centrado (por defecto CENTER).
     * @param t Puntero al componente Transform asociado (por defecto nullptr).
     */
    SpriteRendererParameters(SDL_Texture* tex, AnchorPoint center = AnchorPoint::CENTER, Transform* t = nullptr, int alpha = 255);
};

/**
 * @brief Componente SpriteRenderer que renderiza un sprite en pantalla.
 */
struct SpriteRenderer : public Component {
public:
    SDL_Texture* texture;      /**< Textura del sprite. */
    SDL_Rect worldBounds;      /**< Cached world bounds in unscaled world coordinates. */
    int width, height;         /**< Ancho y alto del sprite para renderizado. */
    AnchorPoint centerMode;    /**< Modo de centrado. */
    Transform* transform;      /**< Puntero al componente Transform asociado. */
    SDL_RendererFlip flip;     /**< Modo de volteo del sprite para renderizado. */
    int alpha;               /**< Canal alfa para transparencia. */

public:
    /**
     * @brief Constructor de SpriteRenderer con parámetros.
     * @param srp Instancia de SpriteRendererParameters.
     */
    SpriteRenderer(SpriteRendererParameters srp);

    /**
     * @brief Constructor por defecto de SpriteRenderer.
     */
    SpriteRenderer();

    /** @brief Método de inicio del componente SpriteRenderer. */
    void start() override;

    /**
     * @brief Actualiza el estado del componente SpriteRenderer.
     * @param deltaTime Tiempo transcurrido desde la última actualización.
     */
    void update(float deltaTime) override;

private:
    void updateBounds();
};

/**
 * @brief Clase TextureManager para gestionar la carga y almacenamiento de texturas.
 */
class TextureManager {
private:
    SDL_Renderer* renderer;                         /**< Renderizador SDL asociado. */
    std::unordered_map<std::string, SDL_Texture*> textureCache; /**< Caché de texturas cargadas. */
    static TextureManager* instance;                /**< Instancia única de TextureManager. */

    /**
     * @brief Constructor privado de TextureManager.
     * @param r Puntero al renderizador SDL.
     */
    TextureManager(SDL_Renderer* r);

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

public:
    /** @brief Destructor de TextureManager que limpia las texturas cargadas. */
    ~TextureManager();

    /**
     * @brief Obtiene la instancia única de TextureManager.
     * @param renderer Puntero al renderizador SDL.
     * @return Puntero a la instancia de TextureManager.
     */
    static TextureManager* getInstance(SDL_Renderer* renderer);

    static TextureManager* getInstance();

    /**
     * @brief Carga una textura BMP desde un archivo.
     * @param filePath Ruta al archivo BMP.
     * @return Puntero a la textura SDL cargada, o nullptr si falla.
     */
    SDL_Texture* loadBMP(std::string& filePath);

    /**
     * @brief Carga una textura PNG desde un archivo.
     * @param filePath Ruta al archivo PNG.
     * @return Puntero a la textura SDL cargada, o nullptr si falla.
     */
    SDL_Texture* loadPNG(std::string& filePath);

    /**
     * @brief Obtiene una textura del caché o la carga si no existe.
     * @param path Ruta al archivo de la textura.
     * @return Puntero a la textura SDL, o nullptr si falla.
     */
    SDL_Texture* getTexture(std::string& path);
};

/**
 * @brief Clase SpriteRendererLoader para cargar parámetros de SpriteRenderer desde JSON.
 */
class SpriteRendererLoader {
private:
    static TextureManager* textureManager; /**< Instancia de TextureManager para cargar texturas. */

public:
    /**
     * @brief Inicializa SpriteRendererLoader con un renderizador SDL.
     * @param renderer Puntero al renderizador SDL.
     */
    static void initialize(SDL_Renderer* renderer);

    /** @brief Limpia la instancia de TextureManager. */
    static void cleanup();

    /**
     * @brief Convierte un objeto JSON en una instancia de SpriteRendererParameters.
     * @param j Objeto JSON con los datos.
     * @param entityManager Instancia de EntityManager.
     * @return Instancia de SpriteRendererParameters generada desde JSON.
     */
    static SpriteRendererParameters fromJSON(const nlohmann::json& j, const EntityManager& entityManager);

    /**
     * @brief Crea un componente SpriteRenderer directamente desde JSON.
     * @param j Objeto JSON con los datos.
     * @param entityManager Instancia de EntityManager.
     * @return Componente SpriteRenderer creado.
     */
    static SpriteRenderer createFromJSON(const nlohmann::json& j, const EntityManager& entityManager);
};

#endif