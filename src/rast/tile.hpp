#pragma once
#include <glm/glm.hpp>
#include "math/vec.hpp"

namespace rast {
	struct tile {
	public:
		glm::ivec2 min;
		glm::ivec2 max;
		inline constexpr tile(int minx, int miny, int maxx, int maxy) :
			min(minx << 4, miny << 4), max(maxx << 4, maxy << 4) {}
	};

	template <size_t Count>
	struct tile_x {
		math::i32vec2x<Count> min;
		math::i32vec2x<Count> max;
		inline constexpr tile_x(int MinX, int MinY, int MaxX, int MaxY) :
			min(math::i32x<Count>(MinX << 4), math::i32x<Count>(MinY)),
			max(math::i32x<Count>(MaxX << 4), math::i32x<Count>(MaxY)) { }
	};
}
