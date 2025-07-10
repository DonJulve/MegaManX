#include "Engine.h"
#include "Systems.h"
#include "Debug.h"
#include "Scene.h"
#include "InputManager.h"
#include "Components.h"
#include "RenderTextManager.h"
#include <SDL2/SDL_mixer.h>

// Initialize the static instance pointer to nullptr
Engine* Engine::instance = nullptr;

// Private constructor to prevent direct instantiation
Engine::Engine(const char* title, int width, int height)
    : running(true), window(nullptr), renderer(nullptr) {
    init(title, width, height);  // Initialize with the title, width, and height
}

// Destructor to clean up resources
Engine::~Engine() {
    cleanUp();
}

// Initialize SDL, window, and renderer
void Engine::init(const char* title, int width, int height) {
    // Initialize SDL video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return;  // Stop execution if SDL initialization fails
    }

    // Initialize SDL_image (with PNG support)
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        SDL_Quit();  // Cleanup SDL
        return;  // Stop execution if SDL_image initialization fails
    }

    // Create SDL window
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if (!window) {
        IMG_Quit();  // Cleanup SDL_image
        SDL_Quit();  // Cleanup SDL
        return;
    }

    // Create SDL renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(window);  // Cleanup window
        IMG_Quit();  // Cleanup SDL_image
        SDL_Quit();  // Cleanup SDL
        return;
    }

    SpriteRendererLoader::initialize(getRenderer());
    RenderTextManager::getInstance().initialize(getRenderer());

    // Initialize SDL audio subsystem
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        SDL_DestroyWindow(window);  // Cleanup window
        IMG_Quit();  // Cleanup SDL_image
        SDL_Quit();  // Cleanup SDL
        return;
    }
    
    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, AUDIO_S16, 2, 256) < 0) {
        SDL_DestroyWindow(window);  // Cleanup window
        IMG_Quit();  // Cleanup SDL_image
        SDL_Quit();  // Cleanup SDL
        return;
    }

    Mix_AllocateChannels(16);

    SDL_Log("Engine initialized successfully.");
}

void Engine::handleEvents() {
    int time1 = SDL_GetTicks();
    SDL_Event event;
    InputManager::updatePreviousState(); 

    while (SDL_PollEvent(&event)) {
        InputManager::handleEvents(event);

        switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    // Get new window dimensions
                    int newWindowWidth = event.window.data1;
                    int newWindowHeight = event.window.data2;

                    // Update the camera scaling based on the new window size
                    Camera* camera = nullptr;
                    for (const auto& [name, s] : *(sceneManager.getScenes())) {
                        for (Entity entity : s->getEntityManager().getEntities()) {
                            camera = s->getEntityManager().getComponent<Camera>(entity);
                            if (camera) break;
                        }
                    }

                    if (camera) {
                        camera->updateScaling(newWindowWidth, newWindowHeight);
                    }
                }
                break;
        }
    }

    InputManager::handleKeyboard();  // Process real-time keyboard state

    int time2 = SDL_GetTicks();

    delayInputs = time2 - time1;  // Calculate delay for input processing
}

void Engine::start() {
    // Update each active entity in the entityManager
    for (const auto& [name, s] : *(sceneManager.getScenes())) {

        std::vector<Collider*> colliders;

        for (auto& entity : s->getEntityManager().getEntities()) {
            // Retrieve the component map for the current entity
            auto& componentMap = s->getEntityManager().getEntityComponents(entity);

            // Iterate over all components of the entity
            for (auto& [typeIndex, componentPtr] : componentMap) {
                if (componentPtr) {
                    componentPtr->start();  // Call the start method of the component
                }
            }

            Collider* collider = s->getEntityManager().getComponent<Collider>(entity);
            if (collider) {
                if(collider->getTag()=="Terrain") colliders.push_back(collider);
            }
        }

        s->setupStaticColliders(colliders);
    }
}

void Engine::update(float deltaTime) {
    sceneManager.update(deltaTime);
}

void Engine::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Clear the screen with black
    SDL_RenderClear(renderer);

    sceneManager.render();

    SDL_RenderPresent(renderer);  // Present the rendered frame to the window
}

void Engine::cleanUp() {
    SDL_DestroyRenderer(renderer);  // Destroy the renderer
    SDL_DestroyWindow(window);  // Destroy the window
    SDL_Quit();  // Quit SDL
}

void Engine::run() {
    int lastTime = SDL_GetTicks();  // Time of the previous frame
    int frameCount = 0;             // Count how many frames have passed
    int fpsTimer = SDL_GetTicks();  // Timer to track one-second intervals
    delayInputs = 0;              // Delay for input processing

    start();

    Debug::Log("Engine is running.");

    while (running) {
        int currentTime = SDL_GetTicks();
        float deltaTime = ((currentTime - lastTime) - delayInputs) / 1000.0f;
        lastTime = currentTime;

        handleEvents();
        update(deltaTime);
        render();
        InputManager::resetInputState();  // Reset input state at the end of each frame
        
        frameCount++;  // Increase frame count

        // Every second, calculate and print FPS
        if (currentTime - fpsTimer >= 1000) {
            int fps = frameCount;
            frameCount = 0;
            fpsTimer = currentTime;

            Debug::Log("FPS: " + std::to_string(fps));  // Print FPS to console (or log it)
        }
    }

    cleanUp();
}

// Public static method to access the singleton instance
Engine* Engine::getInstance(const char* title, int width, int height) {
    if (!instance) {
        instance = new Engine(title, width, height);
    }
    return instance;
}
