#pragma once
#include <glm/glm.hpp>

namespace rast::interpol {
	template <typename T>
	inline T interpolate(const T& a, const T& b, const T& c, const glm::vec3& coefs) {
		return (T)(a * coefs.x) + (T)(b * coefs.y) + (T)(c * coefs.z);
	}

	template <typename T>
	inline T interpolate(const glm::vec<3, T>& values, const glm::vec3& coefs) {
		return (T)(values.x * coefs.x) + (T)(values.y * coefs.y) + (T)(values.z * coefs.z);
	}

	template <typename T>
	inline T interpolate2(T a, T b, T c, const glm::vec3& coefs) {
		return (T)(a * coefs.x) + (T)(b * coefs.y) + (T)(c * coefs.z);
	}

	inline constexpr glm::vec3 average_coefs() {
		return glm::vec3(
			1.0f / 3.0f,
			1.0f / 3.0f,
			1.0f / 3.0f
		);
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
		inline constexpr T linear(T partial_coefs) {
			return normalized(partial_coefs);
		}
		template <typename T, typename VertexT>
		inline constexpr T perspective(T partial_coefs, VertexT* triangle) {
			return normalized(T(
				partial_coefs.x / triangle[0].rastPos.w,
				partial_coefs.y / triangle[1].rastPos.w,
				partial_coefs.z / triangle[2].rastPos.w
			));
		}
	}

	inline glm::vec3 linear_coefs(const glm::ivec3& equation_results, int triangle_area) {
		glm::vec3 res(
			(float)equation_results.y / triangle_area,
			(float)equation_results.z / triangle_area,
			(float)equation_results.x / triangle_area
		);
		float sum = res.x + res.y + res.z;
		return res / sum;
	}

	template <typename VertexT>
	inline glm::vec3 persp_coefs(const glm::ivec3& equation_results, int triangle_area, const VertexT* triangle) {
		glm::vec3 pcoefs(
			(float)equation_results.y / triangle_area / triangle[0].rastPos.w,
			(float)equation_results.z / triangle_area / triangle[1].rastPos.w,
			(float)equation_results.x / triangle_area / triangle[2].rastPos.w
		);
		float sum = pcoefs.x + pcoefs.y + pcoefs.z;
		return pcoefs / sum;
	}
}
