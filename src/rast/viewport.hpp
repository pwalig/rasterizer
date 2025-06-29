#pragma once
#include <glm/glm.hpp>

namespace rast {
	class viewport {
	public:
		glm::ivec2 offset;
		glm::ivec2 extent;
		inline viewport(int xoffset, int yoffset, int width, int height) :
			offset(xoffset << 4, yoffset << 4), extent(width << 4, height << 4) {}
	};
}
