#pragma once
#include <algorithm>
#include <xmmintrin.h>

#include <glm/glm.hpp>

namespace rast::convert {
	template <typename float_type, typename int_type>
	inline constexpr glm::vec<4, int_type> f01_to_uint(const glm::vec<4, float_type>& in) {
		return glm::vec<4, int_type>(
			std::clamp(in.x, 0.0f, 1.0f) * std::numeric_limits<int_type>::max(),
			std::clamp(in.y, 0.0f, 1.0f) * std::numeric_limits<int_type>::max(),
			std::clamp(in.z, 0.0f, 1.0f) * std::numeric_limits<int_type>::max(),
			std::clamp(in.w, 0.0f, 1.0f) * std::numeric_limits<int_type>::max()
		);
	}

	template <typename int_type, typename float_type>
	inline constexpr glm::vec<4, float_type> uint_to_f01(glm::vec<4, int_type> in) {
		return glm::vec<4, float_type>(
			static_cast<float_type>(in.x) / std::numeric_limits<int_type>::max(),
			static_cast<float_type>(in.y) / std::numeric_limits<int_type>::max(),
			static_cast<float_type>(in.z) / std::numeric_limits<int_type>::max(),
			static_cast<float_type>(in.w) / std::numeric_limits<int_type>::max()
		);
	}

	inline __m128 uint_to_f01_sse(__m128i in) {
		__m128 res = _mm_cvtepi32_ps(in);
		__m128 div = _mm_set_ps1(static_cast<float>(std::numeric_limits<uint32_t>::max()));
		return _mm_div_ps(res, div);
	}
}
