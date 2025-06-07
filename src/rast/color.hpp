#pragma once
#include <glm/glm.hpp>

namespace rast::color {
	using rgb8 = glm::vec<3, uint8_t>;
	using rgba8 = glm::vec<4, uint8_t>;

	using rgb32f = glm::vec<3, float>;
	using rgba32f = glm::vec<4, float>;
}
