#pragma once
#include <glm/glm.hpp>

#include "types.hpp"

namespace rast::color {
	using rgb8 = glm::vec<3, u8>;
	using rgba8 = glm::vec<4, u8>;

	using rgb32f = glm::vec<3, f32>;
	using rgba32f = glm::vec<4, f32>;
}
