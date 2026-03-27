#pragma once
#include <cstdint>
#include "simd.hpp"

namespace rast {
	// base class for anything that needs 2d size
	// images / samplers / framebuffers
	// defines width, height, area and data_offset methods
	struct sized2d_base {
		using size_type = uint32_t;

	protected:
		size_type _width;
		size_type _height;

		inline constexpr size_type data_offset(size_type x, size_type y) const { return y * _width + x; }

		template <size_t Count>
		inline simd::u32x4 data_offset(simd::u32x_<Count> x, simd::u32x_<Count> y) const {
			return y * simd::u32x_<Count>(_width) + x;
		}

	public:
		inline constexpr sized2d_base(size_type Width, size_type Height) noexcept : _width(Width), _height(Height) {}

		inline constexpr size_type width() const noexcept { return _width; }
		inline constexpr size_type height() const noexcept { return _height; }
		inline constexpr size_type area() const { return _width * _height; }
	};
}
