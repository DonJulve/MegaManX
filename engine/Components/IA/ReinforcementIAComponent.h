#pragma once
#include "IAComponents.h"
#include <unordered_map>
#include <vector>
#include <functional>
#include <random>
#include <nlohmann/json.hpp>

struct ReinforcementIAState {
    std::string name;
    std::vector<std::string> actions; // Available actions in this state
    std::unordered_map<std::string, std::function<void(EntityManager*, float)>> actionCallbacks; // Action-specific callbacks
};

struct ReinforcementIAParameters {
    EntityManager* entityManager;
    std::vector<std::string> states_names;
    std::unordered_map<std::string, std::vector<std::string>> actions_per_state;
    float learning_rate = 0.1f;
    float discount_factor = 0.9f;
    float epsilon = 0.1f; // For epsilon-greedy exploration
    float epsilon_decay = 0.995f; // Epsilon decay rate
    float epsilon_min = 0.01f; // Minimum epsilon value

    
    std::string qTableFile; // File to save/load Q-table
};

class ReinforcementIAComponent : public IAComponent {
public:
    ReinforcementIAComponent(ReinforcementIAParameters rIAp);
    void start() override;
    void update(float deltaTime) override;
    void render(EntityManager& entityManager, SDL_Renderer* renderer) override;
    void registerAction(const std::string& state, const std::string& actionName,
                       std::function<void(EntityManager*, float)> callback);
    void registerRewardFunction(std::function<float(EntityManager*, const std::string&, const std::string&, float)> callback);
    void registerTransitionFunction(std::function<std::string(EntityManager*, const std::string&, const std::string&)> callback);

    void saveQTable(const std::string& filename);
    void loadQTable(const std::string& filename);

private:
    struct pair_hash {
        template <class T1, class T2>
        std::size_t operator()(const std::pair<T1, T2>& p) const {
            auto h1 = std::hash<T1>{}(p.first);
            auto h2 = std::hash<T2>{}(p.second);
            return h1 ^ (h2 << 1);
        }
    };

    std::string selectAction(const std::string& state);
    float computeReward(const std::string& state, const std::string& action, float deltaTime);
    std::string determineNextState(const std::string& currentState, const std::string& action);
    void updateQTable(const std::string& state, const std::string& action, float reward, const std::string& nextState);

    

    ReinforcementIAParameters params;
    std::string qTableFile; // File to save/load Q-table
    std::unordered_map<std::string, ReinforcementIAState> states;
    std::string currentState;
    std::unordered_map<std::pair<std::string, std::string>, float, pair_hash> qTable;
    std::unordered_map<std::string, float> maxQCache; // Cache for max Q-values per state
    std::function<float(EntityManager*, const std::string&, const std::string&, float)> rewardCallback;
    std::function<std::string(EntityManager*, const std::string&, const std::string&)> transitionCallback;
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;
};

class ReinforcementIALoader {
public:
    static ReinforcementIAParameters fromJSON(const nlohmann::json& j, EntityManager& entityManager);
    static ReinforcementIAComponent createFromJSON(const nlohmann::json& j, EntityManager& entityManager);
};