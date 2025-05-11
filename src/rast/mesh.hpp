#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "types.hpp"

namespace rast {
	class mesh {
	public:
		class cube {
		public:
			static const f32 vertex_array [108];
			static const u32 indices [36];
			static const f32 vertices [72];
			static const f32 uv[48];
		};
		static std::vector<glm::vec3> grid(u32 x, u32 y, f siz);
	};
}
