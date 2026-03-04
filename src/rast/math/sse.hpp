#pragma once
#include <immintrin.h>
#include <xmmintrin.h>

namespace rast::math {
	namespace simd {
		template <typename T, size_t Count> struct _register {};

		template <> struct _register<int8_t, 16> { using type = __m128i; };
		template <> struct _register<uint8_t, 16> { using type = __m128i; };
		template <> struct _register<int16_t, 8> { using type = __m128i; };
		template <> struct _register<uint16_t, 8> { using type = __m128i; };
		template <> struct _register<int32_t, 4> { using type = __m128i; };
		template <> struct _register<uint32_t, 4> { using type = __m128i; };
		template <> struct _register<int64_t, 2> { using type = __m128i; };
		template <> struct _register<uint64_t, 2> { using type = __m128i; };
		template <> struct _register<float, 4> { using type = __m128; };
		template <> struct _register<double, 2> { using type = __m128d; };

		template <typename T, size_t Count>
		using register_t = typename _register<T, Count>::type;

		struct mm_epi32 {
			using value_type = int;
			using register_type = __m128i;
			inline static __m128i set(int a) { return _mm_set1_epi32(a); }
			inline static __m128i set(int a, int b, int c, int d) { return _mm_set_epi32(a, b, c, d); }
			inline static int extract(__m128i a, int i) {
				switch (i) {
				case 0: return _mm_extract_epi32(a, 0);
				case 1: return _mm_extract_epi32(a, 1);
				case 2: return _mm_extract_epi32(a, 2);
				case 3: return _mm_extract_epi32(a, 3);
				default: assert(0); return 0;
				}
			}
			inline static __m128i add(__m128i a, __m128i b) { return _mm_add_epi32(a, b); }
			inline static __m128i sub(__m128i a, __m128i b) { return _mm_sub_epi32(a, b); }
			inline static __m128i mul(__m128i a, __m128i b) { return _mm_mullo_epi32(a, b); }
			inline static __m128i sll(__m128i a, __m128i b) { return _mm_sll_epi32(a, b); }
			inline static __m128i sra(__m128i a, __m128i b) { return _mm_sra_epi32(a, b); }
			inline static __m128i slli(__m128i a, int b) { return _mm_slli_epi32(a, b); }
			inline static __m128i srai(__m128i a, int b) { return _mm_srai_epi32(a, b); }
			inline static __m128i bitwise_and(__m128i a, __m128i b) { return _mm_and_si128(a, b); }
			inline static __m128i bitwise_or(__m128i a, __m128i b) { return _mm_or_si128(a, b); }
			inline static __m128i min(__m128i a, __m128i b) { return _mm_min_epi32(a, b); }
			inline static __m128i max(__m128i a, __m128i b) { return _mm_max_epi32(a, b); }
			inline static __m128i load(int* src) { return _mm_load_si128(reinterpret_cast<__m128i*>(src)); }
			inline static __m128i load(uint32_t* src) { return _mm_load_si128(reinterpret_cast<__m128i*>(src)); }
			inline static void store(int* dst, __m128i a) { return _mm_store_si128(reinterpret_cast<__m128i*>(dst), a); }
			inline static void store(uint32_t* dst, __m128i a) { return _mm_store_si128(reinterpret_cast<__m128i*>(dst), a); }

			template <typename T>
			inline static register_t<T, 4> cvt(__m128i a);
		};

		template <>
		inline register_t<float, 4> mm_epi32::cvt<float>(__m128i a) { return _mm_cvtepi32_ps(a); }

		struct mm_ps {
			using value_type = float;
			using register_type = __m128;
			inline static __m128 set(float a) { return _mm_set_ps1(a); }
			inline static __m128 set(float a, float b, float c, float d) { return _mm_set_ps(a, b, c, d); }
			inline static float extract(__m128 a, int i) {
				switch (i) {
				case 0: return _mm_cvtss_f32(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 0)));
				case 1: return _mm_cvtss_f32(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 1)));
				case 2: return _mm_cvtss_f32(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 2)));
				case 3: return _mm_cvtss_f32(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 3)));
				default: assert(0); return 0;
				}
			}
			inline static __m128 add(__m128 a, __m128 b) { return _mm_add_ps(a, b); }
			inline static __m128 sub(__m128 a, __m128 b) { return _mm_sub_ps(a, b); }
			inline static __m128 mul(__m128 a, __m128 b) { return _mm_mul_ps(a, b); }
			inline static __m128 div(__m128 a, __m128 b) { return _mm_div_ps(a, b); }
			inline static __m128 fmadd(__m128 a, __m128 b, __m128 c) { return _mm_fmadd_ps(a, b, c); }
			inline static __m128 bitwise_and(__m128 a, __m128 b) { return _mm_and_ps(a, b); }
			inline static __m128 bitwise_or(__m128 a, __m128 b) { return _mm_or_ps(a, b); }
			inline static __m128 ceil(__m128 a) { return _mm_ceil_ps(a); }
			inline static __m128 floor(__m128 a) { return _mm_floor_ps(a); }
			inline static __m128 round(__m128 a) { return _mm_round_ps(a, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC); }
			inline static __m128 min(__m128 a, __m128 b) { return _mm_min_ps(a, b); }
			inline static __m128 max(__m128 a, __m128 b) { return _mm_max_ps(a, b); }
			inline static __m128 load(float* src) { return _mm_load_ps(src); }
			inline static void store(float* dst, __m128 a) { return _mm_store_ps(dst, a); }
		};

		template <typename T, size_t Count>
		struct implementation { };

		template <> struct implementation<uint32_t, 4> {
			using type = mm_epi32;
		};
		template <> struct implementation<int, 4> {
			using type = mm_epi32;
		};
		template <> struct implementation<float, 4> {
			using type = mm_ps;
		};

		template <typename T, size_t Count>
		using implementation_t = typename implementation<T, Count>::type;

		template <typename T, size_t Count>
		struct _x_ {
			using value_type = T;
			using impl = implementation_t<T, Count>;
			using register_type = register_t<T, Count>;

			register_type m_value;

			inline _x_() : m_value() {}
			inline _x_(register_type Value) : m_value(Value) {}
			inline explicit _x_(T Value) : m_value(impl::set(Value)) {}
			template <typename U>
			inline explicit _x_(_x_<U, Count> Value) : m_value(cast<T>(Value)) {}

			inline _x_& operator=(register_type Value) { m_value = Value; return *this; }
			inline operator register_type() const { return m_value; }

			inline value_type operator[](int i) { return impl::extract(m_value, i); }

			friend inline _x_ operator+(_x_ lhs, _x_ rhs) { return impl::add(lhs, rhs); }
			friend inline _x_ operator-(_x_ lhs, _x_ rhs) { return impl::sub(lhs, rhs); }
			friend inline _x_ operator*(_x_ lhs, _x_ rhs) { return impl::mul(lhs, rhs); }
			friend inline _x_ operator/(_x_ lhs, _x_ rhs) { return impl::div(lhs, rhs); }
			friend inline _x_ operator<<(_x_ lhs, _x_ rhs) { return impl::sll(lhs, rhs); }
			friend inline _x_ operator>>(_x_ lhs, _x_ rhs) { return impl::sra(lhs, rhs); }
			friend inline _x_ operator&(_x_ lhs, _x_ rhs) { return impl::bitwise_and(lhs, rhs); }
			friend inline _x_ operator|(_x_ lhs, _x_ rhs) { return impl::bitwise_or(lhs, rhs); }

			friend inline _x_ operator+(_x_ lhs, register_type rhs) { return impl::add(lhs, rhs); }
			friend inline _x_ operator-(_x_ lhs, register_type rhs) { return impl::sub(lhs, rhs); }
			friend inline _x_ operator*(_x_ lhs, register_type rhs) { return impl::mul(lhs, rhs); }
			friend inline _x_ operator/(_x_ lhs, register_type rhs) { return impl::div(lhs, rhs); }
			friend inline _x_ operator<<(_x_ lhs, register_type rhs) { return impl::sll(lhs, rhs); }
			friend inline _x_ operator>>(_x_ lhs, register_type rhs) { return impl::sra(lhs, rhs); }
			friend inline _x_ operator&(_x_ lhs, register_type rhs) { return impl::bitwise_and(lhs, rhs); }
			friend inline _x_ operator|(_x_ lhs, register_type rhs) { return impl::bitwise_or(lhs, rhs); }

			friend inline _x_ operator<<(_x_ lhs, int rhs) { return impl::slli(lhs, rhs); }
			friend inline _x_ operator>>(_x_ lhs, int rhs) { return impl::srai(lhs, rhs); }


			inline _x_& operator+=(_x_ rhs) { m_value = impl::add(m_value, rhs); return *this; }
			inline _x_& operator-=(_x_ rhs) { m_value = impl::sub(m_value, rhs); return *this; }
			inline _x_& operator*=(_x_ rhs) { m_value = impl::mul(m_value, rhs); return *this; }
			inline _x_& operator/=(_x_ rhs) { m_value = impl::div(m_value, rhs); return *this; }
			inline _x_& operator<<=(_x_ rhs) { m_value = impl::sll(m_value, rhs); return *this; }
			inline _x_& operator>>=(_x_ rhs) { m_value = impl::sra(m_value, rhs); return *this; }
			inline _x_& operator&=(_x_ rhs) { m_value = impl::bitwise_and(m_value, rhs); return *this; }
			inline _x_& operator|=(_x_ rhs) { m_value = impl::bitwise_or(m_value, rhs); return *this; }

			inline _x_& operator+=(register_type rhs) { m_value = impl::add(m_value, rhs); return *this; }
			inline _x_& operator-=(register_type rhs) { m_value = impl::sub(m_value, rhs); return *this; }
			inline _x_& operator*=(register_type rhs) { m_value = impl::mul(m_value, rhs); return *this; }
			inline _x_& operator/=(register_type rhs) { m_value = impl::div(m_value, rhs); return *this; }
			inline _x_& operator<<=(register_type rhs) { m_value = impl::sll(m_value, rhs); return *this; }
			inline _x_& operator>>=(register_type rhs) { m_value = impl::sra(m_value, rhs); return *this; }
			inline _x_& operator&=(register_type rhs) { m_value = impl::bitwise_and(m_value, rhs); return *this; }
			inline _x_& operator|=(register_type rhs) { m_value = impl::bitwise_or(m_value, rhs); return *this; }

			inline _x_& operator<<=(int rhs) { m_value = impl::slli(m_value, rhs); return *this; }
			inline _x_& operator>>=(int rhs) { m_value = impl::srai(m_value, rhs); return *this; }
		};

		template <typename T, typename U, size_t Count>
		inline register_t<T, Count> cast(register_t<U, Count> a) {
			return implementation_t<U, Count>::template cvt<T>(a);
		}
		template <typename T, typename U, size_t Count>
		inline _x_<T, Count> cast(_x_<U, Count> a) {
			return implementation_t<U, Count>::template cvt<T>(a);
		}
		template <typename U, size_t Count>
		inline void store(U* dst, register_t<U, Count> a) {
			return implementation_t<U, Count>::store(dst, a);
		}
		template <typename U, size_t Count>
		inline void store(U* dst, _x_<U, Count> a) {
			return implementation_t<U, Count>::store(dst, a);
		}

		template <typename T> using _x2 = _x_<T, 2>;
		template <typename T> using _x4 = _x_<T, 4>;
		template <typename T> using _x8 = _x_<T, 8>;
		template <typename T> using _x16 = _x_<T, 16>;
		template <typename T> using _x32 = _x_<T, 32>;

		template <size_t Count> using i32x_ = _x_<int32_t, Count>;
		template <size_t Count> using u32x_ = _x_<uint32_t, Count>;
		template <size_t Count> using f32x_ = _x_<float, Count>;

		using i32x4 = _x_<int32_t, 4>;
		using u32x4 = _x_<uint32_t, 4>;
		using f32x4 = _x_<float, 4>;

		template <typename T>
		inline _x2<T> make_x2(T a, T b) {
			return implementation_t<T, 2>::set(a, b);
		}

		template <typename T>
		inline _x4<T> make_x4(T a, T b, T c, T d) {
			return implementation_t<T, 4>::set(a, b, c, d);
		}

		template <typename T>
		inline _x8<T> make_x8(T a, T b, T c, T d, T e, T f, T g, T h) {
			return implementation_t<T, 8>::set(a, b, c, d, e, f, g, h);
		}
	}

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
