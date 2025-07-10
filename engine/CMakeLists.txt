# Configuración específica para Windows
if (WIN32)
    # Rutas de Windows
    set(CMAKE_PREFIX_PATH "C:/dev/SDL2")
    link_directories("C:/dev/SDL2/x86_64-w64-mingw32/lib")
endif()

# Define la biblioteca del motor y los archivos fuente
add_library(engine STATIC
    Core/Engine.cpp
    Core/Systems.cpp
    Managers/Scenes/Scene.cpp
    Managers/Scenes/SceneManager.cpp
    Managers/Scenes/SceneLoader.cpp
    Managers/EntityManager.cpp
    Managers/RaycastManager.cpp
    Managers/AudioManager.cpp
    Managers/InputManager.cpp
    Managers/DynamicInputManager.cpp
    Managers/PrefabManager.cpp
    Managers/RenderTextManager.cpp
    Components/ParallaxLayer.cpp
    Components/Camera.cpp
    Components/Collider.cpp
    Components/Rigidbody.cpp
    Components/SpriteRenderer.cpp
    Components/Transform.cpp
    Components/AnimatorComponent.cpp
    Components/UIComponent.cpp
    Components/IA/ClassicIA.cpp
    Components/IA/ReinforcementIAComponent.cpp
    Utils/Debug.cpp
    Utils/BVH.cpp
)

# Busca SDL2, SDL2_image y SDL2_mixer
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)

# Configura FetchContent para nlohmann/json
include(FetchContent)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE  # Evita advertencias de CMake
)
FetchContent_MakeAvailable(json)

# Directorios de inclusión
target_include_directories(engine PUBLIC
    ${CMAKE_SOURCE_DIR}/include         # Root include/ folder
    ${CMAKE_CURRENT_SOURCE_DIR}/Core  # engine/Components/
    ${CMAKE_CURRENT_SOURCE_DIR}/Managers  # engine/Components/
    ${CMAKE_CURRENT_SOURCE_DIR}/Managers/Scenes  # engine/Components/
    ${CMAKE_CURRENT_SOURCE_DIR}/Components  # engine/Components/
    ${CMAKE_CURRENT_SOURCE_DIR}/Components/IA  # engine/Components/IA
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils  # engine/Components/
    ${json_SOURCE_DIR}/include          # nlohmann/json include path
)

# Enlaza las bibliotecas de SDL2 y nlohmann/json
target_link_libraries(engine PRIVATE
    SDL2::SDL2
    SDL2_image::SDL2_image
    SDL2_mixer::SDL2_mixer
    nlohmann_json::nlohmann_json
)

# Configuración específica para Windows
if (WIN32)
    # Enlaza las bibliotecas .dll.a en Windows
    target_link_libraries(engine PRIVATE
        "C:/dev/SDL2/x86_64-w64-mingw32/lib/libSDL2.dll.a"
        "C:/dev/SDL2/x86_64-w64-mingw32/lib/libSDL2_image.dll.a"
        "C:/dev/SDL2/x86_64-w64-mingw32/lib/libSDL2_mixer.dll.a"
    )

    # Configura el subsistema de Windows
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
endif()