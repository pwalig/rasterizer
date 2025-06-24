#pragma once
#include <algorithm>

#include <glm/glm.hpp>

namespace rast::convert {
	template <typename float_type, typename int_type>
	inline glm::vec<4, int_type> f01_to_uint(const glm::vec<4, float_type>& in) {
		return glm::vec<4, int_type>(
			std::clamp(in.x, 0.0f, 1.0f) * std::numeric_limits<int_type>::max(),
			std::clamp(in.y, 0.0f, 1.0f) * std::numeric_limits<int_type>::max(),
			std::clamp(in.z, 0.0f, 1.0f) * std::numeric_limits<int_type>::max(),
			std::clamp(in.w, 0.0f, 1.0f) * std::numeric_limits<int_type>::max()
		);
	}

	template <typename int_type, typename float_type>
	inline glm::vec<4, float_type> uint_to_f01(glm::vec<4, int_type> in) {
		return glm::vec<4, float_type>(
			static_cast<float_type>(in.x) / std::numeric_limits<int_type>::max(),
			static_cast<float_type>(in.y) / std::numeric_limits<int_type>::max(),
			static_cast<float_type>(in.z) / std::numeric_limits<int_type>::max(),
			static_cast<float_type>(in.w) / std::numeric_limits<int_type>::max()
		);
	}
}
