# rasterizer

Parallel software rasterizer written in C++.

![image](./img/Suzannes-261f3ae.png)

# Performance

Above scene (~35k triangles) renders in 60fps at 1920 x 1080 resolution on AMD Ryzen 5 3600 CPU.

# Features

* rendering with or without depth buffer
* drawing with or without index buffer
* programmable shaders (at compile time with C++ templates)
* multithreaded (each thread renders one vertical slice of the image)
* ability to specify viewport dimensions and offset (to render to only part of the image)
* loading meshes and textures
* sutherland-hodgman and front triangle clipping
* configurable alpha blending
* configurable depth test (less, more, never, always, ...)
* configurable texture sampling (wrapping mode, minification and magnification filters)
* configurable face culling (clockwise, counter clockwise, both, none)
* deffered rendering is possible and accelerated with specialized method for screen quad rendering
* rendering to screen or texture

# Building

## Clone the repository

```
git clone https://github.com/pwalig/rasterizer.git
cd rasterizer
```

## Dependencies

### SDL GLM, STB and Rapidjson

```
git submodule init
git submodule update --depth 1
```

## Build

### CMake

Project can be built with [CMake](https://cmake.org).

Run the following.

```
cd examples/sdl_model_viewer/
mkdir build
cmake -S . -B build
cmake --build build 
```

to get release build with MSVC on Windows use:
```
cmake --build build --config Release
```

### Visual Studio 2026

Project can be built with [Visual Studio](https://visualstudio.microsoft.com/).

Just open `examples/sdl_model_viewer/sdl_model_viewer.slnx` and run.

Tested only on Visual Studio 2026 other versions might not work.
