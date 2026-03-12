#pragma once
#include "../simd.hpp"
#include <glm/glm.hpp>

// redefinitions of glm functions for rast::simd types
namespace rast::simd::glm {
	template<size_t Count> using vec2 = ::glm::vec<2, f32x_<Count>>;
	template<size_t Count> using vec3 = ::glm::vec<3, f32x_<Count>>;
	template<size_t Count> using vec4 = ::glm::vec<4, f32x_<Count>>;

	template<size_t Count> using ivec2 = ::glm::vec<2, i32x_<Count>>;
	template<size_t Count> using ivec3 = ::glm::vec<3, i32x_<Count>>;
	template<size_t Count> using ivec4 = ::glm::vec<4, i32x_<Count>>;

	template <size_t Count>
	inline f32x_<Count> dot(vec3<Count> a, vec3<Count> b) {
		return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
	}
}
