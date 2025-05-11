#include "renderer.hpp"
#include <algorithm>
#include <iostream>

void rast::renderer::setViewport(int xoffset, int yoffset, int width, int height)
{
    viewportDims.x = width;
    viewportDims.y = height;
    viewportOffset.x = xoffset;
    viewportOffset.y = yoffset;
}
