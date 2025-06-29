#pragma once
#include <glm/glm.hpp>

namespace rast {
	class tile {
	public:
		glm::ivec2 min;
		glm::ivec2 max;
		inline tile(int minx, int miny, int maxx, int maxy) :
			min(minx << 4, miny << 4), max(maxx << 4, maxy << 4) {}
	};
}
