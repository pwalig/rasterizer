cmake_minimum_required(VERSION 3.16)
project(sdl_model_viewer CXX)
set(CMAKE_CXX_STANDARD 20)

# Set the output directory for built objects. (Required by SDL)
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_subdirectory(../../ext/SDL sdl EXCLUDE_FROM_ALL)

add_executable(sdl_model_viewer)

if (MSVC)
    target_compile_options(sdl_model_viewer PRIVATE /W4 /WX)
    target_compile_options(sdl_model_viewer PRIVATE /arch:AVX2) # Enable avx2 (default is SSE2 - or so i read)
else()
    target_compile_options(sdl_model_viewer PRIVATE -Wall -Wextra -pedantic -Werror)
    target_compile_options(sdl_model_viewer PRIVATE -march=native) # Enable simd
endif()

target_include_directories(sdl_model_viewer PRIVATE
    ../../ext/rapidjson/include/
    ../../include/
)

target_sources(sdl_model_viewer PRIVATE
	main.cpp
    fly_cam.cpp
)

add_subdirectory(../../ rast)
target_link_libraries(sdl_model_viewer PRIVATE rast)

add_subdirectory(../../ext/assimp assimp)
target_link_libraries(sdl_model_viewer PRIVATE assimp)

# Link to the actual SDL3 library.
target_link_libraries(sdl_model_viewer PRIVATE rast assimp SDL3::SDL3)