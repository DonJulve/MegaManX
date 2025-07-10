#ifndef DYNAMICINPUTMANAGER_H
#define DYNAMICINPUTMANAGER_H

#include <unordered_map>
#include <string>
#include <SDL2/SDL.h>
#include <nlohmann/json.hpp>

using json = nlohmann::ordered_json;

/// @brief Clase que gestiona entradas dinámicas para acciones del juego.
/// @details Permite vincular teclas y botones del ratón a acciones, verificar su estado y guardar/cargar configuraciones en JSON.
class DynamicInputManager {
public:
    /// @brief Estructura que define una vinculación de entrada (tecla o botón del ratón).
    struct InputBinding {
        SDL_Scancode key;       /**< Código de la tecla SDL (si aplica). */
        Uint8 mouseButton;      /**< Identificador del botón del ratón (si aplica). */
        bool isMouseInput;      /**< Indica si la entrada es un botón del ratón. */
        
        /// @brief Constructor para vinculación de tecla.
        /// @param k Código de la tecla SDL (por defecto SDL_SCANCODE_UNKNOWN).
        InputBinding(SDL_Scancode k = SDL_SCANCODE_UNKNOWN);
        
        /// @brief Constructor para vinculación de botón del ratón.
        /// @param btn Identificador del botón del ratón.
        InputBinding(Uint8 btn);
    };
    
    /// @brief Obtiene la instancia única de DynamicInputManager (patrón Singleton).
    /// @return Referencia a la instancia única de DynamicInputManager.
    static DynamicInputManager& getInstance();

    DynamicInputManager(const DynamicInputManager&) = delete;
    DynamicInputManager& operator=(const DynamicInputManager&) = delete;
    
    /// @brief Vincula una acción a una tecla específica.
    /// @param actionName Nombre de la acción a vincular.
    /// @param key Código de la tecla SDL a asociar.
    void bindAction(const std::string& actionName, SDL_Scancode key);
    
    /// @brief Vincula una acción a un botón del ratón específico.
    /// @param actionName Nombre de la acción a vincular.
    /// @param mouseButton Identificador del botón del ratón a asociar.
    void bindAction(const std::string& actionName, Uint8 mouseButton);
    
    /// @brief Verifica si una acción está siendo presionada en el frame actual.
    /// @param actionName Nombre de la acción a verificar.
    /// @return Verdadero si la acción está presionada, falso en caso contrario.
    bool isActionPressed(const std::string& actionName);
    
    /// @brief Verifica si una acción está siendo mantenida presionada.
    /// @param actionName Nombre de la acción a verificar.
    /// @return Verdadero si la acción está mantenida, falso en caso contrario.
    bool isActionHeld(const std::string& actionName);
    
    /// @brief Verifica si una acción ha sido liberada en el frame actual.
    /// @param actionName Nombre de la acción a verificar.
    /// @return Verdadero si la acción ha sido liberada, falso en caso contrario.
    bool isActionReleased(const std::string& actionName);
    
    /// @brief Elimina la vinculación de una acción específica.
    /// @param actionName Nombre de la acción a desvincular.
    void unbindAction(const std::string& actionName);
    
    /// @brief Elimina todas las vinculaciones de entrada.
    void clearBindings();
    
    /// @brief Guarda las vinculaciones actuales en un archivo JSON.
    /// @param filename Nombre del archivo donde se guardarán las vinculaciones.
    /// @return Verdadero si se guardó correctamente, falso en caso de error.
    bool saveToJson(const std::string& filename);
    
    /// @brief Carga vinculaciones desde un archivo JSON.
    /// @param filename Nombre del archivo desde el cual cargar las vinculaciones.
    /// @return Verdadero si se cargó correctamente, falso en caso de error.
    bool loadFromJson(const std::string& filename);

    SDL_Scancode getBind(const std::string& actionName);

private:
    /// @brief Constructor privado para implementar el patrón Singleton.
    DynamicInputManager();
    
    std::unordered_map<std::string, InputBinding> bindings; ///< Mapa de acciones a sus vinculaciones de entrada.

    /// @brief Convierte un identificador de botón del ratón a su nombre en texto.
    /// @param button Identificador del botón del ratón.
    /// @return Nombre textual del botón del ratón.
    static std::string mouseButtonToName(Uint8 button);
    
    /// @brief Convierte un nombre de botón del ratón a su identificador.
    /// @param name Nombre textual del botón del ratón.
    /// @return Identificador del botón del ratón (0 si es inválido).
    static Uint8 nameToMouseButton(const std::string& name);
};

#endif