cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(MegaManX VERSION 1.0)

# Set C++ standard version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Specify include directories
include_directories(include)
include_directories(engine)
include_directories(game)

# Configuración específica para Windows
if (WIN32)
    # Set SDL2 path (adjust this to match your SDL2 and SDL_image installation)
    set(CMAKE_PREFIX_PATH "C:/dev/SDL2")
    link_directories("C:/dev/SDL2/x86_64-w64-mingw32/lib")
endif()

# Find SDL2 package
find_package(SDL2 REQUIRED)

# Find SDL2_image package
find_package(SDL2_image REQUIRED)

# Find SDL2_mixer package (si lo usas)
find_package(SDL2_mixer REQUIRED)

# Include directories for SDL2 and SDL2_image
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
include_directories(${SDL2_MIXER_INCLUDE_DIRS})  # Si usas SDL2_mixer

# Add subdirectories
add_subdirectory(engine)
add_subdirectory(game)

# Add your main executable
add_executable(MegaManX main.cpp)

# Link libraries
target_link_libraries(MegaManX PRIVATE
    engine
    game
    SDL2::SDL2main
    SDL2::SDL2
    SDL2_image::SDL2_image
    SDL2_mixer::SDL2_mixer  # Si usas SDL2_mixer
)

# Ensure the project is a Windows application (no console)
if (WIN32)
    if (MSVC)
        # For Microsoft Visual Studio
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
    elseif (MINGW)
        # For MinGW - Windows GUI app without console
        target_link_options(MegaManX PRIVATE -mwindows)

        # Static linking of C++ runtime
        target_link_options(MegaManX PRIVATE -static -static-libgcc -static-libstdc++)
    endif()
endif()

# Copy SDL2.dll and SDL2_image.dll to the build folder (solo en Windows)
if (WIN32)
    add_custom_command(TARGET MegaManX POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/dev/SDL2/x86_64-w64-mingw32/bin/SDL2.dll"
        $<TARGET_FILE_DIR:MegaManX>
        COMMENT "Copying SDL2.dll to build directory"
    )

    add_custom_command(TARGET MegaManX POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/dev/SDL2/x86_64-w64-mingw32/bin/SDL2_image.dll"
        $<TARGET_FILE_DIR:MegaManX>
        COMMENT "Copying SDL2_image.dll to build directory"
    )

    add_custom_command(TARGET MegaManX POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/dev/SDL2/x86_64-w64-mingw32/bin/SDL2_mixer.dll"
        $<TARGET_FILE_DIR:MegaManX>
        COMMENT "Copying SDL2_mixer.dll to build directory"
    )
endif()

# Ensure resources folder is copied to the build directory
add_custom_command(TARGET MegaManX POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resources"
    "$<TARGET_FILE_DIR:MegaManX>/resources"
    COMMENT "Copying resources folder to build directory"
)

# Create a 'logs' folder before running the executable
add_custom_command(
    TARGET MegaManX PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:MegaManX>/logs"
    COMMENT "Creating logs directory"
)