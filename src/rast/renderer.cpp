#include "renderer.hpp"
#include <algorithm>
#include <iostream>

glm::ivec3 rast::renderer::toScreenSpace(const glm::vec4& vertex) {
    glm::vec4 res = vertex / vertex.w;

    return glm::ivec3(
        (( res.x + 1.0f ) * (float)viewportDims.x * 0.5f + (float)viewportOffset.x) * 16.0f,
        (( -res.y + 1.0f ) * (float)viewportDims.y * 0.5f + (float)viewportOffset.y) * 16.0f,
        res.z * 1000.0f // (-1 : 1) ==> (0 : 100000)
    );
}

void rast::renderer::setViewport(int xoffset, int yoffset, int width, int height)
{
    viewportDims.x = width;
    viewportDims.y = height;
    viewportOffset.x = xoffset;
    viewportOffset.y = yoffset;
}
