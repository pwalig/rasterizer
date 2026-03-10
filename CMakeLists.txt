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

if (MSVC)
    target_compile_options(rasterizer PRIVATE /W4 /WX)
    target_compile_options(rasterizer PRIVATE /arch:SSE4.1) #SIMD
else()
    target_compile_options(rasterizer PRIVATE -Wall -Werror)
    # target_compile_options(rasterizer PRIVATE -Wall -Wextra -pedantic -Werror)
    target_compile_options(rasterizer PRIVATE -march=native) #SIMD
endif()

target_include_directories(rasterizer PRIVATE ext/glm)
target_include_directories(rasterizer PRIVATE ext/stb)
target_include_directories(rasterizer PRIVATE ext/rapidjson/include/)
target_compile_definitions(rasterizer PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_sources(rasterizer PRIVATE src/main.cpp)
target_sources(rasterizer PRIVATE src/thread_pool.cpp)
target_sources(rasterizer PRIVATE src/stb_implementation.cpp)
target_sources(rasterizer PRIVATE src/rast/mesh.cpp)

target_sources(rasterizer PRIVATE src/game/fly_cam.cpp)

# Link to the actual SDL3 library.
target_link_libraries(rasterizer PRIVATE SDL3::SDL3)