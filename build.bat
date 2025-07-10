@echo off
cls

REM Check if the build folder exists to avoid unnecessary deletion
if not exist build mkdir build

cd build

REM Run CMake only if CMakeLists.txt or other configurations change
if not exist CMakeCache.txt (
    cmake .. -G "MinGW Makefiles" -DCMAKE_VERBOSE_MAKEFILE=TRUE
)

REM Use parallel build to speed up compilation
cmake --build . --config Debug -- -j %NUMBER_OF_PROCESSORS%

cd ..
