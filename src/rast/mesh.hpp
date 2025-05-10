#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "types.hpp"

namespace rast {
	class mesh {
	public:
		static const glm::vec3 cube [36];
		static std::vector<glm::vec3> grid(u32 x, u32 y, f siz);
	};
}
