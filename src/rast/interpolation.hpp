#pragma once

namespace rast::interpol {
	template <size_t Size, typename T>
	inline constexpr T interpolate(T elems[Size], float coefs[Size]) {
		T res;
		for (size_t i = 0; i < Size; ++i) {
			res += elems[i] * coefs[i];
		}
		return res;
	}
	template <typename T, typename Vec3>
	inline constexpr T interpolate(T a, T b, T c, Vec3 coefs) {
		return (a * coefs[0]) + (b * coefs[1]) + (c * coefs[2]);
	}
	template <typename T, typename Vec3>
	inline constexpr T interpolate(T elems[3], Vec3 coefs) {
		return (elems[0] * coefs[0]) + (elems[1] * coefs[1]) + (elems[2] * coefs[2]);
	}

	namespace coefs {
		template <typename T>
		inline constexpr void normalize(T& coefs) {
			auto sum = coefs.x + coefs.y + coefs.z;
			coefs /= sum;
		}
		template <typename T>
		inline constexpr T normalized(T coefs) {
			auto sum = coefs.x + coefs.y + coefs.z;
			return coefs / sum;
		}
		template <typename T>
		inline constexpr T average() {
			return T(1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f);
		}
		template <typename T>
		inline constexpr T linear(T partial_coefs) {
			return normalized(partial_coefs);
		}
		template <typename T>
		inline constexpr T perspective(T partial_coefs, float w1, float w2, float w3) {
			return normalized(T(
				partial_coefs.x / w1,
				partial_coefs.y / w2,
				partial_coefs.z / w3
			));
		}
		template <typename T, typename VertexT>
		inline constexpr T perspective(T partial_coefs, VertexT* triangle) {
			return perspective(partial_coefs, triangle[0].rastPos.w, triangle[1].rastPos.w, triangle[2].rastPos.w);
		}
	}

	template <typename VertexT, typename Vec3>
	inline constexpr VertexT perspective(Vec3 partial_coefs, VertexT* triangle) {
		return interpolate(triangle[0].data, triangle[1].data, triangle[2].data, coefs::perspective(partial_coefs, triangle));
	}

}
