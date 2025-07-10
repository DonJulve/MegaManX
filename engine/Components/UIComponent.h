#ifndef UI_COMPONENT_H
#define UI_COMPONENT_H

#include <nlohmann/json.hpp>
#include "Components.h"

/// @brief Estructura que contiene los parámetros necesarios para configurar un componente de interfaz de usuario.
/// @details Esta estructura almacena información como la transformación asociada al componente UI.
struct UIParameters {
    Transform* transform; ///< Puntero a la transformación del componente UI.

    /// @brief Constructor de UIParameters.
    /// @param t Puntero a la transformación que se asociará al componente UI.
    UIParameters(Transform* t) : transform(t) {};
};

/// @brief Componente de interfaz de usuario que hereda de Component.
/// @details Representa un elemento de la interfaz de usuario con una transformación asociada.
struct UIComponent : public Component {
    
    Transform* transform; ///< Puntero a la transformación del componente UI.

    /// @brief Constructor de UIComponent.
    /// @param uip Parámetros de UI que contienen la transformación inicial.
    UIComponent(UIParameters uip) : transform(uip.transform) {};

    /// @brief Renderiza el componente UI en pantalla.
    /// @param entityManager Referencia al administrador de entidades que gestiona los componentes.
    /// @param renderer Puntero al renderizador SDL utilizado para dibujar el componente.
    void render(EntityManager& entityManager, SDL_Renderer* renderer) override;
};

/// @brief Clase utilitaria para cargar componentes UI desde datos JSON.
/// @details Proporciona métodos estáticos para crear parámetros y componentes UI a partir de JSON.
class UILoader {
public:
    /// @brief Crea parámetros UI a partir de un objeto JSON.
    /// @param j Objeto JSON que contiene los datos de configuración.
    /// @param entityManager Referencia al administrador de entidades para acceder a los componentes necesarios.
    /// @return UIParameters con los datos extraídos del JSON.
    static UIParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager);

    /// @brief Crea un componente UI a partir de un objeto JSON.
    /// @param j Objeto JSON que contiene los datos de configuración.
    /// @param entityManager Referencia al administrador de entidades para asociar el componente.
    /// @return UIComponent creado con los datos del JSON.
    static UIComponent createFromJSON(nlohmann::json& j, EntityManager& entityManager);
};

#endif // UI_COMPONENT_H