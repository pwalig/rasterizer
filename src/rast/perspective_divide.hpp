#pragma once

namespace rast {
	template <typename Vec4>
	inline constexpr Vec4 perspective_divided(Vec4 vec4) {
		return Vec4(
			vec4[0] / vec4[3],
			vec4[1] / vec4[3],
			vec4[2] / vec4[3],
			vec4[3]
		);
	}
	namespace perspective_divide {
		template <typename Vec4>
		inline constexpr void divide(Vec4& vec4) {
			vec4[0] /= vec4[3];
			vec4[1] /= vec4[3];
			vec4[2] /= vec4[3];
		}
		template <typename Vec4Iter>
		inline constexpr void range(Vec4Iter begin, Vec4Iter end) {
			for (auto it = begin; it != end; ++it) divide(*it);
		}
		template <typename RangeOfVec4s>
		inline constexpr void range(RangeOfVec4s& Range) {
			range(Range.begin(), Range.end());
		}
	}
}
