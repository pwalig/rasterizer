#pragma once
#include <glm/glm.hpp>
#include "math/vec.hpp"

namespace rast {
	class viewport {
	public:
		glm::ivec2 offset;
		glm::ivec2 extent;
		inline viewport(int xoffset, int yoffset, int width, int height) :
			offset(xoffset << 4, yoffset << 4), extent(width << 4, height << 4) {}
	};

	template <size_t Count>
	struct viewport_x {
		math::i32vec2x<Count> offset;
		math::i32vec2x<Count> extent;
		inline constexpr viewport_x(int OffsetX, int OffsetY, int ExtentX, int ExtentY) :
			offset(math::i32x<Count>(OffsetX << 4), math::i32x<Count>(OffsetY << 4)),
			extent(math::i32x<Count>(ExtentX << 4), math::i32x<Count>(ExtentY << 4)) { }
	};
}
