#include "ReinforcementIAComponent.h"
#include <limits>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

ReinforcementIAComponent::ReinforcementIAComponent(ReinforcementIAParameters rIAp)
    : IAComponent("ReinforcementFSM"), params(rIAp) {
    std::random_device rd;
    rng = std::mt19937(rd());
    dist = std::uniform_real_distribution<float>(0.0f, 1.0f);

    qTableFile = rIAp.qTableFile;
    if (qTableFile.empty()) {
        Debug::Log("Warning: Q-table file not specified, using default 'q_table.json'");
        qTableFile = "q_table.json";
    }

    for (const auto& stateName : rIAp.states_names) {
        ReinforcementIAState state;
        state.name = stateName;
        if (rIAp.actions_per_state.find(stateName) != rIAp.actions_per_state.end()) {
            state.actions = rIAp.actions_per_state[stateName];
        }
        states[stateName] = state;

        for (const auto& action : state.actions) {
            qTable[{stateName, action}] = 0.0f;
        }
        maxQCache[stateName] = 0.0f; // Initialize max Q-value cache
    }

    currentState = rIAp.states_names.empty() ? "Idle" : rIAp.states_names[0];
    if (!rIAp.states_names.empty() && states.find("Idle") == states.end() && currentState == "Idle") {
        Debug::Log("Warning: 'Idle' state not found in states_names, but set as default");
    }
}

void ReinforcementIAComponent::start() {
    // If qTableFile exists in disk, load the Q-table
    std::ifstream file(qTableFile);
    if (file.is_open()) {
        file.close();
        loadQTable(qTableFile);
    } else {
        Debug::Log("Warning: Q-table file '" + qTableFile + "' not found, starting with empty Q-table");
    }
}

void ReinforcementIAComponent::update(float deltaTime) {
    if (states.find(currentState) == states.end()) {
        Debug::Log("Error: Invalid current state '" + currentState + "'");
        return;
    }

    auto& current = states[currentState];
    std::string selectedAction = selectAction(currentState);

    if (!selectedAction.empty() && current.actionCallbacks.find(selectedAction) != current.actionCallbacks.end()) {
        current.actionCallbacks[selectedAction](params.entityManager, deltaTime);
    } else if (!selectedAction.empty()) {
        Debug::Log("Warning: No callback registered for action '" + selectedAction + "' in state '" + currentState + "'");
    }

    float reward = computeReward(currentState, selectedAction, deltaTime);
    std::string nextState = determineNextState(currentState, selectedAction);
    updateQTable(currentState, selectedAction, reward, nextState);
    currentState = nextState;

    // Apply epsilon decay
    params.epsilon = std::max(params.epsilon * params.epsilon_decay, params.epsilon_min);
}

void ReinforcementIAComponent::render(EntityManager& entityManager, SDL_Renderer* renderer) {
}

void ReinforcementIAComponent::registerAction(const std::string& state, const std::string& actionName,
                                             std::function<void(EntityManager*, float)> callback) {
    if (states.find(state) == states.end()) {
        Debug::Log("Warning: State '" + state + "' not found when registering action '" + actionName + "'");
        return;
    }
    if (actionName.empty()) {
        Debug::Log("Warning: Empty action name provided for state '" + state + "'");
        return;
    }
    states[state].actionCallbacks[actionName] = callback;
    if (std::find(states[state].actions.begin(), states[state].actions.end(), actionName) == states[state].actions.end()) {
        states[state].actions.push_back(actionName);
        qTable[{state, actionName}] = 0.0f;
        maxQCache[state] = 0.0f; // Reset max Q-value cache
    }
}

void ReinforcementIAComponent::registerRewardFunction(std::function<float(EntityManager*, const std::string&, const std::string&, float)> callback) {
    rewardCallback = callback;
}

void ReinforcementIAComponent::registerTransitionFunction(std::function<std::string(EntityManager*, const std::string&, const std::string&)> callback) {
    transitionCallback = callback;
}

void ReinforcementIAComponent::saveQTable(const std::string& filename) {
    nlohmann::json j;
    j["q_table"] = nlohmann::json::array();

    for (const auto& [state_action, q_value] : qTable) {
        nlohmann::json entry;
        entry["state"] = state_action.first;
        entry["action"] = state_action.second;
        entry["q_value"] = q_value;
        j["q_table"].push_back(entry);
    }

    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            Debug::Log("Error: Failed to open file '" + filename + "' for writing Q-table");
            return;
        }
        file << j.dump(4); // Pretty print with 4-space indentation
        file.close();
        std::cout << "Q-table successfully saved to '" << filename << "'" << std::endl;
    } catch (const std::exception& e) {
        Debug::Log("Error: Failed to save Q-table to '" + filename + "': " + e.what());
    }
}

void ReinforcementIAComponent::loadQTable(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            Debug::Log("Error: Failed to open file '" + filename + "' for reading Q-table");
            return;
        }

        nlohmann::json j;
        file >> j;
        file.close();

        if (!j.contains("q_table") || !j["q_table"].is_array()) {
            Debug::Log("Error: Invalid Q-table format in '" + filename + "': missing or invalid 'q_table' array");
            return;
        }

        // Clear existing Q-table and maxQCache to avoid stale data
        qTable.clear();
        maxQCache.clear();

        for (const auto& entry : j["q_table"]) {
            if (!entry.contains("state") || !entry.contains("action") || !entry.contains("q_value")) {
                Debug::Log("Warning: Skipping invalid Q-table entry: missing state, action, or q_value");
                continue;
            }

            std::string state = entry["state"].get<std::string>();
            std::string action = entry["action"].get<std::string>();
            float q_value = entry["q_value"].get<float>();

            // Validate state and action
            if (states.find(state) == states.end()) {
                Debug::Log("Warning: Skipping Q-table entry for unknown state '" + state + "'");
                continue;
            }
            if (std::find(states[state].actions.begin(), states[state].actions.end(), action) == states[state].actions.end()) {
                Debug::Log("Warning: Skipping Q-table entry for unknown action '" + action + "' in state '" + state + "'");
                continue;
            }

            qTable[{state, action}] = q_value;

            // Update maxQCache for the state
            if (q_value > maxQCache[state]) {
                maxQCache[state] = q_value;
            }
        }

        // Initialize maxQCache for states without Q-values
        for (const auto& [state, state_data] : states) {
            if (maxQCache.find(state) == maxQCache.end()) {
                maxQCache[state] = 0.0f;
            }
        }

        std::cout << "Q-table successfully loaded from '" << filename << "'" << std::endl;
    } catch (const std::exception& e) {
        Debug::Log("Error: Failed to load Q-table from '" + filename + "': " + e.what());
    }
}

std::string ReinforcementIAComponent::selectAction(const std::string& state) {
    if (states.find(state) == states.end() || states[state].actions.empty()) {
        Debug::Log("Warning: Invalid state '" + state + "' or no actions available");
        return "";
    }

    if (dist(rng) < params.epsilon) {
        std::uniform_int_distribution<size_t> actionDist(0, states[state].actions.size() - 1);
        return states[state].actions[actionDist(rng)];
    } else {
        std::vector<std::string> bestActions;
        float maxQ = -std::numeric_limits<float>::infinity();
        for (const auto& action : states[state].actions) {
            float qValue = qTable[{state, action}];
            if (qValue > maxQ) {
                maxQ = qValue;
                bestActions = {action};
            } else if (qValue == maxQ) {
                bestActions.push_back(action);
            }
        }
        if (bestActions.empty()) {
            Debug::Log("Warning: No valid actions found for state '" + state + "'");
            return "";
        }
        std::uniform_int_distribution<size_t> actionDist(0, bestActions.size() - 1);
        maxQCache[state] = maxQ; // Update max Q-value cache
        return bestActions[actionDist(rng)];
    }
}

float ReinforcementIAComponent::computeReward(const std::string& state, const std::string& action, float deltaTime) {
    if (!rewardCallback) {
        Debug::Log("Warning: No reward callback registered, returning default reward 0.0");
        return 0.0f;
    }
    return rewardCallback(params.entityManager, state, action, deltaTime);
}

std::string ReinforcementIAComponent::determineNextState(const std::string& currentState, const std::string& action) {
    if (transitionCallback) {
        std::string nextState = transitionCallback(params.entityManager, currentState, action);
        if (states.find(nextState) == states.end()) {
            Debug::Log("Warning: Transition callback returned invalid state '" + nextState + "'");
            return currentState;
        }
        return nextState;
    }
    // Fallback to original linear progression
    auto it = std::find(params.states_names.begin(), params.states_names.end(), currentState);
    if (it != params.states_names.end() && it + 1 != params.states_names.end()) {
        return *(it + 1);
    }
    return currentState;
}

void ReinforcementIAComponent::updateQTable(const std::string& state, const std::string& action, float reward, const std::string& nextState) {
    if (states.find(nextState) == states.end()) {
        Debug::Log("Warning: Invalid next state '" + nextState + "' in updateQTable");
        return;
    }

    float maxNextQ = maxQCache[nextState]; // Use cached max Q-value
    if (states[nextState].actions.empty()) {
        maxNextQ = 0.0f;
    }

    float& currentQ = qTable[{state, action}];
    currentQ += params.learning_rate * (reward + params.discount_factor * maxNextQ - currentQ);
}

ReinforcementIAParameters ReinforcementIALoader::fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
    ReinforcementIAParameters params;
    params.entityManager = &entityManager;

    if (j.contains("states")) {
        for (const auto& stateJson : j["states"]) {
            if (!stateJson.contains("name")) {
                Debug::Log("Warning: State JSON missing 'name' field");
                continue;
            }
            std::string stateName = stateJson["name"].get<std::string>();
            if (stateName.empty()) {
                Debug::Log("Warning: Empty state name in JSON");
                continue;
            }
            if (std::find(params.states_names.begin(), params.states_names.end(), stateName) != params.states_names.end()) {
                Debug::Log("Warning: Duplicate state name '" + stateName + "' in JSON");
                continue;
            }
            params.states_names.push_back(stateName);

            if (stateJson.contains("actions")) {
                for (const auto& action : stateJson["actions"]) {
                    std::string actionName = action.get<std::string>();
                    if (actionName.empty()) {
                        Debug::Log("Warning: Empty action name for state '" + stateName + "' in JSON");
                        continue;
                    }
                    params.actions_per_state[stateName].push_back(actionName);
                }
            }
        }
    } else {
        Debug::Log("Warning: No 'states' field in JSON configuration");
    }

    if (j.contains("learning_rate")) {
        params.learning_rate = j["learning_rate"].get<float>();
        if (params.learning_rate <= 0.0f || params.learning_rate > 1.0f) {
            Debug::Log("Warning: Invalid learning_rate " + std::to_string(params.learning_rate) + ", setting to 0.1");
            params.learning_rate = 0.1f;
        }
    }
    if (j.contains("discount_factor")) {
        params.discount_factor = j["discount_factor"].get<float>();
        if (params.discount_factor < 0.0f || params.discount_factor > 1.0f) {
            Debug::Log("Warning: Invalid discount_factor " + std::to_string(params.discount_factor) + ", setting to 0.9");
            params.discount_factor = 0.9f;
        }
    }
    if (j.contains("epsilon")) {
        params.epsilon = j["epsilon"].get<float>();
        if (params.epsilon < 0.0f || params.epsilon > 1.0f) {
            Debug::Log("Warning: Invalid epsilon " + std::to_string(params.epsilon) + ", setting to 0.1");
            params.epsilon = 0.1f;
        }
    }
    if (j.contains("epsilon_decay")) {
        params.epsilon_decay = j["epsilon_decay"].get<float>();
        if (params.epsilon_decay <= 0.0f || params.epsilon_decay > 1.0f) {
            Debug::Log("Warning: Invalid epsilon_decay " + std::to_string(params.epsilon_decay) + ", setting to 0.995");
            params.epsilon_decay = 0.995f;
        }
    }
    if (j.contains("epsilon_min")) {
        params.epsilon_min = j["epsilon_min"].get<float>();
        if (params.epsilon_min < 0.0f || params.epsilon_min > params.epsilon) {
            Debug::Log("Warning: Invalid epsilon_min " + std::to_string(params.epsilon_min) + ", setting to 0.01");
            params.epsilon_min = 0.01f;
        }
    }
    if (j.contains("qTableFile")) {
        params.qTableFile = j["qTableFile"].get<std::string>();
        if (params.qTableFile.empty()) {
            Debug::Log("Warning: Empty qTableFile in JSON, using default 'q_table.json'");
            params.qTableFile = "q_table.json";
        }
    } else {
        params.qTableFile = "q_table.json"; // Default value
    }

    return params;
}

ReinforcementIAComponent ReinforcementIALoader::createFromJSON(const nlohmann::json& j, EntityManager& entityManager) {
    return ReinforcementIAComponent(fromJSON(j, entityManager));
}