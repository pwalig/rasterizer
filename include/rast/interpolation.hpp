#pragma once

namespace rast::interpol {
	template <typename T, typename Vec3>
	inline constexpr T interpolate(T a, T b, T c, Vec3 coefs) {
		return (a * coefs[0]) + (b * coefs[1]) + (c * coefs[2]);
	}

	namespace coefs {
		template <typename Vec3>
		inline constexpr void normalize(Vec3 coefs) {
			auto sum = coefs[0] + coefs[1] + coefs[2];
			coefs /= sum;
		}
		template <typename Vec3>
		inline constexpr Vec3 normalized(Vec3 coefs) {
			auto sum = coefs[0] + coefs[1] + coefs[2];
			return coefs / sum;
		}
		template <typename Vec3>
		inline constexpr Vec3 average() {
			using value_type = typename Vec3::value_type;
			return Vec3(
				value_type(1.0 / 3.0),
				value_type(1.0 / 3.0),
				value_type(1.0 / 3.0)
			);
		}
		template <typename T>
		inline constexpr T linear(T partial_coefs) {
			return normalized(partial_coefs);
		}
		template <typename Vec3>
		inline constexpr Vec3 perspective(Vec3&& partial_coefs, Vec3 w) {
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
		template <typename Vec3, typename VertexT>
		inline constexpr Vec3 perspective(Vec3 partial_coefs, const VertexT* triangle) {
			return perspective(partial_coefs,
				triangle[0].rastPos.w, triangle[1].rastPos.w, triangle[2].rastPos.w
			);
		}
	}

	template <typename VertexT, typename Vec3>
	inline constexpr auto perspective(const VertexT* triangle, Vec3 partial_coefs) {
		return interpolate(
			triangle[0].data, triangle[1].data, triangle[2].data,
			coefs::perspective(partial_coefs, triangle)
		);
	}

	template <typename VertexT, typename Vec3>
	inline constexpr auto depth(const VertexT* triangle, Vec3 partial_coefs) {
		return interpolate(
			triangle[0].rastPos.z, triangle[1].rastPos.z, triangle[2].rastPos.z,
			coefs::linear(partial_coefs)
		);
	}
}
