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
			auto sum = coefs[0] + coefs[1] + coefs[2];
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
		template <typename Vec3>
		inline constexpr Vec3 perspective(Vec3 partial_coefs, Vec3 w) {
			return normalized(Vec3(
				partial_coefs[0] / w[0],
				partial_coefs[1] / w[1],
				partial_coefs[2] / w[2]
			));
		}
		template <typename Vec3>
		inline constexpr Vec3 perspective(
			Vec3 partial_coefs,
			typename Vec3::value_type w1,
			typename Vec3::value_type w2,
			typename Vec3::value_type w3
		) {
			return normalized(Vec3(
				partial_coefs[0] / w1,
				partial_coefs[1] / w2,
				partial_coefs[2] / w3
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
