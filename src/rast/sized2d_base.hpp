#pragma once
#include <cstdint>
#include <emmintrin.h>
#include <immintrin.h>

namespace rast {
	// base class for anything that needs 2d size
	// images / samplers / framebuffers
	// defines width, height, area and data_offset methods
	struct sized2d_base {
		using size_type = uint32_t;

	private:
		size_type _width;
		size_type _height;

	public:
		inline constexpr sized2d_base(size_type Width, size_type Height) noexcept : _width(Width), _height(Height) {}

		inline constexpr size_type width() const noexcept { return _width; }
		inline constexpr size_type height() const noexcept { return _height; }
		inline constexpr size_type area() const { return _width * _height; }

		inline constexpr size_type data_offset(size_type x, size_type y) const { return y * _width + x; }
		inline __m128i data_offset(__m128i x, __m128i y) const {
			__m128i Width = _mm_set1_epi32(_width);
			return _mm_add_epi32(_mm_mullo_epi32(y, Width), x);
		}
#ifdef rast_avx
		inline __m256i data_offset(__m256i x, __m256i y) const {
			__m256i Width = _mm256_set1_epi32(_width);
			return _mm256_add_epi32(_mm256_mul_epu32(y, Width), x);
		}
#endif
	};
}
