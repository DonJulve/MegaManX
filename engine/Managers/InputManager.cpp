#include "InputManager.h"
#include <cstring> // for memcpy

// Define static variables
std::array<Uint8, SDL_NUM_SCANCODES> InputManager::keyStates = {0};
std::array<Uint8, SDL_NUM_SCANCODES> InputManager::prevKeyStates = {0};
std::unordered_map<Uint8, bool> InputManager::mouseButtonStates;
std::unordered_map<Uint8, bool> InputManager::prevMouseButtonStates;
int InputManager::mouseX = 0;
int InputManager::mouseY = 0;
int InputManager::scrollWheelX = 0;
int InputManager::scrollWheelY = 0;
SDL_Scancode InputManager::lastKeyPressed = SDL_SCANCODE_UNKNOWN;

void InputManager::resetLastKey()
{
    lastKeyPressed = SDL_SCANCODE_UNKNOWN;
}

SDL_Scancode InputManager::getLastKey()
{
    return lastKeyPressed;
}

// Call this at the start of each frame
void InputManager::updatePreviousState() {
    // Copy current key states to previous (fast with memcpy)
    memcpy(prevKeyStates.data(), keyStates.data(), SDL_NUM_SCANCODES * sizeof(Uint8));
    prevMouseButtonStates = mouseButtonStates; // Small map, assignment is fine

    scrollWheelX = 0;
    scrollWheelY = 0;

    resetLastKey();
}

// Optimized keyboard handling with memcpy
void InputManager::handleKeyboard() {
    const Uint8* state = SDL_GetKeyboardState(nullptr);
    memcpy(keyStates.data(), state, SDL_NUM_SCANCODES * sizeof(Uint8));
}

// Process mouse events only
void InputManager::handleEvents(SDL_Event& event) {
    switch (event.type) {
        case SDL_MOUSEBUTTONDOWN:
            mouseButtonStates[event.button.button] = true;
            break;
        case SDL_MOUSEBUTTONUP:
            mouseButtonStates[event.button.button] = false;
            break;
        case SDL_MOUSEMOTION:
            mouseX = event.motion.x;
            mouseY = event.motion.y;
            break;
        case SDL_MOUSEWHEEL:
            scrollWheelX = event.wheel.x;
            scrollWheelY = event.wheel.y;
            break;
        case SDL_KEYDOWN:
            lastKeyPressed = event.key.keysym.scancode;  // Store the scancode of the pressed key
            break;
    }
}

// Returns true only on the first frame the key is pressed
bool InputManager::isKeyPressed(SDL_Scancode key) {
    return keyStates[key] && !prevKeyStates[key];
}

// Returns true as long as the key is held down
bool InputManager::isKeyHeld(SDL_Scancode key) {
    return keyStates[key];
}

// Returns true only on the first frame the key is released
bool InputManager::isKeyReleased(SDL_Scancode key) {
    return !keyStates[key] && prevKeyStates[key];
}

// Mouse input functions
bool InputManager::isMouseButtonPressed(Uint8 button) {
    return mouseButtonStates[button] && !prevMouseButtonStates[button];
}

bool InputManager::isMouseButtonHeld(Uint8 button) {
    return mouseButtonStates[button];
}

bool InputManager::isMouseButtonReleased(Uint8 button) {
    return !mouseButtonStates[button] && prevMouseButtonStates[button];
}

int InputManager::getMouseX() {
    return mouseX;
}

int InputManager::getMouseY() {
    return mouseY;
}

int InputManager::getScrollWheelX() {
    return scrollWheelX;
}

int InputManager::getScrollWheelY() {
    return scrollWheelY;
}


// Clears input states (optional)
void InputManager::resetInputState() {
    keyStates.fill(0);
    prevKeyStates.fill(0);
    mouseButtonStates.clear();
    prevMouseButtonStates.clear();
}

SDL_FPoint InputManager::screenToWorld(float mouseX, float mouseY, Camera& camera) {
    SDL_FPoint worldPoint;
    
    // 1. Convert screen coordinates to normalized coordinates (-1 to 1 range)
    float normalizedX = (mouseX - camera.windowWidth / 2.0f) / (camera.windowWidth / 2.0f);
    float normalizedY = (mouseY - camera.windowHeight / 2.0f) / (camera.windowHeight / 2.0f);
    
    // 2. Apply inverse zoom and add camera position
    worldPoint.x = (normalizedX / camera.zoom) * (camera.windowWidth / 2.0f) + camera.getPosX();
    worldPoint.y = (normalizedY / camera.zoom) * (camera.windowHeight / 2.0f) + camera.getPosY();

    return worldPoint;
}

SDL_FPoint InputManager::worldToScreen(float worldX, float worldY, Camera& camera) {
    SDL_FPoint screenPoint;
    
    // 1. Subtract camera position to get relative coordinates
    float relativeX = worldX - camera.getPosX();
    float relativeY = worldY - camera.getPosY();
    
    // 2. Apply zoom and convert to screen coordinates
    screenPoint.x = (relativeX * camera.zoom) + (camera.windowWidth / 2.0f);
    screenPoint.y = (relativeY * camera.zoom) + (camera.windowHeight / 2.0f);

    return screenPoint;
}
