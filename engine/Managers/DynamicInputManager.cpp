#include "DynamicInputManager.h"
#include <fstream>
#include "InputManager.h"
#include "Debug.h"

DynamicInputManager::InputBinding::InputBinding(SDL_Scancode k) 
    : key(k), mouseButton(0), isMouseInput(false) {}

DynamicInputManager::InputBinding::InputBinding(Uint8 btn) 
    : key(SDL_SCANCODE_UNKNOWN), mouseButton(btn), isMouseInput(true) {}

DynamicInputManager& DynamicInputManager::getInstance() {
    static DynamicInputManager instance;
    return instance;
}

DynamicInputManager::DynamicInputManager() {
    Debug::Log("DynamicInputManager instance created.");
}

void DynamicInputManager::bindAction(const std::string& actionName, SDL_Scancode key) {
    bindings[actionName] = InputBinding(key);
    Debug::Log("Bound action '" + actionName + "' to key '" + SDL_GetScancodeName(key) + "'.");
}

void DynamicInputManager::bindAction(const std::string& actionName, Uint8 mouseButton) {
    bindings[actionName] = InputBinding(mouseButton);
    Debug::Log("Bound action '" + actionName + "' to mouse button '" + mouseButtonToName(mouseButton) + "'.");
}

bool DynamicInputManager::isActionPressed(const std::string& actionName) {
    auto it = bindings.find(actionName);
    if (it == bindings.end()) {
        Debug::Log("Action '" + actionName + "' not found in bindings.");
        return false;
    }

    const InputBinding& binding = it->second;
    if (binding.isMouseInput) {
        bool pressed = InputManager::isMouseButtonPressed(binding.mouseButton);
        return pressed;
    } else {
        bool pressed = InputManager::isKeyPressed(binding.key);
        return pressed;
    }
}

bool DynamicInputManager::isActionHeld(const std::string& actionName) {
    auto it = bindings.find(actionName);
    if (it == bindings.end()) {
        Debug::Log("Action '" + actionName + "' not found in bindings.");
        return false;
    }

    const InputBinding& binding = it->second;
    if (binding.isMouseInput) {
        bool held = InputManager::isMouseButtonHeld(binding.mouseButton);
        return held;
    } else {
        bool held = InputManager::isKeyHeld(binding.key);
        return held;
    }
}

bool DynamicInputManager::isActionReleased(const std::string& actionName) {
    auto it = bindings.find(actionName);
    if (it == bindings.end()) {
        Debug::Log("Action '" + actionName + "' not found in bindings.");
        return false;
    }

    const InputBinding& binding = it->second;
    if (binding.isMouseInput) {
        bool released = InputManager::isMouseButtonReleased(binding.mouseButton);
        return released;
    } else {
        bool released = InputManager::isKeyReleased(binding.key);
        return released;
    }
}

void DynamicInputManager::unbindAction(const std::string& actionName) {
    if (bindings.erase(actionName) > 0) {
        Debug::Log("Unbound action '" + actionName + "'.");
    } else {
        Debug::Log("Failed to unbind action '" + actionName + "': not found.");
    }
}

void DynamicInputManager::clearBindings() {
    Debug::Log("Clearing all input bindings.");
    bindings.clear();
}

bool DynamicInputManager::saveToJson(const std::string& filename) {
    json j;
    for (const auto& [action, binding] : bindings) {
        json bindingJson;
        bindingJson["isMouseInput"] = binding.isMouseInput;
        if (binding.isMouseInput) {
            bindingJson["input"] = mouseButtonToName(binding.mouseButton);
        } else {
            bindingJson["input"] = SDL_GetScancodeName(binding.key);
        }
        j[action] = bindingJson;
    }

    std::string filepath = "resources/input_config/" + filename;
    std::ofstream file(filepath);
    if (!file.is_open()) {
        Debug::Log("Failed to open file '" + filepath + "' for saving input config.");
        return false;
    }
    file << j.dump(4);
    file.close();
    Debug::Log("Saved input config to '" + filepath + "'.");
    return true;
}

bool DynamicInputManager::loadFromJson(const std::string& filename) {
    std::string filepath = "resources/input_config/" + filename;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        Debug::Log("Failed to open file '" + filepath + "' for loading input config.");
        return false;
    }

    json j;
    try {
        file >> j;
    } catch (const json::parse_error& e) {
        Debug::Log("JSON parse error in '" + filepath + "': " + e.what());
        file.close();
        return false;
    }
    file.close();

    bindings.clear();
    Debug::Log("Cleared existing bindings before loading from '" + filepath + "'.");

    for (const auto& [actionName, bindingJson] : j.items()) {
        bool isMouseInput = bindingJson.value("isMouseInput", false);
        std::string inputName = bindingJson.value("input", "Unknown");

        if (isMouseInput) {
            Uint8 mouseButton = nameToMouseButton(inputName);
            if (mouseButton != 0) {
                bindings[actionName] = InputBinding(mouseButton);
                Debug::Log("Loaded binding: '" + actionName + "' -> mouse button '" + inputName + "'.");
            } else {
                Debug::Log("Invalid mouse button '" + inputName + "' for action '" + actionName + "'. Skipping.");
            }
        } else {
            SDL_Scancode key = SDL_GetScancodeFromName(inputName.c_str());
            if (key != SDL_SCANCODE_UNKNOWN) {
                bindings[actionName] = InputBinding(key);
                Debug::Log("Loaded binding: '" + actionName + "' -> key '" + inputName + "'.");
            } else {
                Debug::Log("Invalid key '" + inputName + "' for action '" + actionName + "'. Skipping.");
            }
        }
    }
    Debug::Log("Finished loading input config from '" + filepath + "'.");
    return true;
}

std::string DynamicInputManager::mouseButtonToName(Uint8 button) {
    switch (button) {
        case SDL_BUTTON_LEFT:   return "SDL_BUTTON_LEFT";
        case SDL_BUTTON_RIGHT:  return "SDL_BUTTON_RIGHT";
        case SDL_BUTTON_MIDDLE: return "SDL_BUTTON_MIDDLE";
        case SDL_BUTTON_X1:     return "SDL_BUTTON_X1";
        case SDL_BUTTON_X2:     return "SDL_BUTTON_X2";
        default:                return "Unknown";
    }
}

Uint8 DynamicInputManager::nameToMouseButton(const std::string& name) {
    if (name == "SDL_BUTTON_LEFT")   return SDL_BUTTON_LEFT;
    if (name == "SDL_BUTTON_RIGHT")  return SDL_BUTTON_RIGHT;
    if (name == "SDL_BUTTON_MIDDLE") return SDL_BUTTON_MIDDLE;
    if (name == "SDL_BUTTON_X1")     return SDL_BUTTON_X1;
    if (name == "SDL_BUTTON_X2")     return SDL_BUTTON_X2;
    return 0;
}

SDL_Scancode DynamicInputManager::getBind(const std::string& actionName){
    auto it = bindings.find(actionName);
    if (it == bindings.end()) {
        throw std::runtime_error("Action '" + actionName + "' not found in bindings.");
    }

    const InputBinding& binding = it->second;

    return binding.key;
}