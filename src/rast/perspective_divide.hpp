#pragma once

namespace rast {
	template <typename Vec4>
	inline constexpr Vec4 perspective_divided(Vec4 vertex) {
		return {
			vertex[0] /= vertex[3],
			vertex[1] /= vertex[3],
			vertex[2] /= vertex[3],
			vertex[3]
		};
	}
	namespace perspective_divide {
		template <typename Vec4>
		inline constexpr void one(Vec4& Vector) {
			Vector[0] /= Vector[3];
			Vector[1] /= Vector[3];
			Vector[2] /= Vector[3];
		}
		template <typename Vec4Iter>
		inline constexpr void range(Vec4Iter begin, Vec4Iter end) {
			for (auto it = begin; it != end; ++it) one(*it);
		}
		template <typename RangeOfVec4s>
		inline constexpr void range(RangeOfVec4s& Range) {
			range(Range.begin(), Range.end());
		}
	}
}
