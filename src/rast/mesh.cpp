#include "mesh.hpp"

const float rast::mesh::cube::vertex_array[108] = {
    // top
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,

    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,

    // bottom
    1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,

    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,

    // front
    1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,

    // back
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,

    // left
    -1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,

    -1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,

    // right
    1.0f, 1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, 1.0f,

    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, -1.0f, 1.0f,
};

const float rast::mesh::cube::vertices[72] = {
    // top
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,

    // bottom
    1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,

    // front
    1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,

    // back
    1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,

    // left
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,

    // right
    1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, -1.0f, 1.0f,
};

const rast::u32 rast::mesh::cube::indices[36] = {
    0, 1, 2,
    1, 2, 3,

    4, 5, 6,
    5, 6, 7,

    8, 9, 10,
    9, 10, 11,

    12, 13, 14,
    13, 14, 15,

    16, 17, 18,
    17, 18, 19,

    20, 21, 22,
    21, 22, 23
};

std::vector<glm::vec3> rast::mesh::grid(u32 x, u32 y, f siz)
{
    std::vector<glm::vec3> res;
    res.reserve(3 * 2 * x * y);

    for (f i = 0; i < siz * y; i += siz) {
        for (f j = 0; j < siz * x; j += siz) {
            res.push_back(glm::vec3(j, i, 0.0f));
            res.push_back(glm::vec3(j + siz, i, 0.0f));
            res.push_back(glm::vec3(j, i + siz, 0.0f));

            res.push_back(glm::vec3(j, i + siz, 0.0f));
            res.push_back(glm::vec3(j + siz, i, 0.0f));
            res.push_back(glm::vec3(j + siz, i + siz, 0.0f));
        }
    }

    return res;
}
