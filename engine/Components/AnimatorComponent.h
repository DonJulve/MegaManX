#ifndef ANIMATOR_COMPONENT_H
#define ANIMATOR_COMPONENT_H

#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "Components.h"
#include <nlohmann/json.hpp>
#include <filesystem>

namespace fs = std::filesystem;

struct AnimatorParameter;
struct Transition;
struct State;
struct AnimatorComponent;
struct AnimatorParameters;

/**
 * @struct AnimatorParameter
 * @brief Estructura que representa un parámetro de animación con diferentes tipos de valores.
 */
struct AnimatorParameter {
    enum class Type { BOOL, INT, FLOAT, TRIGGER };
    Type type;
    std::string name;
    union {
        bool boolValue;
        int intValue;
        float floatValue;
    };

    AnimatorParameter() : type(Type::BOOL), name(""), boolValue(false) {}
    AnimatorParameter(const std::string& n, bool value);
    AnimatorParameter(const std::string& n, int value);
    AnimatorParameter(const std::string& n, float value);
    AnimatorParameter(const std::string& n);
};

/**
 * @struct Transition
 * @brief Estructura que define una transición entre estados en la máquina de estados.
 */
struct Transition {
    std::string fromState;
    std::string toState;
    bool keepFrame = false;
    bool waitEndState = false;

    /**
     * @struct Condition
     * @brief Condición para evaluar una transición.
     */
    struct Condition {
        std::string parameterName;
        enum class Mode { EQ, NEQ, LT, GT, LTE, GTE };
        Mode mode;
        float value;

        bool evaluate(const std::unordered_map<std::string, AnimatorParameter>& params) const;
    };
    std::vector<Condition> conditions;

    Transition(const std::string& from, const std::string& to, bool keepF, bool waitEndS);
};

/**
 * @struct AnimatorComponent
 * @brief Componente que maneja la lógica de animación para una entidad.
 * @ingroup Components
 */
struct AnimatorComponent : public Component {
    SpriteRenderer* spriteRenderer;

    /**
     * @struct AnimationFrame
     * @brief Representa un cuadro individual en una animación.
     */
    struct AnimationFrame {
        SDL_Texture* texture;
        float duration;
        int width, height;

        AnimationFrame(SDL_Texture* tex = nullptr, float dur = 0.1f);
    };

    /**
     * @struct State
     * @brief Representa un estado en la máquina de estados de animación.
     */
    struct State {
        std::string name;
        std::vector<AnimationFrame> frames;
        std::unordered_map<int, std::function<void()>> frameTriggers;  // Map of frame index to trigger function
        int currentFrame = 0;
        float timeAccumulator = 0.0f;
        bool isLooping = true;

        State() : name(""), isLooping(true) {}
        State(const std::string& stateName, bool loop = true);
        bool update(float deltaTime, SDL_Texture*& outTexture, int& outWidth, int& outHeight);
    };

    /**
     * @struct StateMachine
     * @brief Máquina de estados que controla las transiciones y parámetros.
     */
    struct StateMachine {
        std::unordered_map<std::string, State> states;
        std::string currentState;
        std::vector<Transition> transitions;
        std::unordered_map<std::string, AnimatorParameter> parameters;
    } stateMachine;

    AnimatorComponent(AnimatorParameters params);
    void start() override;
    void update(float deltaTime) override;

    void setBool(const std::string& name, bool value);
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setTrigger(const std::string& name);

    bool getBool(const std::string& name);
    int getInt(const std::string& name);
    float getFloat(const std::string& name);

    /**
     * @brief Obtiene el nombre del estado actual.
     * @return Nombre del estado actual.
     */
    std::string currentState() const { return stateMachine.currentState; }

    /**
     * @brief Resetea el estado actual a su primer frame.
     */
    void restart_current_state();

    /**
     * @brief Agrega una función que se triggerá cuando se alcance un frame específico en un estado.
     * @param stateName Nombre del estado.
     * @param frameIndex Índice del frame.
     * @param trigger Función a ejecutar cuando se alcance el frame.
     */
    void addFrameTrigger(const std::string& stateName, int frameIndex, std::function<void()> trigger);
};

/**
 * @struct AnimatorParameters
 * @brief Estructura que contiene los parámetros necesarios para configurar un AnimatorComponent.
 */
struct AnimatorParameters {
    std::unordered_map<std::string, AnimatorComponent::State> states;
    std::vector<Transition> transitions;
    std::string initialState;
    std::unordered_map<std::string, AnimatorParameter> parameters;
    SpriteRenderer* spriteRenderer;

    AnimatorParameters(
        const std::unordered_map<std::string, AnimatorComponent::State>& s,
        const std::vector<Transition>& t,
        const std::string& init = "",
        const std::unordered_map<std::string, AnimatorParameter>& p = {},
        SpriteRenderer* sr = nullptr
    );
};

/**
 * @class AnimatorLoader
 * @brief Clase encargada de cargar configuraciones de animación desde JSON.
 */
class AnimatorLoader {
public:
    static AnimatorParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager);
    static AnimatorComponent createFromJSON(nlohmann::json& j, EntityManager& entityManager);
};

#endif // ANIMATOR_COMPONENT_H