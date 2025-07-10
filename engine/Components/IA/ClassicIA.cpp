#include "IAComponents.h"

ClassicIAComponent::ClassicIAComponent(ClassicIAParameters cIAp) 
    : IAComponent("ClassicFSM") {
    // Inicializar los estados desde los parámetros
    for (const auto& stateName : cIAp.states_names) {
        ClassicIAState state;
        state.name = stateName;
        
        // Si hay transiciones definidas para este estado, agregarlas
        if (cIAp.transitions_per_state.find(stateName) != cIAp.transitions_per_state.end()) {
            for(auto nextState : cIAp.transitions_per_state[stateName]) {
                ClassicIATransition transition(stateName, nextState);
                state.transitions.push_back(transition);
            }
        }
        
        states[stateName] = state;
    }

    // Establecer el estado inicial como el primero de la lista o "Idle" si está vacía
    currentState = cIAp.states_names.empty() ? "Idle" : cIAp.states_names[0];
}

void ClassicIAComponent::start() {
    // No se necesita acción adicional aquí, ya que la inicialización ocurre en el constructor
}

void ClassicIAComponent::update(float deltaTime) {
    if (states.find(currentState) != states.end()) {
        ClassicIAState& current = states[currentState];
        
        // Ejecuta la acción del estado actual, si existe
        if (current.action) {
            current.action(entityManager, deltaTime);
        }

        // Evalúa las transiciones
        for (auto& transition : current.transitions) {
            if (transition.conditionCallback && 
                transition.conditionCallback(entityManager,deltaTime)) {
                currentState = transition.to;
                break; // Solo una transición por actualización
            }
        }
    }
}


void ClassicIAComponent::render(EntityManager& entityManager, SDL_Renderer* renderer) {
    // Opcional: Renderizar información de depuración (estado actual, etc.)
}

void ClassicIAComponent::registerAction(const std::string& name, std::function<void(EntityManager*,float)> callback) {
    if (states.find(name) != states.end()) {
        states[name].action = callback;
    }
}

void ClassicIAComponent::registerCondition(const std::string& from, const std::string& to, std::function<bool(EntityManager*,float)> callback) {
    if (states.find(from) != states.end()) {
        for (auto& transition : states[from].transitions) {
            if (transition.from == from && transition.to == to) {
                transition.conditionCallback = callback;
                break;
            }
        }
    }
}

/**
 * @brief Implementación de ClassicLoader
 */
ClassicIAParameters ClassicIALoader::fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
    ClassicIAParameters params;

    params.entityManager = &entityManager;

    // Cargar nombres de estados y transiciones desde JSON
    if (j.contains("states")) {
        for (const auto& stateJson : j["states"]) {
            std::string stateName = stateJson["name"].get<std::string>();
            params.states_names.push_back(stateName);

            // Cargar transiciones para este estado
            if (stateJson.contains("transitions")) {
                for (const auto& trans : stateJson["transitions"]) {
                    std::string nextState = trans["nextState"].get<std::string>();
                    // Solo se guarda la última transición si hay múltiples debido al diseño del mapa
                    params.transitions_per_state[stateName].push_back(nextState);
                }
            }
        }
    }

    return params;
}

ClassicIAComponent ClassicIALoader::createFromJSON(const nlohmann::json& j, EntityManager& entityManager) {
    return ClassicIAComponent(fromJSON(j, entityManager));
}