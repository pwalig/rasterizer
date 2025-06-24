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
* deffered rendering is possible and accelerated with specialized method for screen quad rendering
* rendering to screen or texture

# Building

## Clone the repository

```
git clone https://github.com/pwalig/rasterizer.git
cd rasterizer
```

## Dependencies

### SDL and Rapidjson

```
git submodule init
git submodule update --depth 1
```

### STB and GLM
Project depends on:
* [glm](https://github.com/g-truc/glm) - for math.
* [stb](https://github.com/nothings/stb) - for image loading.

Download both libraries and set envirionment variables `GLM_PATH` and `STB_PATH` to point to downloaded / cloned source code.

## Build

### CMake

Project can be built with [CMake](https://cmake.org).

Run the following.

```
mkdir build
cmake -S . -B build
cmake --build build 
```

to get release build with MSVC on Windows use:
```
cmake --build build --config Release
```

### Visual Studio 2022

Project can be built with [Visual Studio](https://visualstudio.microsoft.com/).

Just open rasterizer.sln and run.

Tested only on Visual Studio 2022 other versions might not work.