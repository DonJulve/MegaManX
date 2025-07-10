#ifndef IACOMPONENT_H
#define IACOMPONENT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <functional>
#include <memory>  // Para std::shared_ptr
#include "Debug.h"
#include <string>
#include "EntityManager.h"
#include <nlohmann/json.hpp>

struct IAComponent : public Component {
    protected:
        std::string iaType; ///< Tipo de IA (para identificación o depuración)
        EntityManager* entityManager; ///< Puntero a EntityManager para acceder a otras entidades
    
    public:
        IAComponent(const std::string& type) : iaType(type) {}
        std::string getIAType() const { return iaType; }
        virtual ~IAComponent() = default;
};

/**
 * @brief Estructura que contiene parámetros para el componente ClassicIAComponent.
 */
struct ClassicIAParameters {

    std::vector<std::string> states_names;
    std::unordered_map<std::string,std::vector<std::string>> transitions_per_state;
    EntityManager* entityManager;

    ClassicIAParameters() {}
};


/// @brief Componente de inteligencia artificial clásica basado en máquina de estados.
/// @details Hereda de IAComponent y gestiona estados, acciones y transiciones para la lógica de IA.
struct ClassicIAComponent : public IAComponent {
        
    /// @brief Estructura que representa una transición entre estados en la IA clásica.
    /// @details Contiene el estado origen, destino y una condición para realizar la transición.
    struct ClassicIATransition {
        std::string from; ///< Estado de origen de la transición.
        std::string to;   ///< Estado de destino de la transición.
        std::function<bool(EntityManager*, float)> conditionCallback; ///< Callback que evalúa si se debe realizar la transición.

        /// @brief Constructor de IATransition.
        /// @param f Nombre del estado de origen (por defecto vacío).
        /// @param t Nombre del estado de destino (por defecto vacío).
        ClassicIATransition(const std::string& f = "", const std::string& t = "") 
            : from(f), to(t), conditionCallback(nullptr) {}
    };

    /// @brief Estructura que define un estado en la máquina de estados de la IA.
    /// @details Incluye el nombre del estado, una acción asociada y una lista de transiciones posibles.
    struct ClassicIAState {
        std::string name; ///< Nombre del estado.
        std::function<void(EntityManager*, float)> action; ///< Acción a ejecutar mientras está en este estado.
        std::vector<ClassicIATransition> transitions; ///< Lista de transiciones posibles desde este estado.

        /// @brief Constructor por defecto de IAState.
        ClassicIAState() : name(""), action(nullptr) {}
    };
    private:
        
    
        std::unordered_map<std::string, ClassicIAState> states;
        std::string currentState;
    
    public:
        /// @brief Constructor de ClassicIAComponent.
        /// @param cIAp Parámetros iniciales para configurar la IA clásica.
        ClassicIAComponent(ClassicIAParameters cIAp);

        /// @brief Inicia el componente de IA.
        void start() override;

        /// @brief Actualiza el estado de la IA en función del tiempo transcurrido.
        /// @param deltaTime Tiempo transcurrido desde la última actualización (en segundos).
        void update(float deltaTime) override;

        /// @brief Renderiza el componente de IA (si aplica).
        /// @param entityManager Referencia al administrador de entidades.
        /// @param renderer Puntero al renderizador SDL.
        void render(EntityManager& entityManager, SDL_Renderer* renderer) override;

        /// @brief Establece el estado inicial de la IA.
        /// @param state Nombre del estado inicial.
        void setInitialState(const std::string& state) { currentState = state; }

        /// @brief Registra una acción asociada a un estado.
        /// @param name Nombre del estado.
        /// @param callback Función que define la acción a ejecutar en el estado.
        void registerAction(const std::string& name, std::function<void(EntityManager*, float)> callback);

        /// @brief Registra una condición de transición entre dos estados.
        /// @param from Nombre del estado de origen.
        /// @param to Nombre del estado de destino.
        /// @param callback Función que evalúa si se cumple la condición para la transición.
        void registerCondition(const std::string& from, const std::string& to, std::function<bool(EntityManager*, float)> callback);
};

/// @brief Clase utilitaria para cargar componentes de IA clásica desde JSON.
/// @details Proporciona métodos estáticos para crear parámetros y componentes ClassicIA desde datos JSON.
class ClassicIALoader {
public:
    /// @brief Crea parámetros de IA clásica a partir de un objeto JSON.
    /// @param j Objeto JSON con los datos de configuración.
    /// @param entityManager Referencia al administrador de entidades.
    /// @return ClassicIAParameters generados desde el JSON.
    static ClassicIAParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager);

    /// @brief Crea un componente ClassicIAComponent directamente desde JSON.
    /// @param j Objeto JSON con los datos de configuración.
    /// @param entityManager Referencia al administrador de entidades.
    /// @return ClassicIAComponent creado con los datos del JSON.
    static ClassicIAComponent createFromJSON(const nlohmann::json& j, EntityManager& entityManager);
};


#endif // !IA_COMPONENT_H
