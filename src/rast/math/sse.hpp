#pragma once
#include <xmmintrin.h>

namespace rast::math {
	struct f32sse {
	private:
		__m128 _data;
	public:
		inline f32sse() : _data() {}
		inline f32sse(__m128 Value) : _data(Value) {}
		inline explicit f32sse(float Value) : _data(_mm_set_ps1(Value)) {}
		inline explicit f32sse(float v0, float v1, float v2, float v3) : _data(_mm_set_ps(v0, v1, v2, v3)) {}

		inline f32sse& operator=(__m128 Value) { _data = Value; return *this; }

		inline operator __m128() const { return _data; }

		friend inline f32sse operator+(f32sse lhs, f32sse rhs) { return f32sse(_mm_add_ps(lhs, rhs)); }
		friend inline f32sse operator-(f32sse lhs, f32sse rhs) { return f32sse(_mm_sub_ps(lhs, rhs)); }
		friend inline f32sse operator*(f32sse lhs, f32sse rhs) { return f32sse(_mm_mul_ps(lhs, rhs)); }
		friend inline f32sse operator/(f32sse lhs, f32sse rhs) { return f32sse(_mm_div_ps(lhs, rhs)); }

		inline f32sse& operator+=(f32sse rhs) { _data = _mm_add_ps(_data, rhs._data); return *this; }
		inline f32sse& operator-=(f32sse rhs) { _data = _mm_sub_ps(_data, rhs._data); return *this; }
		inline f32sse& operator*=(f32sse rhs) { _data = _mm_mul_ps(_data, rhs._data); return *this; }
		inline f32sse& operator/=(f32sse rhs) { _data = _mm_div_ps(_data, rhs._data); return *this; }
	};
}
