# rasterizer

Parallel software rasterizer written in C++.

![image](./img/Suzannes-261f3ae.png)

## Performance

Above scene (~35k triangles) renders in 60fps at 1920 x 1080 resolution on AMD Ryzen 5 3600 CPU.

## Features

* rendering with or without depth buffer
* drawing with or without index buffer
* programmable shaders (at compile time with C++ templates)
* multithreaded (each thread renders one vertical slice of the image)
* ability to specify viewport dimensions and offset (to render to only part of the image)
* loading meshes and textures
* triangle clipping (against z-near only)
* deffered rendering is possible and accelerated with specialized method for screen quad rendering
* rendering to screen or texture
