# Collect all .cpp files from game folder and subdirectories
file(GLOB_RECURSE GAME_SOURCES "*.cpp")

# Define the game library as STATIC
add_library(game STATIC ${GAME_SOURCES})

# Set include directories for game
target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link game to engine
target_link_libraries(game PUBLIC engine)