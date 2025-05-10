#include "mesh.hpp"

const glm::vec3 rast::mesh::cube[36] = {
    // top
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(-1.0f, 1.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, -1.0f),

    glm::vec3(1.0f, 1.0f, -1.0f),
    glm::vec3(-1.0f, 1.0f, 1.0f),
    glm::vec3(-1.0f, 1.0f, -1.0f),

    // bottom
    glm::vec3(1.0f, -1.0f, 1.0f),
    glm::vec3(1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f, -1.0f, 1.0f),

    glm::vec3(1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f, -1.0f, 1.0f),

    // front
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(1.0f, -1.0f, 1.0f),
    glm::vec3(-1.0f, 1.0f, 1.0f),

    glm::vec3(-1.0f, -1.0f, 1.0f),
    glm::vec3(-1.0f, 1.0f, 1.0f),
    glm::vec3(1.0f, -1.0f, 1.0f),

    // back
    glm::vec3(1.0f, 1.0f, -1.0f),
    glm::vec3(-1.0f, 1.0f, -1.0f),
    glm::vec3(1.0f, -1.0f, -1.0f),

    glm::vec3(-1.0f, -1.0f, -1.0f),
    glm::vec3(1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f, 1.0f, -1.0f),

    // left
    glm::vec3(-1.0f, 1.0f, -1.0f),
    glm::vec3(-1.0f, -1.0f, 1.0f),
    glm::vec3(-1.0f, -1.0f, -1.0f),

    glm::vec3(-1.0f, 1.0f, 1.0f),
    glm::vec3(-1.0f, -1.0f, 1.0f),
    glm::vec3(-1.0f, 1.0f, -1.0f),

    // right
    glm::vec3(1.0f, 1.0f, -1.0f),
    glm::vec3(1.0f, -1.0f, -1.0f),
    glm::vec3(1.0f, -1.0f, 1.0f),

    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, -1.0f),
    glm::vec3(1.0f, -1.0f, 1.0f),
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
