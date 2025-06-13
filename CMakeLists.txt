cmake_minimum_required(VERSION 3.16)
project(rasterizer CXX)
set(CMAKE_CXX_STANDARD 17)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(ext/SDL EXCLUDE_FROM_ALL)

# Create your game executable target as usual
add_executable(rasterizer)
target_include_directories(rasterizer PRIVATE $ENV{GLM_PATH})
target_include_directories(rasterizer PRIVATE $ENV{STB_PATH})
target_compile_definitions(rasterizer PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_sources(rasterizer PRIVATE src/main.cpp)
target_sources(rasterizer PRIVATE src/thread_pool.cpp)
target_sources(rasterizer PRIVATE src/stb_implementation.cpp)
target_sources(rasterizer PRIVATE src/rast/mesh.cpp)

target_sources(rasterizer PRIVATE src/game/fly_cam.cpp)

# Link to the actual SDL3 library.
target_link_libraries(rasterizer PRIVATE SDL3::SDL3)