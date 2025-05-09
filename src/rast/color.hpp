#pragma once
#include "types.hpp"
#include <glm/glm.hpp>

namespace rast::color {
	template <typename T>
	class rgba {
	public:
		T r, g, b, a;
		inline rgba(T R, T G, T B, T A) : r(R), g(G), b(B), a(A) {}
	};

	template <typename T>
	class rgb {
	public:
		T r, g, b;
		inline rgb(T R, T G, T B) : r(R), g(G), b(B) {}
	};

	using rgb8 = rgb<u8>;
	using rgba8 = glm::vec<4, u8>;

	using rgb32f = rgb<f32>;
	using rgba32f = rgba<f32>;
}
