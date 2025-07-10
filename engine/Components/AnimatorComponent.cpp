#include "AnimatorComponent.h"
#include "Debug.h" // Assuming Debug::Log is defined here

AnimatorParameter::AnimatorParameter(const std::string& n, bool value) 
    : type(Type::BOOL), name(n), boolValue(value) {}

AnimatorParameter::AnimatorParameter(const std::string& n, int value) 
    : type(Type::INT), name(n), intValue(value) {}

AnimatorParameter::AnimatorParameter(const std::string& n, float value) 
    : type(Type::FLOAT), name(n), floatValue(value) {}

AnimatorParameter::AnimatorParameter(const std::string& n) 
    : type(Type::TRIGGER), name(n), boolValue(false) {}

Transition::Transition(const std::string& from, const std::string& to, bool keepF, bool waitEndS) 
    : fromState(from), toState(to), keepFrame(keepF), waitEndState(waitEndS) {}

bool Transition::Condition::evaluate(const std::unordered_map<std::string, AnimatorParameter>& params) const {
    auto it = params.find(parameterName);
    if (it == params.end()) return false;

    const AnimatorParameter& param = it->second;
    switch (param.type) {
        case AnimatorParameter::Type::BOOL: {
            bool val = value != 0;
            return mode == Mode::EQ ? param.boolValue == val :
                   mode == Mode::NEQ ? param.boolValue != val : false;
        }
        case AnimatorParameter::Type::INT: {
            int val = static_cast<int>(value);
            return mode == Mode::EQ ? param.intValue == val :
                   mode == Mode::NEQ ? param.intValue != val :
                   mode == Mode::LT ? param.intValue < val :
                   mode == Mode::GT ? param.intValue > val :
                   mode == Mode::LTE ? param.intValue <= val :
                   mode == Mode::GTE ? param.intValue >= val : false;
        }
        case AnimatorParameter::Type::FLOAT:
            return mode == Mode::EQ ? param.floatValue == value :
                   mode == Mode::NEQ ? param.floatValue != value :
                   mode == Mode::LT ? param.floatValue < value :
                   mode == Mode::GT ? param.floatValue > value :
                   mode == Mode::LTE ? param.floatValue <= value :
                   mode == Mode::GTE ? param.floatValue >= value : false;
        case AnimatorParameter::Type::TRIGGER:
            return mode == Mode::EQ && param.boolValue && value != 0;
    }
    return false;
}

AnimatorComponent::State::State(const std::string& stateName, bool loop) 
    : name(stateName), isLooping(loop) {}

bool AnimatorComponent::State::update(float deltaTime, SDL_Texture*& outTexture, int& outWidth, int& outHeight) {
    bool result = true;

    if (frames.empty()) {
        outTexture = nullptr;
        outWidth = outHeight = 0;
        return false;
    }
    
    timeAccumulator += deltaTime;
    bool frameChanged = false;

    while (timeAccumulator >= frames[currentFrame].duration) {
        timeAccumulator -= frames[currentFrame].duration;
        int previousFrame = currentFrame;
        currentFrame++;
        if (currentFrame >= frames.size()) {
            if (isLooping) {
                currentFrame = 0;
            } else {
                currentFrame--;
                result = false;
            }
        }
        frameChanged = (currentFrame != previousFrame);

        // Check and execute frame trigger if it exists
        if (frameChanged) {
            auto it = frameTriggers.find(currentFrame);
            if (it != frameTriggers.end() && it->second) {
                Debug::Log("AnimatorComponent: Triggering function on frame " + std::to_string(currentFrame) + " in state '" + name + "'");
                it->second();  // Execute the trigger function
            }
        }
    }
    
    outTexture = frames[currentFrame].texture;
    outWidth = frames[currentFrame].width;
    outHeight = frames[currentFrame].height;
    return result;
}

AnimatorParameters::AnimatorParameters(
    const std::unordered_map<std::string, AnimatorComponent::State>& s,
    const std::vector<Transition>& t,
    const std::string& init,
    const std::unordered_map<std::string, AnimatorParameter>& p,
    SpriteRenderer* sr
) : states(s), transitions(t), initialState(init), parameters(p), spriteRenderer(sr) {}

AnimatorComponent::AnimationFrame::AnimationFrame(SDL_Texture* tex, float dur) 
    : texture(tex), duration(dur) {
    if (texture) {
        SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
    } else {
        width = height = 0;
    }
}

AnimatorComponent::AnimatorComponent(AnimatorParameters params) {
    spriteRenderer = params.spriteRenderer;
    stateMachine.states = params.states;
    stateMachine.transitions = params.transitions;
    stateMachine.parameters = params.parameters;
    stateMachine.currentState = params.initialState.empty() && !params.states.empty() 
        ? params.states.begin()->first 
        : params.initialState;
}

void AnimatorComponent::start() {
    if (!spriteRenderer) return;
    if (!stateMachine.states.empty() && !stateMachine.currentState.empty()) {
        auto& initialState = stateMachine.states[stateMachine.currentState];
        if (!initialState.frames.empty()) {
            spriteRenderer->texture = initialState.frames[0].texture;
            spriteRenderer->width = initialState.frames[0].width;
            spriteRenderer->height = initialState.frames[0].height;
        }
    }
}

void AnimatorComponent::update(float deltaTime) {
    // Check initial conditions
    if (!spriteRenderer) {
        Debug::Log("AnimatorComponent: No SpriteRenderer!");
        return;
    }
    if (stateMachine.currentState.empty() || stateMachine.states.empty()) {
        Debug::Log("AnimatorComponent: Invalid state - current: " + stateMachine.currentState +
                   ", states count: " + std::to_string(stateMachine.states.size()));
        return;
    }

    State& current = stateMachine.states[stateMachine.currentState];
    SDL_Texture* currentTexture = nullptr;
    int width, height;

    // Update animation state
    bool isPlaying = current.update(deltaTime, currentTexture, width, height);

    // Update SpriteRenderer if we have a texture
    if (currentTexture) {
        spriteRenderer->texture = currentTexture;
        spriteRenderer->width = width;
        spriteRenderer->height = height;
    } else {
        Debug::Log("AnimatorComponent: No texture for state " + current.name +
                   ", frame " + std::to_string(current.currentFrame));
    }

    // Always check transitions (even if not playing, to allow state changes)
    for (auto& transition : stateMachine.transitions) {
        if (transition.fromState == stateMachine.currentState) {
            bool allConditionsMet = true;
            for (const auto& condition : transition.conditions) {
                bool result = condition.evaluate(stateMachine.parameters);
            
                if (!result) {
                    allConditionsMet = false;
                    break;
                }
                
            }

            if (stateMachine.states[stateMachine.currentState].currentFrame < stateMachine.states[stateMachine.currentState].frames.size() - 1 && transition.waitEndState) {
                allConditionsMet = false;
            }
            
            if (allConditionsMet) {
                Debug::Log("AnimatorComponent: Transitioning from " + stateMachine.currentState +
                           " to " + transition.toState);
                stateMachine.currentState = transition.toState;
                auto& newState = stateMachine.states[stateMachine.currentState];
                if(transition.keepFrame)
                {
                    newState.currentFrame = current.currentFrame;
                    newState.timeAccumulator = current.timeAccumulator;

                    auto it = newState.frameTriggers.find(newState.currentFrame);
                    if (it != newState.frameTriggers.end() && it->second) {
                        Debug::Log("AnimatorComponent: Triggering function on frame " + std::to_string(newState.currentFrame) + " in state '" + newState.name + "'");
                        it->second();  // Execute the trigger function
                    }
                }
                else
                {
                    newState.currentFrame = 0;
                    newState.timeAccumulator = 0.0f;

                    auto it = newState.frameTriggers.find(newState.currentFrame);
                    if (it != newState.frameTriggers.end() && it->second) {
                        Debug::Log("AnimatorComponent: Triggering function on frame " + std::to_string(newState.currentFrame) + " in state '" + newState.name + "'");
                        it->second();  // Execute the trigger function
                    }
                }

                if (!newState.frames.empty()) {
                    spriteRenderer->texture = newState.frames[0].texture;
                    spriteRenderer->width = newState.frames[0].width;
                    spriteRenderer->height = newState.frames[0].height;
                } else {
                    Debug::Log("AnimatorComponent: New state " + newState.name + " has no frames!");
                }
                for (auto& [name, param] : stateMachine.parameters) {
                    if (param.type == AnimatorParameter::Type::TRIGGER && param.boolValue) {
                        param.boolValue = false;
                    }
                }
                break;  // Exit after first valid transition
            }
        }
    }
}

void AnimatorComponent::setBool(const std::string& name, bool value) {
    if (auto it = stateMachine.parameters.find(name); it != stateMachine.parameters.end() &&
        it->second.type == AnimatorParameter::Type::BOOL) {
        it->second.boolValue = value;
    }
}

void AnimatorComponent::setInt(const std::string& name, int value) {
    if (auto it = stateMachine.parameters.find(name); it != stateMachine.parameters.end() &&
        it->second.type == AnimatorParameter::Type::INT) {
        it->second.intValue = value;
    }
}

void AnimatorComponent::setFloat(const std::string& name, float value) {
    if (auto it = stateMachine.parameters.find(name); it != stateMachine.parameters.end() &&
        it->second.type == AnimatorParameter::Type::FLOAT) {
        it->second.floatValue = value;
    }
}

void AnimatorComponent::setTrigger(const std::string& name) {
    if (auto it = stateMachine.parameters.find(name); it != stateMachine.parameters.end() &&
        it->second.type == AnimatorParameter::Type::TRIGGER) {
        it->second.boolValue = true;
    }
}

bool AnimatorComponent::getBool(const std::string& name) {
    if (auto it = stateMachine.parameters.find(name); it != stateMachine.parameters.end() &&
        it->second.type == AnimatorParameter::Type::BOOL) {
        return it->second.boolValue;
    }
    return false;
}

int AnimatorComponent::getInt(const std::string& name) {
    if (auto it = stateMachine.parameters.find(name); it != stateMachine.parameters.end() &&
        it->second.type == AnimatorParameter::Type::INT) {
        return it->second.intValue;
    }
    return 0;
}

float AnimatorComponent::getFloat(const std::string& name) {
    if (auto it = stateMachine.parameters.find(name); it != stateMachine.parameters.end() &&
        it->second.type == AnimatorParameter::Type::FLOAT) {
        return it->second.floatValue;
    }
    return 0;
}

void AnimatorComponent::restart_current_state() {
    auto& newState = stateMachine.states[stateMachine.currentState];
    newState.currentFrame = 0;
    newState.timeAccumulator = 0.0f;
}

void AnimatorComponent::addFrameTrigger(const std::string& stateName, int frameIndex, std::function<void()> trigger) {
    auto it = stateMachine.states.find(stateName);
    if (it == stateMachine.states.end()) {
        Debug::Log("AnimatorComponent: Cannot add frame trigger - state '" + stateName + "' not found.");
        return;
    }

    State& state = it->second;
    if (frameIndex < 0 || frameIndex >= static_cast<int>(state.frames.size())) {
        Debug::Log("AnimatorComponent: Cannot add frame trigger - frame index " + std::to_string(frameIndex) + 
                   " out of bounds for state '" + stateName + "' (frames: " + std::to_string(state.frames.size()) + ").");
        return;
    }

    state.frameTriggers[frameIndex] = trigger;
    Debug::Log("AnimatorComponent: Added frame trigger to frame " + std::to_string(frameIndex) + " in state '" + stateName + "'.");
}

AnimatorParameters AnimatorLoader::fromJSON(const nlohmann::json& j, EntityManager& entityManager) {
    std::unordered_map<std::string, AnimatorComponent::State> states;
    std::vector<Transition> transitions;
    std::unordered_map<std::string, AnimatorParameter> parameters;
    std::string initialState;

    Debug::Log("Starting AnimatorLoader::fromJSON");
    
    SpriteRenderer* spriteRenderer = entityManager.getComponent<SpriteRenderer>(entityManager.getLast());
    if (!spriteRenderer) {
        Debug::Log("Error: No SpriteRenderer component found for entity " + std::to_string(entityManager.getLast()));
        throw std::runtime_error("Animator requiere un componente SpriteRenderer");
    }
    Debug::Log("SpriteRenderer component found");

    // Parameters parsing
    if (j.contains("parameters") && j["parameters"].is_object()) {
        Debug::Log("Processing " + std::to_string(j["parameters"].size()) + " parameters");
        for (auto& [name, paramJson] : j["parameters"].items()) {
            std::string type = paramJson.value("type", "bool");
            Debug::Log("Parameter '" + name + "' type: " + type);
            
            if (type == "bool") {
                bool value = paramJson.value("value", false);
                parameters[name] = AnimatorParameter(name, value);
                Debug::Log("Added bool parameter '" + name + "' with value: " + (value ? "true" : "false"));
            } else if (type == "int") {
                int value = paramJson.value("value", 0);
                parameters[name] = AnimatorParameter(name, value);
                Debug::Log("Added int parameter '" + name + "' with value: " + std::to_string(value));
            } else if (type == "float") {
                float value = paramJson.value("value", 0.0f);
                parameters[name] = AnimatorParameter(name, value);
                Debug::Log("Added float parameter '" + name + "' with value: " + std::to_string(value));
            } else if (type == "trigger") {
                parameters[name] = AnimatorParameter(name);
                Debug::Log("Added trigger parameter '" + name + "'");
            }
        }
    }

    // States parsing
    if (j.contains("states") && j["states"].is_object()) {
        Debug::Log("Processing " + std::to_string(j["states"].size()) + " states");
        for (auto& [name, stateJson] : j["states"].items()) {
            bool looping = stateJson.value("looping", true);
            AnimatorComponent::State state(name, looping);
            Debug::Log("State '" + name + "' looping: " + (looping ? "true" : "false"));
            
            if (stateJson.contains("folder") && stateJson["folder"].is_string()) {
                std::string folderPath = "resources/" + stateJson.value("folder", "");
                float duration = stateJson.value("duration", 0.1f); // Single duration for all frames
                Debug::Log("Processing frames from folder '" + folderPath + "' with duration: " + std::to_string(duration));
            
                std::vector<std::string> textureFiles;
                try {
                    // Collect all files in the folder
                    for (const auto& entry : fs::directory_iterator(folderPath)) {
                        if (entry.is_regular_file()) {
                            textureFiles.push_back(entry.path().string());
                        }
                    }
                    // Sort files for consistent ordering
                    std::sort(textureFiles.begin(), textureFiles.end());
                    
                    Debug::Log("Found " + std::to_string(textureFiles.size()) + " files in folder '" + folderPath + "'");
                    
                    // Process each file as a frame
                    for (auto& texturePath : textureFiles) {
                        SDL_Texture* texture = TextureManager::getInstance()->getTexture(texturePath);
                        Debug::Log("Loaded texture from path: " + texturePath + " (" + (texture ? "success" : "failed") + ")");
                        if (texture) {
                            state.frames.push_back(AnimatorComponent::AnimationFrame(texture, duration));
                            Debug::Log("Added frame from '" + texturePath + "' with duration: " + std::to_string(duration));
                        }
                    }
                } catch (const fs::filesystem_error& e) {
                    Debug::Log("Error accessing folder '" + folderPath + "': " + e.what());
                }
            }
            states[name] = state;
        }
    }

    // Transitions parsing
    if (j.contains("transitions") && j["transitions"].is_array()) {
        Debug::Log("Processing " + std::to_string(j["transitions"].size()) + " transitions");
        for (const auto& transJson : j["transitions"]) {
            std::string from = transJson.value("from", "");
            std::string to = transJson.value("to", "");
            bool keepFrame = transJson.value("keepFrame", false);
            Debug::Log("Keep frame: " + std::to_string(keepFrame));
            bool waitEndState = transJson.value("waitEndState", false);
            Debug::Log("Wait end state: " + std::to_string(waitEndState));
            Transition trans(from, to, keepFrame,waitEndState);
            Debug::Log("Transition from '" + from + "' to '" + to + "'" + (keepFrame ? " keeping frame" : "") + (waitEndState ? " waiting end state" : ""));
            
            if (transJson.contains("conditions") && transJson["conditions"].is_array()) {
                Debug::Log("Processing " + std::to_string(transJson["conditions"].size()) + " conditions");
                for (const auto& condJson : transJson["conditions"]) {
                    Transition::Condition cond;
                    cond.parameterName = condJson.value("parameter", "");
                    std::string mode = condJson.value("mode", "EQ");
                    Debug::Log("Condition on parameter '" + cond.parameterName + "' mode: " + mode);
                    
                    if (mode == "EQ") cond.mode = Transition::Condition::Mode::EQ;
                    else if (mode == "NEQ") cond.mode = Transition::Condition::Mode::NEQ;
                    else if (mode == "LT") cond.mode = Transition::Condition::Mode::LT;
                    else if (mode == "GT") cond.mode = Transition::Condition::Mode::GT;
                    else if (mode == "LTE") cond.mode = Transition::Condition::Mode::LTE;
                    else if (mode == "GTE") cond.mode = Transition::Condition::Mode::GTE;
                    
                    cond.value = condJson.value("value", 0.0f);
                    trans.conditions.push_back(cond);
                    Debug::Log("Condition value: " + std::to_string(cond.value));
                }
            }
            transitions.push_back(trans);
        }
    }

    initialState = j.value("initialState", "");
    Debug::Log("Initial state set to: '" + initialState + "'");

    Debug::Log("AnimatorLoader::fromJSON completed");
    return AnimatorParameters(states, transitions, initialState, parameters, spriteRenderer);
}

AnimatorComponent AnimatorLoader::createFromJSON(nlohmann::json& j, EntityManager& entityManager) {
    return AnimatorComponent(fromJSON(j, entityManager));
}