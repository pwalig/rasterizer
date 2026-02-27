#pragma once
#include <immintrin.h>
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

	namespace sse {
		struct vec3 {
			using value_type = __m128;

			__m128 x;
			__m128 y;
			__m128 z;

			inline vec3() = default;
			inline vec3(__m128 X, __m128 Y, __m128 Z) : x(X), y(Y), z(Z) {}

			friend inline vec3 operator+(vec3 lhs, vec3 rhs) {
				return {
					_mm_add_ps(lhs.x, rhs.x),
					_mm_add_ps(lhs.y, rhs.y),
					_mm_add_ps(lhs.z, rhs.z)
				};
			}
			friend inline vec3 operator-(vec3 lhs, vec3 rhs) {
				return {
					_mm_sub_ps(lhs.x, rhs.x),
					_mm_sub_ps(lhs.y, rhs.y),
					_mm_sub_ps(lhs.z, rhs.z)
				};
			}
			friend inline vec3 operator*(vec3 lhs, __m128 rhs) {
				return {
					_mm_mul_ps(lhs.x, rhs),
					_mm_mul_ps(lhs.y, rhs),
					_mm_mul_ps(lhs.z, rhs)
				};
			}
			friend inline vec3 operator/(vec3 lhs, __m128 rhs) {
				return {
					_mm_div_ps(lhs.x, rhs),
					_mm_div_ps(lhs.y, rhs),
					_mm_div_ps(lhs.z, rhs)
				};
			}

			inline vec3& operator+=(vec3 rhs) {
				x = _mm_add_ps(x, rhs.x);
				y = _mm_add_ps(y, rhs.y);
				z = _mm_add_ps(z, rhs.z);
				return *this;
			}

			inline static __m128 length(__m128 x, __m128 y, __m128 z) {
				// sqrt((x * x) + (y * y) + (z * z))
				return _mm_sqrt_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(x, x), _mm_mul_ps(y, y)), _mm_mul_ps(z, z)));
			}
			inline __m128 length() const {
				return length(x, y, z);
			}
		};

		inline __m128 dot(vec3 a, vec3 b) {
			return _mm_add_ps(_mm_add_ps(_mm_mul_ps(a.x, b.x), _mm_mul_ps(a.y, b.y)), _mm_mul_ps(a.z, b.z));
		}
		inline bool or_accross(__m128i x) {
			return _mm_extract_epi32(_mm_or_si128(
				_mm_or_si128(
					_mm_shuffle_epi32(x, 0b11100100), // 3 2 1 0
					_mm_shuffle_epi32(x, 0b10010011)  // 2 1 0 3
				), _mm_or_si128(
					_mm_shuffle_epi32(x, 0b01001110), // 1 0 3 2
					_mm_shuffle_epi32(x, 0b00111001)  // 0 3 2 1
				)
			), 0) != 0;
		}

		struct ivec2 {
			using value_type = __m128;

			__m128i x;
			__m128i y;

			inline ivec2() = default;
			inline ivec2(__m128i X, __m128i Y) : x(X), y(Y) {}

			friend inline ivec2 operator+(ivec2 lhs, ivec2 rhs) {
				return {
					_mm_add_epi32(lhs.x, rhs.x),
					_mm_add_epi32(lhs.y, rhs.y)
				};
			}
			friend inline ivec2 operator-(ivec2 lhs, ivec2 rhs) {
				return {
					_mm_sub_epi32(lhs.x, rhs.x),
					_mm_sub_epi32(lhs.y, rhs.y)
				};
			}
			friend inline ivec2 operator*(ivec2 lhs, __m128i rhs) {
				return {
					_mm_mullo_epi32(lhs.x, rhs),
					_mm_mullo_epi32(lhs.y, rhs)
				};
			}

			inline ivec2& operator+=(ivec2 rhs) {
				x = _mm_add_epi32(x, rhs.x);
				y = _mm_add_epi32(y, rhs.y);
				return *this;
			}
			inline ivec2& operator-=(ivec2 rhs) {
				x = _mm_sub_epi32(x, rhs.x);
				y = _mm_sub_epi32(y, rhs.y);
				return *this;
			}
		};
		struct ivec3 {
			using value_type = __m128;

			__m128i x;
			__m128i y;
			__m128i z;

			inline ivec3() = default;
			inline ivec3(__m128i X, __m128i Y, __m128i Z) : x(X), y(Y), z(Z) {}

			friend inline ivec3 operator+(ivec3 lhs, ivec3 rhs) {
				return {
					_mm_add_epi32(lhs.x, rhs.x),
					_mm_add_epi32(lhs.y, rhs.y),
					_mm_add_epi32(lhs.z, rhs.z)
				};
			}
			friend inline ivec3 operator-(ivec3 lhs, ivec3 rhs) {
				return {
					_mm_sub_epi32(lhs.x, rhs.x),
					_mm_sub_epi32(lhs.y, rhs.y),
					_mm_sub_epi32(lhs.z, rhs.z)
				};
			}
			friend inline ivec3 operator*(ivec3 lhs, __m128i rhs) {
				return {
					_mm_mullo_epi32(lhs.x, rhs),
					_mm_mullo_epi32(lhs.y, rhs),
					_mm_mullo_epi32(lhs.z, rhs)
				};
			}

			inline ivec3& operator+=(ivec3 rhs) {
				x = _mm_add_epi32(x, rhs.x);
				y = _mm_add_epi32(y, rhs.y);
				z = _mm_add_epi32(z, rhs.z);
				return *this;
			}
			inline ivec3& operator-=(ivec3 rhs) {
				x = _mm_sub_epi32(x, rhs.x);
				y = _mm_sub_epi32(y, rhs.y);
				z = _mm_sub_epi32(z, rhs.z);
				return *this;
			}
		};
		struct ivec4 {
			using value_type = __m128;

			__m128i x;
			__m128i y;
			__m128i z;
			__m128i w;

			inline ivec4() = default;
			inline ivec4(__m128i X, __m128i Y, __m128i Z, __m128i W) : x(X), y(Y), z(Z), w(W) {}

			friend inline ivec4 operator+(ivec4 lhs, ivec4 rhs) {
				return {
					_mm_add_epi32(lhs.x, rhs.x),
					_mm_add_epi32(lhs.y, rhs.y),
					_mm_add_epi32(lhs.z, rhs.z),
					_mm_add_epi32(lhs.w, rhs.w)
				};
			}
			friend inline ivec4 operator-(ivec4 lhs, ivec4 rhs) {
				return {
					_mm_sub_epi32(lhs.x, rhs.x),
					_mm_sub_epi32(lhs.y, rhs.y),
					_mm_sub_epi32(lhs.z, rhs.z),
					_mm_sub_epi32(lhs.w, rhs.w)
				};
			}
			friend inline ivec4 operator*(ivec4 lhs, __m128i rhs) {
				return {
					_mm_mullo_epi32(lhs.x, rhs),
					_mm_mullo_epi32(lhs.y, rhs),
					_mm_mullo_epi32(lhs.z, rhs),
					_mm_mullo_epi32(lhs.w, rhs)
				};
			}

			inline ivec4& operator+=(ivec4 rhs) {
				x = _mm_add_epi32(x, rhs.x);
				y = _mm_add_epi32(y, rhs.y);
				z = _mm_add_epi32(z, rhs.z);
				w = _mm_add_epi32(w, rhs.w);
				return *this;
			}
			inline ivec4& operator-=(ivec4 rhs) {
				x = _mm_sub_epi32(x, rhs.x);
				y = _mm_sub_epi32(y, rhs.y);
				z = _mm_sub_epi32(z, rhs.z);
				w = _mm_sub_epi32(w, rhs.w);
				return *this;
			}
		};
	}
}
