#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "types.hpp"

namespace rast::mesh {
	namespace cube {
		extern const f32 vertex_array [108];
		extern const u32 indices [36];
		extern const f32 vertices [72];
		extern const f32 normals [72];
		extern const f32 uv[48];
	};
	std::vector<glm::vec3> grid(u32 x, u32 y, f siz);


}
