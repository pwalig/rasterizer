#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "types.hpp"

namespace rast::mesh{
	std::vector<glm::vec3> grid(u32 x, u32 y, f siz);
}
