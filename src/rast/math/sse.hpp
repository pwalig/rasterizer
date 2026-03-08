#pragma once
#include <immintrin.h>
#include <xmmintrin.h>

namespace rast::math {
	namespace simd {
		enum struct cpu_flag {
			MMX = 1,
			SSE = 2,
			SSE2 = 4,
			SSE3 = 8,
			SSSE3 = 16,
			SSE4_1 = 32,
			SSE4_2 = 64,
			AVX = 128,
			AVX2 = 256,
			AVX512 = 512,
			FMA = 1024
		};
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

		template <typename T, size_t Count> inline register_t<T, Count> load(const T*);
		template <> inline __m128i load<int8_t, 16>(const int8_t* a) { return _mm_load_si128(reinterpret_cast<const __m128i*>(a)); }
		template <> inline __m128i load<uint8_t, 16>(const uint8_t* a) { return _mm_load_si128(reinterpret_cast<const __m128i*>(a)); }
		template <> inline __m128i load<int16_t, 8>(const int16_t* a) { return _mm_load_si128(reinterpret_cast<const __m128i*>(a)); }
		template <> inline __m128i load<uint16_t, 8>(const uint16_t* a) { return _mm_load_si128(reinterpret_cast<const __m128i*>(a)); }
		template <> inline __m128i load<int32_t, 4>(const int32_t* a) { return _mm_load_si128(reinterpret_cast<const __m128i*>(a)); }
		template <> inline __m128i load<uint32_t, 4>(const uint32_t* a) { return _mm_load_si128(reinterpret_cast<const __m128i*>(a)); }
		template <> inline __m128i load<int64_t, 2>(const int64_t* a) { return _mm_load_si128(reinterpret_cast<const __m128i*>(a)); }
		template <> inline __m128i load<uint64_t, 2>(const uint64_t* a) { return _mm_load_si128(reinterpret_cast<const __m128i*>(a)); }
		template <> inline __m128 load<float, 4>(const float* a) { return _mm_load_ps(a); }
		template <> inline __m128d load<double, 2>(const double* a) { return _mm_load_pd(a); }

		template <typename T, size_t Count> inline void store(T*, register_t<T, Count>);
		template <> inline void store<int8_t, 16>(int8_t* dst, __m128i value) { return _mm_store_si128(reinterpret_cast<__m128i*>(dst), value); }
		template <> inline void store<uint8_t, 16>(uint8_t* dst, __m128i value) { return _mm_store_si128(reinterpret_cast<__m128i*>(dst), value); }
		template <> inline void store<int16_t, 8>(int16_t* dst, __m128i value) { return _mm_store_si128(reinterpret_cast<__m128i*>(dst), value); }
		template <> inline void store<uint16_t, 8>(uint16_t* dst, __m128i value) { return _mm_store_si128(reinterpret_cast<__m128i*>(dst), value); }
		template <> inline void store<int32_t, 4>(int32_t* dst, __m128i value) { return _mm_store_si128(reinterpret_cast<__m128i*>(dst), value); }
		template <> inline void store<uint32_t, 4>(uint32_t* dst, __m128i value) { return _mm_store_si128(reinterpret_cast<__m128i*>(dst), value); }
		template <> inline void store<int64_t, 2>(int64_t* dst, __m128i value) { return _mm_store_si128(reinterpret_cast<__m128i*>(dst), value); }
		template <> inline void store<uint64_t, 2>(uint64_t* dst, __m128i value) { return _mm_store_si128(reinterpret_cast<__m128i*>(dst), value); }
		template <> inline void store<float, 4>(float* dst, __m128 value) { return _mm_store_ps(dst, value); }
		template <> inline void store<double, 2>(double* dst, __m128d value) { return _mm_store_pd(dst, value); }

		template <typename T, size_t Count> inline register_t<T, Count> set1(T);
		template <> inline __m128i set1<int8_t, 16>(int8_t a) { return _mm_set1_epi8(a); }
		template <> inline __m128i set1<uint8_t, 16>(uint8_t a) { return _mm_set1_epi8(a); }
		template <> inline __m128i set1<int16_t, 8>(int16_t a) { return _mm_set1_epi16(a); }
		template <> inline __m128i set1<uint16_t, 8>(uint16_t a) { return _mm_set1_epi16(a); }
		template <> inline __m128i set1<int32_t, 4>(int32_t a) { return _mm_set1_epi32(a); }
		template <> inline __m128i set1<uint32_t, 4>(uint32_t a) { return _mm_set1_epi32(a); }
		template <> inline __m128 set1<float, 4>(float a) { return _mm_set1_ps(a); }
		template <> inline __m128d set1<double, 2>(double a) { return _mm_set1_pd(a); }

		template <typename T> inline register_t<T, 2> set(T, T);
		template <> inline __m128i set(int64_t v1, int64_t v0) { return _mm_set_epi64x(v1, v0); }
		template <> inline __m128i set(uint64_t v1, uint64_t v0) { return _mm_set_epi64x(v1, v0); }
		template <> inline __m128d set(double v1, double v0) { return _mm_set_pd(v1, v0); }

		template <typename T> inline register_t<T, 4> set(T, T, T, T);
		template <> inline __m128i set(int32_t v3, int32_t v2, int32_t v1, int32_t v0) { return _mm_set_epi32(v3, v2, v1, v0); }
		template <> inline __m128i set(uint32_t v3, uint32_t v2, uint32_t v1, uint32_t v0) { return _mm_set_epi32(v3, v2, v1, v0); }
		template <> inline __m128 set(float v3, float v2, float v1, float v0) { return _mm_set_ps(v3, v2, v1, v0); }

		template <typename T> inline register_t<T, 8> set(T, T, T, T, T, T, T, T);
		template <> inline __m128i set(
			int16_t v7, int16_t v6, int16_t v5, int16_t v4,
			int16_t v3, int16_t v2, int16_t v1, int16_t v0
		) { return _mm_set_epi16(v7, v6, v5, v4, v3, v2, v1, v0); }
		template <> inline __m128i set(
			uint16_t v7, uint16_t v6, uint16_t v5, uint16_t v4,
			uint16_t v3, uint16_t v2, uint16_t v1, uint16_t v0
		) { return _mm_set_epi16(v7, v6, v5, v4, v3, v2, v1, v0); }

		template <typename T, size_t Count> inline T extract(register_t<T, Count>, int);
		template <> inline int extract<int, 4>(__m128i a, int i) {
			switch (i) {
			case 0: return _mm_extract_epi32(a, 0);
			case 1: return _mm_extract_epi32(a, 1);
			case 2: return _mm_extract_epi32(a, 2);
			case 3: return _mm_extract_epi32(a, 3);
			default: assert(0); return 0;
			}
		}
		template <> inline uint32_t extract<uint32_t, 4>(__m128i a, int i) {
			switch (i) {
			case 0: return static_cast<uint32_t>(_mm_extract_epi32(a, 0));
			case 1: return static_cast<uint32_t>(_mm_extract_epi32(a, 1));
			case 2: return static_cast<uint32_t>(_mm_extract_epi32(a, 2));
			case 3: return static_cast<uint32_t>(_mm_extract_epi32(a, 3));
			default: assert(0); return 0;
			}
		}
		template <> inline float extract<float, 4>(__m128 a, int i) {
			switch (i) {
			case 0: return _mm_cvtss_f32(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 0)));
			case 1: return _mm_cvtss_f32(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 1)));
			case 2: return _mm_cvtss_f32(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 2)));
			case 3: return _mm_cvtss_f32(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 3)));
			default: assert(0); return 0;
			}
		}

		template <typename t, size_t count>
		inline register_t<t, count> add(register_t<t, count> lhs, register_t<t, count> rhs);
		template <> inline __m128i add<int8_t, 16>(__m128i lhs, __m128i rhs) { return _mm_add_epi8(lhs, rhs); }
		template <> inline __m128i add<uint8_t, 16>(__m128i lhs, __m128i rhs) { return _mm_add_epi8(lhs, rhs); }
		template <> inline __m128i add<int16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_add_epi16(lhs, rhs); }
		template <> inline __m128i add<uint16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_add_epi16(lhs, rhs); }
		template <> inline __m128i add<int32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_add_epi32(lhs, rhs); }
		template <> inline __m128i add<uint32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_add_epi32(lhs, rhs); }
		template <> inline __m128i add<int64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_add_epi64(lhs, rhs); }
		template <> inline __m128i add<uint64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_add_epi64(lhs, rhs); }
		template <> inline __m128 add<float, 4>(__m128 lhs, __m128 rhs) { return _mm_add_ps(lhs, rhs); }
		template <> inline __m128d add<double, 2>(__m128d lhs, __m128d rhs) { return _mm_add_pd(lhs, rhs); }

		template <typename T, size_t Count>
		inline register_t<T, Count> sub(register_t<T, Count> lhs, register_t<T, Count> rhs);
		template <> inline __m128i sub<int8_t, 16>(__m128i lhs, __m128i rhs) { return _mm_sub_epi8(lhs, rhs); }
		template <> inline __m128i sub<uint8_t, 16>(__m128i lhs, __m128i rhs) { return _mm_sub_epi8(lhs, rhs); }
		template <> inline __m128i sub<int16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_sub_epi16(lhs, rhs); }
		template <> inline __m128i sub<uint16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_sub_epi16(lhs, rhs); }
		template <> inline __m128i sub<int32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_sub_epi32(lhs, rhs); }
		template <> inline __m128i sub<uint32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_sub_epi32(lhs, rhs); }
		template <> inline __m128i sub<int64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_sub_epi64(lhs, rhs); }
		template <> inline __m128i sub<uint64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_sub_epi64(lhs, rhs); }
		template <> inline __m128 sub<float, 4>(__m128 lhs, __m128 rhs) { return _mm_sub_ps(lhs, rhs); }
		template <> inline __m128d sub<double, 2>(__m128d lhs, __m128d rhs) { return _mm_sub_pd(lhs, rhs); }

		template <typename T, size_t Count>
		inline register_t<T, Count> mul(register_t<T, Count> lhs, register_t<T, Count> rhs);
		template <> inline __m128i mul<int16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_mullo_epi16(lhs, rhs); }
		template <> inline __m128i mul<uint16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_mullo_epi16(lhs, rhs); }
		template <> inline __m128i mul<int32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_mullo_epi32(lhs, rhs); }
		template <> inline __m128i mul<uint32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_mullo_epi32(lhs, rhs); }
		template <> inline __m128i mul<int64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_mullo_epi64(lhs, rhs); } // AVX512
		template <> inline __m128i mul<uint64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_mullo_epi64(lhs, rhs); } // AVX512
		template <> inline __m128 mul<float, 4>(__m128 lhs, __m128 rhs) { return _mm_mul_ps(lhs, rhs); }
		template <> inline __m128d mul<double, 2>(__m128d lhs, __m128d rhs) { return _mm_mul_pd(lhs, rhs); }

		template <typename T, size_t Count>
		inline register_t<T, Count> div(register_t<T, Count> lhs, register_t<T, Count> rhs);
		template <> inline __m128 div<float, 4>(__m128 lhs, __m128 rhs) { return _mm_div_ps(lhs, rhs); }
		template <> inline __m128d div<double, 2>(__m128d lhs, __m128d rhs) { return _mm_div_pd(lhs, rhs); }

		template <typename T, size_t Count>
		inline register_t<T, Count> fmadd(register_t<T, Count>, register_t<T, Count>, register_t<T, Count>);
		template <> inline __m128 fmadd<float, 4>(__m128 a, __m128 b, __m128 c) { return _mm_fmadd_ps(a, b, c); }
		template <> inline __m128d fmadd<double, 2>(__m128d a, __m128d b, __m128d c) { return _mm_fmadd_pd(a, b, c); }

		template <typename T, size_t Count>
		inline register_t<T, Count> sll(register_t<T, Count> lhs, register_t<T, Count> rhs);
		template <> inline __m128i sll<int16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_sll_epi16(lhs, rhs); }
		template <> inline __m128i sll<uint16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_sll_epi16(lhs, rhs); }
		template <> inline __m128i sll<int32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_sll_epi32(lhs, rhs); }
		template <> inline __m128i sll<uint32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_sll_epi32(lhs, rhs); }
		template <> inline __m128i sll<int64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_sll_epi64(lhs, rhs); }
		template <> inline __m128i sll<uint64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_sll_epi64(lhs, rhs); }

		template <typename T, size_t Count>
		inline register_t<T, Count> sra(register_t<T, Count> lhs, register_t<T, Count> rhs);
		template <> inline __m128i sra<int16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_sra_epi16(lhs, rhs); }
		template <> inline __m128i sra<uint16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_sra_epi16(lhs, rhs); }
		template <> inline __m128i sra<int32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_sra_epi32(lhs, rhs); }
		template <> inline __m128i sra<uint32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_sra_epi32(lhs, rhs); }
		template <> inline __m128i sra<int64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_sra_epi64(lhs, rhs); }
		template <> inline __m128i sra<uint64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_sra_epi64(lhs, rhs); }

		template <typename T, size_t Count>
		inline register_t<T, Count> slli(register_t<T, Count> lhs, int rhs);
		template <> inline __m128i slli<int16_t, 8>(__m128i lhs, int rhs) { return _mm_slli_epi16(lhs, rhs); }
		template <> inline __m128i slli<uint16_t, 8>(__m128i lhs, int rhs) { return _mm_slli_epi16(lhs, rhs); }
		template <> inline __m128i slli<int32_t, 4>(__m128i lhs, int rhs) { return _mm_slli_epi32(lhs, rhs); }
		template <> inline __m128i slli<uint32_t, 4>(__m128i lhs, int rhs) { return _mm_slli_epi32(lhs, rhs); }
		template <> inline __m128i slli<int64_t, 2>(__m128i lhs, int rhs) { return _mm_slli_epi64(lhs, rhs); }
		template <> inline __m128i slli<uint64_t, 2>(__m128i lhs, int rhs) { return _mm_slli_epi64(lhs, rhs); }

		template <typename T, size_t Count>
		inline register_t<T, Count> srai(register_t<T, Count> lhs, int rhs);
		template <> inline __m128i srai<int16_t, 8>(__m128i lhs, int rhs) { return _mm_srai_epi16(lhs, rhs); }
		template <> inline __m128i srai<uint16_t, 8>(__m128i lhs, int rhs) { return _mm_srai_epi16(lhs, rhs); }
		template <> inline __m128i srai<int32_t, 4>(__m128i lhs, int rhs) { return _mm_srai_epi32(lhs, rhs); }
		template <> inline __m128i srai<uint32_t, 4>(__m128i lhs, int rhs) { return _mm_srai_epi32(lhs, rhs); }
		template <> inline __m128i srai<int64_t, 2>(__m128i lhs, int rhs) { return _mm_srai_epi64(lhs, rhs); }
		template <> inline __m128i srai<uint64_t, 2>(__m128i lhs, int rhs) { return _mm_srai_epi64(lhs, rhs); }

		template <typename T, size_t Count>
		inline register_t<T, Count> bit_and(register_t<T, Count> lhs, register_t<T, Count> rhs);
		template <> inline __m128i bit_and<int8_t, 16>(__m128i lhs, __m128i rhs) { return _mm_and_si128(lhs, rhs); }
		template <> inline __m128i bit_and<uint8_t, 16>(__m128i lhs, __m128i rhs) { return _mm_and_si128(lhs, rhs); }
		template <> inline __m128i bit_and<int16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_and_si128(lhs, rhs); }
		template <> inline __m128i bit_and<uint16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_and_si128(lhs, rhs); }
		template <> inline __m128i bit_and<int32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_and_si128(lhs, rhs); }
		template <> inline __m128i bit_and<uint32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_and_si128(lhs, rhs); }
		template <> inline __m128i bit_and<int64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_and_si128(lhs, rhs); }
		template <> inline __m128i bit_and<uint64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_and_si128(lhs, rhs); }
		template <> inline __m128 bit_and<float, 4>(__m128 lhs, __m128 rhs) { return _mm_and_ps(lhs, rhs); }
		template <> inline __m128d bit_and<double, 2>(__m128d lhs, __m128d rhs) { return _mm_and_pd(lhs, rhs); }

		template <typename T, size_t Count>
		inline register_t<T, Count> bit_or(register_t<T, Count> lhs, register_t<T, Count> rhs);
		template <> inline __m128i bit_or<int8_t, 16>(__m128i lhs, __m128i rhs) { return _mm_or_si128(lhs, rhs); }
		template <> inline __m128i bit_or<uint8_t, 16>(__m128i lhs, __m128i rhs) { return _mm_or_si128(lhs, rhs); }
		template <> inline __m128i bit_or<int16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_or_si128(lhs, rhs); }
		template <> inline __m128i bit_or<uint16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_or_si128(lhs, rhs); }
		template <> inline __m128i bit_or<int32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_or_si128(lhs, rhs); }
		template <> inline __m128i bit_or<uint32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_or_si128(lhs, rhs); }
		template <> inline __m128i bit_or<int64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_or_si128(lhs, rhs); }
		template <> inline __m128i bit_or<uint64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_or_si128(lhs, rhs); }
		template <> inline __m128 bit_or<float, 4>(__m128 lhs, __m128 rhs) { return _mm_or_ps(lhs, rhs); }
		template <> inline __m128d bit_or<double, 2>(__m128d lhs, __m128d rhs) { return _mm_or_pd(lhs, rhs); }

		template <typename T, size_t Count>
		inline register_t<T, Count> min(register_t<T, Count> lhs, register_t<T, Count> rhs);
		template <> inline __m128i min<int8_t, 16>(__m128i lhs, __m128i rhs) { return _mm_min_epi8(lhs, rhs); }
		template <> inline __m128i min<uint8_t, 16>(__m128i lhs, __m128i rhs) { return _mm_min_epu8(lhs, rhs); }
		template <> inline __m128i min<int16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_min_epi16(lhs, rhs); }
		template <> inline __m128i min<uint16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_min_epu16(lhs, rhs); }
		template <> inline __m128i min<int32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_min_epi32(lhs, rhs); }
		template <> inline __m128i min<uint32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_min_epu32(lhs, rhs); }
		template <> inline __m128i min<int64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_min_epi64(lhs, rhs); }
		template <> inline __m128i min<uint64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_min_epu64(lhs, rhs); }
		template <> inline __m128 min<float, 4>(__m128 lhs, __m128 rhs) { return _mm_min_ps(lhs, rhs); }
		template <> inline __m128d min<double, 2>(__m128d lhs, __m128d rhs) { return _mm_min_pd(lhs, rhs); }

		template <typename T, size_t Count>
		inline register_t<T, Count> max(register_t<T, Count> lhs, register_t<T, Count> rhs);
		template <> inline __m128i max<int8_t, 16>(__m128i lhs, __m128i rhs) { return _mm_max_epi8(lhs, rhs); }
		template <> inline __m128i max<uint8_t, 16>(__m128i lhs, __m128i rhs) { return _mm_max_epu8(lhs, rhs); }
		template <> inline __m128i max<int16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_max_epi16(lhs, rhs); }
		template <> inline __m128i max<uint16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_max_epu16(lhs, rhs); }
		template <> inline __m128i max<int32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_max_epi32(lhs, rhs); }
		template <> inline __m128i max<uint32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_max_epu32(lhs, rhs); }
		template <> inline __m128i max<int64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_max_epi64(lhs, rhs); }
		template <> inline __m128i max<uint64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_max_epu64(lhs, rhs); }
		template <> inline __m128 max<float, 4>(__m128 lhs, __m128 rhs) { return _mm_max_ps(lhs, rhs); }
		template <> inline __m128d max<double, 2>(__m128d lhs, __m128d rhs) { return _mm_max_pd(lhs, rhs); }

		template <typename T, size_t Count> inline register_t<T, Count> ceil(register_t<T, Count>);
		template <> inline __m128 ceil<float, 4>(__m128 val) { return _mm_ceil_ps(val); }
		template <> inline __m128d ceil<double, 2>(__m128d val) { return _mm_ceil_pd(val); }

		template <typename T, size_t Count> inline register_t<T, Count> floor(register_t<T, Count>);
		template <> inline __m128 floor<float, 4>(__m128 val) { return _mm_floor_ps(val); }
		template <> inline __m128d floor<double, 2>(__m128d val) { return _mm_floor_pd(val); }

		template <typename T, size_t Count> inline register_t<T, Count> round(register_t<T, Count>);
		template <> inline __m128 round<float, 4>(__m128 val) { return _mm_round_ps(val, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC); }
		template <> inline __m128d round<double, 2>(__m128d val) { return _mm_round_pd(val, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC); }

		template <typename U, typename T, size_t Count> inline register_t<U, Count> cvt(register_t<T, Count>);
		template <> inline __m128i cvt<int32_t, float, 4>(__m128 val) { return _mm_cvtps_epi32(val); }
		template <> inline __m128 cvt<float, int32_t, 4>(__m128i val) { return _mm_cvtepi32_ps(val); }

		template <typename T, size_t Count>
		struct _x_ {
			using value_type = T;
			using register_type = register_t<T, Count>;

			register_type m_value;

			inline _x_() : m_value() {}
			inline _x_(register_type Value) : m_value(Value) {}
			inline explicit _x_(T Value) : m_value(set1<T, Count>(Value)) {}
			template <typename U>
			inline explicit _x_(_x_<U, Count> Value) : m_value(cast<T>(Value)) {}

			inline _x_& operator=(register_type Value) { m_value = Value; return *this; }
			inline operator register_type() const { return m_value; }

			inline value_type operator[](int i) { return extract<T, Count>(m_value, i); }
			inline register_type value() const { return m_value; }

			friend inline _x_ operator+(_x_ lhs, _x_ rhs) { return add<T, Count>(lhs, rhs); }
			friend inline _x_ operator-(_x_ lhs, _x_ rhs) { return sub<T, Count>(lhs, rhs); }
			friend inline _x_ operator*(_x_ lhs, _x_ rhs) { return mul<T, Count>(lhs, rhs); }
			friend inline _x_ operator/(_x_ lhs, _x_ rhs) { return div<T, Count>(lhs, rhs); }
			friend inline _x_ operator<<(_x_ lhs, _x_ rhs) { return sll<T, Count>(lhs, rhs); }
			friend inline _x_ operator>>(_x_ lhs, _x_ rhs) { return sra<T, Count>(lhs, rhs); }
			friend inline _x_ operator&(_x_ lhs, _x_ rhs) { return bit_and<T, Count>(lhs, rhs); }
			friend inline _x_ operator|(_x_ lhs, _x_ rhs) { return bit_or<T, Count>(lhs, rhs); }

			friend inline _x_ operator+(_x_ lhs, register_type rhs) { return add<T, Count>(lhs, rhs); }
			friend inline _x_ operator-(_x_ lhs, register_type rhs) { return sub<T, Count>(lhs, rhs); }
			friend inline _x_ operator*(_x_ lhs, register_type rhs) { return mul<T, Count>(lhs, rhs); }
			friend inline _x_ operator/(_x_ lhs, register_type rhs) { return div<T, Count>(lhs, rhs); }
			friend inline _x_ operator<<(_x_ lhs, register_type rhs) { return sll<T, Count>(lhs, rhs); }
			friend inline _x_ operator>>(_x_ lhs, register_type rhs) { return sra<T, Count>(lhs, rhs); }
			friend inline _x_ operator&(_x_ lhs, register_type rhs) { return bit_and<T, Count>(lhs, rhs); }
			friend inline _x_ operator|(_x_ lhs, register_type rhs) { return bit_or<T, Count>(lhs, rhs); }

			friend inline _x_ operator<<(_x_ lhs, int rhs) { return slli<T, Count>(lhs, rhs); }
			friend inline _x_ operator>>(_x_ lhs, int rhs) { return srai<T, Count>(lhs, rhs); }


			inline _x_& operator+=(_x_ rhs) { m_value = add<T, Count>(m_value, rhs); return *this; }
			inline _x_& operator-=(_x_ rhs) { m_value = sub<T, Count>(m_value, rhs); return *this; }
			inline _x_& operator*=(_x_ rhs) { m_value = mul<T, Count>(m_value, rhs); return *this; }
			inline _x_& operator/=(_x_ rhs) { m_value = div<T, Count>(m_value, rhs); return *this; }
			inline _x_& operator<<=(_x_ rhs) { m_value = sll<T, Count>(m_value, rhs); return *this; }
			inline _x_& operator>>=(_x_ rhs) { m_value = sra<T, Count>(m_value, rhs); return *this; }
			inline _x_& operator&=(_x_ rhs) { m_value = bit_and<T, Count>(m_value, rhs); return *this; }
			inline _x_& operator|=(_x_ rhs) { m_value = bit_or<T, Count>(m_value, rhs); return *this; }

			inline _x_& operator+=(register_type rhs) { m_value = add<T, Count>(m_value, rhs); return *this; }
			inline _x_& operator-=(register_type rhs) { m_value = sub<T, Count>(m_value, rhs); return *this; }
			inline _x_& operator*=(register_type rhs) { m_value = mul<T, Count>(m_value, rhs); return *this; }
			inline _x_& operator/=(register_type rhs) { m_value = div<T, Count>(m_value, rhs); return *this; }
			inline _x_& operator<<=(register_type rhs) { m_value = sll<T, Count>(m_value, rhs); return *this; }
			inline _x_& operator>>=(register_type rhs) { m_value = sra<T, Count>(m_value, rhs); return *this; }
			inline _x_& operator&=(register_type rhs) { m_value = bit_and<T, Count>(m_value, rhs); return *this; }
			inline _x_& operator|=(register_type rhs) { m_value = bit_or<T, Count>(m_value, rhs); return *this; }

			inline _x_& operator<<=(int rhs) { m_value = slli<T, Count>(m_value, rhs); return *this; }
			inline _x_& operator>>=(int rhs) { m_value = srai<T, Count>(m_value, rhs); return *this; }
		};

		template <typename T, typename U, size_t Count>
		inline _x_<T, Count> cast(_x_<U, Count> a) {
			return cvt<T, U, Count>(a.value());
		}
		template <typename T, size_t Count>
		inline void store(T* dst, _x_<T, Count> a) {
			store<T, Count>(dst, a.value());
		}

		template <typename T, size_t Count>
		inline _x_<T, Count> ceil(_x_<T, Count> Value) {
			return ceil<T, Count>(Value.value());
		}
		template <typename T, size_t Count>
		inline _x_<T, Count> floor(_x_<T, Count> Value) {
			return floor<T, Count>(Value.value());
		}
		template <typename T, size_t Count>
		inline _x_<T, Count> round(_x_<T, Count> Value) {
			return round<T, Count>(Value.value());
		}

		template <typename T, size_t Count>
		inline _x_<T, Count> clamp(
			_x_<T, Count> Value,
			_x_<T, Count> Min,
			_x_<T, Count> Max
		) {
			return max<T, Count>(min<T, Count>(Value.value(), Max.value()), Min.value());
		}

		template <typename T> using _x2 = _x_<T, 2>;
		template <typename T> using _x4 = _x_<T, 4>;
		template <typename T> using _x8 = _x_<T, 8>;
		template <typename T> using _x16 = _x_<T, 16>;
		template <typename T> using _x32 = _x_<T, 32>;
		template <typename T> using _x64 = _x_<T, 64>;

		template <size_t Count> using i8x_ = _x_<int8_t, Count>;
		template <size_t Count> using u8x_ = _x_<uint8_t, Count>;
		template <size_t Count> using i16x_ = _x_<int16_t, Count>;
		template <size_t Count> using u16x_ = _x_<uint16_t, Count>;
		template <size_t Count> using i32x_ = _x_<int32_t, Count>;
		template <size_t Count> using u32x_ = _x_<uint32_t, Count>;
		template <size_t Count> using i64x_ = _x_<int64_t, Count>;
		template <size_t Count> using u64x_ = _x_<uint64_t, Count>;
		template <size_t Count> using f32x_ = _x_<float, Count>;
		template <size_t Count> using f64x_ = _x_<double, Count>;

		using i8x16 = _x_<int8_t, 16>;
		using u8x16 = _x_<uint8_t, 16>;
		using i16x8 = _x_<int16_t, 8>;
		using u16x8 = _x_<uint16_t, 8>;
		using i32x4 = _x_<int32_t, 4>;
		using u32x4 = _x_<uint32_t, 4>;
		using i64x2 = _x_<int64_t, 2>;
		using u64x2 = _x_<uint64_t, 2>;
		using f32x4 = _x_<float, 4>;
		using f64x2 = _x_<double, 2>;

		template <typename T>
		inline _x2<T> make_x2(T a, T b) {
			return set<T>(a, b);
		}

		template <typename T>
		inline _x4<T> make_x4(T a, T b, T c, T d) {
			return set<T>(a, b, c, d);
		}

		template <typename T>
		inline _x8<T> make_x8(T a, T b, T c, T d, T e, T f, T g, T h) {
			return set<T>(a, b, c, d, e, f, g, h);
		}
	}
}
