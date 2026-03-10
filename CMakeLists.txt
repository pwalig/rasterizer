cmake_minimum_required(VERSION 3.16)
project(rasterizer CXX)
set(CMAKE_CXX_STANDARD 17)

# Set the output directory for built objects. (Required by SDL)
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_subdirectory(ext/SDL EXCLUDE_FROM_ALL)

add_executable(rasterizer)

if (MSVC)
    target_compile_options(rasterizer PRIVATE /W4 /WX)
    target_compile_options(rasterizer PRIVATE /arch:AVX2) # Enable avx2 (default is SSE2 - or so i read)
else()
    target_compile_options(rasterizer PRIVATE -Wall -Wextra -pedantic -Werror)
    target_compile_options(rasterizer PRIVATE -march=native) # Enable simd
endif()

target_include_directories(rasterizer PRIVATE ext/glm)
target_include_directories(rasterizer PRIVATE ext/stb)
target_include_directories(rasterizer PRIVATE ext/rapidjson/include/)
target_include_directories(rasterizer PRIVATE include/)

target_compile_definitions(rasterizer PRIVATE GLM_ENABLE_EXPERIMENTAL)

target_sources(rasterizer PRIVATE
	src/main.cpp
	src/game/fly_cam.cpp
)

# Link to the actual SDL3 library.
target_link_libraries(rasterizer PRIVATE SDL3::SDL3)