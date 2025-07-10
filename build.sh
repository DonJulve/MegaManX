clear

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Ejecutar CMake solo si CMakeCache.txt no existe
if [ ! -f "CMakeCache.txt" ]; then
    cmake .. -G "Unix Makefiles" -DCMAKE_VERBOSE_MAKEFILE=TRUE
fi

cmake --build . --config Debug -- -j$(nproc)

cd ..