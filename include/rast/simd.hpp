#pragma once
#include <immintrin.h>
#include <xmmintrin.h>

namespace rast::simd {
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

	template <> struct _register<int8_t, 32> { using type = __m256i; };
	template <> struct _register<uint8_t, 32> { using type = __m256i; };
	template <> struct _register<int16_t, 16> { using type = __m256i; };
	template <> struct _register<uint16_t, 16> { using type = __m256i; };
	template <> struct _register<int32_t, 8> { using type = __m256i; };
	template <> struct _register<uint32_t, 8> { using type = __m256i; };
	template <> struct _register<int64_t, 4> { using type = __m256i; };
	template <> struct _register<uint64_t, 4> { using type = __m256i; };
	template <> struct _register<float, 8> { using type = __m256; };
	template <> struct _register<double, 4> { using type = __m256d; };

	template <typename T, size_t Count>
	using register_t = typename _register<T, Count>::type;

	template <typename T> inline T setzero();
	template <> inline __m128i setzero<__m128i>() { return _mm_setzero_si128(); }
	template <> inline __m128 setzero<__m128>() { return _mm_setzero_ps(); }
	template <> inline __m128d setzero<__m128d>() { return _mm_setzero_pd(); }
	template <> inline __m256i setzero<__m256i>() { return _mm256_setzero_si256(); }
	template <> inline __m256 setzero<__m256>() { return _mm256_setzero_ps(); }
	template <> inline __m256d setzero<__m256d>() { return _mm256_setzero_pd(); }
	template<typename T, size_t Count> inline register_t<T, Count> setzero() { return setzero<register_t<T, Count>>(); }

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
	template <> inline __m256i load<int8_t, 32>(const int8_t* a) { return _mm256_load_si256(reinterpret_cast<const __m256i*>(a)); }
	template <> inline __m256i load<uint8_t, 32>(const uint8_t* a) { return _mm256_load_si256(reinterpret_cast<const __m256i*>(a)); }
	template <> inline __m256i load<int16_t, 16>(const int16_t* a) { return _mm256_load_si256(reinterpret_cast<const __m256i*>(a)); }
	template <> inline __m256i load<uint16_t, 16>(const uint16_t* a) { return _mm256_load_si256(reinterpret_cast<const __m256i*>(a)); }
	template <> inline __m256i load<int32_t, 8>(const int32_t* a) { return _mm256_load_si256(reinterpret_cast<const __m256i*>(a)); }
	template <> inline __m256i load<uint32_t, 8>(const uint32_t* a) { return _mm256_load_si256(reinterpret_cast<const __m256i*>(a)); }
	template <> inline __m256i load<int64_t, 4>(const int64_t* a) { return _mm256_load_si256(reinterpret_cast<const __m256i*>(a)); }
	template <> inline __m256i load<uint64_t, 4>(const uint64_t* a) { return _mm256_load_si256(reinterpret_cast<const __m256i*>(a)); }
	template <> inline __m256 load<float, 8>(const float* a) { return _mm256_load_ps(a); }
	template <> inline __m256d load<double, 4>(const double* a) { return _mm256_load_pd(a); }

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
	template <> inline void store<int8_t, 32>(int8_t* dst, __m256i value) { return _mm256_store_si256(reinterpret_cast<__m256i*>(dst), value); }
	template <> inline void store<uint8_t, 32>(uint8_t* dst, __m256i value) { return _mm256_store_si256(reinterpret_cast<__m256i*>(dst), value); }
	template <> inline void store<int16_t, 16>(int16_t* dst, __m256i value) { return _mm256_store_si256(reinterpret_cast<__m256i*>(dst), value); }
	template <> inline void store<uint16_t, 16>(uint16_t* dst, __m256i value) { return _mm256_store_si256(reinterpret_cast<__m256i*>(dst), value); }
	template <> inline void store<int32_t, 8>(int32_t* dst, __m256i value) { return _mm256_store_si256(reinterpret_cast<__m256i*>(dst), value); }
	template <> inline void store<uint32_t, 8>(uint32_t* dst, __m256i value) { return _mm256_store_si256(reinterpret_cast<__m256i*>(dst), value); }
	template <> inline void store<int64_t, 4>(int64_t* dst, __m256i value) { return _mm256_store_si256(reinterpret_cast<__m256i*>(dst), value); }
	template <> inline void store<uint64_t, 4>(uint64_t* dst, __m256i value) { return _mm256_store_si256(reinterpret_cast<__m256i*>(dst), value); }
	template <> inline void store<float, 8>(float* dst, __m256 value) { return _mm256_store_ps(dst, value); }
	template <> inline void store<double, 4>(double* dst, __m256d value) { return _mm256_store_pd(dst, value); }

	template <typename T, size_t Count> inline register_t<T, Count> set1(T);
	template <> inline __m128i set1<int8_t, 16>(int8_t a) { return _mm_set1_epi8(a); }
	template <> inline __m128i set1<uint8_t, 16>(uint8_t a) { return _mm_set1_epi8(a); }
	template <> inline __m128i set1<int16_t, 8>(int16_t a) { return _mm_set1_epi16(a); }
	template <> inline __m128i set1<uint16_t, 8>(uint16_t a) { return _mm_set1_epi16(a); }
	template <> inline __m128i set1<int32_t, 4>(int32_t a) { return _mm_set1_epi32(a); }
	template <> inline __m128i set1<uint32_t, 4>(uint32_t a) { return _mm_set1_epi32(a); }
	template <> inline __m128i set1<int64_t, 2>(int64_t a) { return _mm_set1_epi64x(a); }
	template <> inline __m128i set1<uint64_t, 2>(uint64_t a) { return _mm_set1_epi64x(a); }
	template <> inline __m128 set1<float, 4>(float a) { return _mm_set1_ps(a); }
	template <> inline __m128d set1<double, 2>(double a) { return _mm_set1_pd(a); }
	template <> inline __m256i set1<int8_t, 32>(int8_t a) { return _mm256_set1_epi8(a); }
	template <> inline __m256i set1<uint8_t, 32>(uint8_t a) { return _mm256_set1_epi8(a); }
	template <> inline __m256i set1<int16_t, 16>(int16_t a) { return _mm256_set1_epi16(a); }
	template <> inline __m256i set1<uint16_t, 16>(uint16_t a) { return _mm256_set1_epi16(a); }
	template <> inline __m256i set1<int32_t, 8>(int32_t a) { return _mm256_set1_epi32(a); }
	template <> inline __m256i set1<uint32_t, 8>(uint32_t a) { return _mm256_set1_epi32(a); }
	template <> inline __m256i set1<int64_t, 4>(int64_t a) { return _mm256_set1_epi64x(a); }
	template <> inline __m256i set1<uint64_t, 4>(uint64_t a) { return _mm256_set1_epi64x(a); }
	template <> inline __m256 set1<float, 8>(float a) { return _mm256_set1_ps(a); }
	template <> inline __m256d set1<double, 4>(double a) { return _mm256_set1_pd(a); }

	template <typename T> inline register_t<T, 2> set(T, T);
	template <> inline __m128i set(int64_t v1, int64_t v0) { return _mm_set_epi64x(v1, v0); }
	template <> inline __m128i set(uint64_t v1, uint64_t v0) { return _mm_set_epi64x(v1, v0); }
	template <> inline __m128d set(double v1, double v0) { return _mm_set_pd(v1, v0); }

	template <typename T> inline register_t<T, 4> set(T, T, T, T);
	template <> inline __m128i set(int32_t v3, int32_t v2, int32_t v1, int32_t v0) { return _mm_set_epi32(v3, v2, v1, v0); }
	template <> inline __m128i set(uint32_t v3, uint32_t v2, uint32_t v1, uint32_t v0) { return _mm_set_epi32(v3, v2, v1, v0); }
	template <> inline __m128 set(float v3, float v2, float v1, float v0) { return _mm_set_ps(v3, v2, v1, v0); }
	template <> inline __m256i set(int64_t v3, int64_t v2, int64_t v1, int64_t v0) { return _mm256_set_epi64x(v3, v2, v1, v0); }
	template <> inline __m256i set(uint64_t v3, uint64_t v2, uint64_t v1, uint64_t v0) { return _mm256_set_epi64x(v3, v2, v1, v0); }
	template <> inline __m256d set(double v3, double v2, double v1, double v0) { return _mm256_set_pd(v3, v2, v1, v0); }

	template <typename T> inline register_t<T, 8> set(T, T, T, T, T, T, T, T);
	template <> inline __m128i set(
		int16_t v7, int16_t v6, int16_t v5, int16_t v4,
		int16_t v3, int16_t v2, int16_t v1, int16_t v0
	) { return _mm_set_epi16(v7, v6, v5, v4, v3, v2, v1, v0); }
	template <> inline __m128i set(
		uint16_t v7, uint16_t v6, uint16_t v5, uint16_t v4,
		uint16_t v3, uint16_t v2, uint16_t v1, uint16_t v0
	) { return _mm_set_epi16(v7, v6, v5, v4, v3, v2, v1, v0); }
	template <> inline __m256i set(
		int32_t v7, int32_t v6, int32_t v5, int32_t v4,
		int32_t v3, int32_t v2, int32_t v1, int32_t v0
	) { return _mm256_set_epi32(v7, v6, v5, v4, v3, v2, v1, v0); }
	template <> inline __m256i set(
		uint32_t v7, uint32_t v6, uint32_t v5, uint32_t v4,
		uint32_t v3, uint32_t v2, uint32_t v1, uint32_t v0
	) { return _mm256_set_epi32(v7, v6, v5, v4, v3, v2, v1, v0); }
	template <> inline __m256 set(
		float v7, float v6, float v5, float v4,
		float v3, float v2, float v1, float v0
	) { return _mm256_set_ps(v7, v6, v5, v4, v3, v2, v1, v0); }

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
	template <> inline int extract<int, 8>(__m256i a, int i) {
		switch (i) {
		case 0: return _mm256_extract_epi32(a, 0);
		case 1: return _mm256_extract_epi32(a, 1);
		case 2: return _mm256_extract_epi32(a, 2);
		case 3: return _mm256_extract_epi32(a, 3);
		case 4: return _mm256_extract_epi32(a, 4);
		case 5: return _mm256_extract_epi32(a, 5);
		case 6: return _mm256_extract_epi32(a, 6);
		case 7: return _mm256_extract_epi32(a, 7);
		default: assert(0); return 0;
		}
	}
	template <> inline uint32_t extract<uint32_t, 8>(__m256i a, int i) {
		switch (i) {
		case 0: return static_cast<uint32_t>(_mm256_extract_epi32(a, 0));
		case 1: return static_cast<uint32_t>(_mm256_extract_epi32(a, 1));
		case 2: return static_cast<uint32_t>(_mm256_extract_epi32(a, 2));
		case 3: return static_cast<uint32_t>(_mm256_extract_epi32(a, 3));
		case 4: return static_cast<uint32_t>(_mm256_extract_epi32(a, 4));
		case 5: return static_cast<uint32_t>(_mm256_extract_epi32(a, 5));
		case 6: return static_cast<uint32_t>(_mm256_extract_epi32(a, 6));
		case 7: return static_cast<uint32_t>(_mm256_extract_epi32(a, 7));
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
	template <> inline float extract<float, 8>(__m256 a, int i) {
		switch (i) {
		case 0: return _mm256_cvtss_f32(_mm256_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 0)));
		case 1: return _mm256_cvtss_f32(_mm256_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 1)));
		case 2: return _mm256_cvtss_f32(_mm256_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 2)));
		case 3: return _mm256_cvtss_f32(_mm256_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 3)));
		case 4: return _mm256_cvtss_f32(_mm256_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 4)));
		case 5: return _mm256_cvtss_f32(_mm256_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 5)));
		case 6: return _mm256_cvtss_f32(_mm256_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 6)));
		case 7: return _mm256_cvtss_f32(_mm256_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 7)));
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
	template <> inline __m256i add<int8_t, 32>(__m256i lhs, __m256i rhs) { return _mm256_add_epi8(lhs, rhs); }
	template <> inline __m256i add<uint8_t, 32>(__m256i lhs, __m256i rhs) { return _mm256_add_epi8(lhs, rhs); }
	template <> inline __m256i add<int16_t, 16>(__m256i lhs, __m256i rhs) { return _mm256_add_epi16(lhs, rhs); }
	template <> inline __m256i add<uint16_t, 16>(__m256i lhs, __m256i rhs) { return _mm256_add_epi16(lhs, rhs); }
	template <> inline __m256i add<int32_t, 8>(__m256i lhs, __m256i rhs) { return _mm256_add_epi32(lhs, rhs); }
	template <> inline __m256i add<uint32_t, 8>(__m256i lhs, __m256i rhs) { return _mm256_add_epi32(lhs, rhs); }
	template <> inline __m256i add<int64_t, 4>(__m256i lhs, __m256i rhs) { return _mm256_add_epi64(lhs, rhs); }
	template <> inline __m256i add<uint64_t, 4>(__m256i lhs, __m256i rhs) { return _mm256_add_epi64(lhs, rhs); }
	template <> inline __m256 add<float, 8>(__m256 lhs, __m256 rhs) { return _mm256_add_ps(lhs, rhs); }
	template <> inline __m256d add<double, 4>(__m256d lhs, __m256d rhs) { return _mm256_add_pd(lhs, rhs); }

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
	template <> inline __m256i sub<int8_t, 32>(__m256i lhs, __m256i rhs) { return _mm256_sub_epi8(lhs, rhs); }
	template <> inline __m256i sub<uint8_t, 32>(__m256i lhs, __m256i rhs) { return _mm256_sub_epi8(lhs, rhs); }
	template <> inline __m256i sub<int16_t, 16>(__m256i lhs, __m256i rhs) { return _mm256_sub_epi16(lhs, rhs); }
	template <> inline __m256i sub<uint16_t, 16>(__m256i lhs, __m256i rhs) { return _mm256_sub_epi16(lhs, rhs); }
	template <> inline __m256i sub<int32_t, 8>(__m256i lhs, __m256i rhs) { return _mm256_sub_epi32(lhs, rhs); }
	template <> inline __m256i sub<uint32_t, 8>(__m256i lhs, __m256i rhs) { return _mm256_sub_epi32(lhs, rhs); }
	template <> inline __m256i sub<int64_t, 4>(__m256i lhs, __m256i rhs) { return _mm256_sub_epi64(lhs, rhs); }
	template <> inline __m256i sub<uint64_t, 4>(__m256i lhs, __m256i rhs) { return _mm256_sub_epi64(lhs, rhs); }
	template <> inline __m256 sub<float, 8>(__m256 lhs, __m256 rhs) { return _mm256_sub_ps(lhs, rhs); }
	template <> inline __m256d sub<double, 4>(__m256d lhs, __m256d rhs) { return _mm256_sub_pd(lhs, rhs); }

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
	template <> inline __m256i mul<int16_t, 16>(__m256i lhs, __m256i rhs) { return _mm256_mullo_epi16(lhs, rhs); }
	template <> inline __m256i mul<uint16_t, 16>(__m256i lhs, __m256i rhs) { return _mm256_mullo_epi16(lhs, rhs); }
	template <> inline __m256i mul<int32_t, 8>(__m256i lhs, __m256i rhs) { return _mm256_mullo_epi32(lhs, rhs); }
	template <> inline __m256i mul<uint32_t, 8>(__m256i lhs, __m256i rhs) { return _mm256_mullo_epi32(lhs, rhs); }
	template <> inline __m256i mul<int64_t, 4>(__m256i lhs, __m256i rhs) { return _mm256_mullo_epi64(lhs, rhs); } // AVX512
	template <> inline __m256i mul<uint64_t, 4>(__m256i lhs, __m256i rhs) { return _mm256_mullo_epi64(lhs, rhs); } // AVX512
	template <> inline __m256 mul<float, 8>(__m256 lhs, __m256 rhs) { return _mm256_mul_ps(lhs, rhs); }
	template <> inline __m256d mul<double, 4>(__m256d lhs, __m256d rhs) { return _mm256_mul_pd(lhs, rhs); }

	template <typename T, size_t Count>
	inline register_t<T, Count> div(register_t<T, Count> lhs, register_t<T, Count> rhs);
	template <> inline __m128 div<float, 4>(__m128 lhs, __m128 rhs) { return _mm_div_ps(lhs, rhs); }
	template <> inline __m128d div<double, 2>(__m128d lhs, __m128d rhs) { return _mm_div_pd(lhs, rhs); }
	template <> inline __m256 div<float, 8>(__m256 lhs, __m256 rhs) { return _mm256_div_ps(lhs, rhs); }
	template <> inline __m256d div<double, 4>(__m256d lhs, __m256d rhs) { return _mm256_div_pd(lhs, rhs); }

	template <typename T, size_t Count>
	inline register_t<T, Count> fmadd(register_t<T, Count>, register_t<T, Count>, register_t<T, Count>);
	template <> inline __m128 fmadd<float, 4>(__m128 a, __m128 b, __m128 c) { return _mm_fmadd_ps(a, b, c); }
	template <> inline __m128d fmadd<double, 2>(__m128d a, __m128d b, __m128d c) { return _mm_fmadd_pd(a, b, c); }
	template <> inline __m256 fmadd<float, 8>(__m256 a, __m256 b, __m256 c) { return _mm256_fmadd_ps(a, b, c); }
	template <> inline __m256d fmadd<double, 4>(__m256d a, __m256d b, __m256d c) { return _mm256_fmadd_pd(a, b, c); }

	template <typename T, size_t Count>
	inline register_t<T, Count> sll(register_t<T, Count> lhs, register_t<T, Count> rhs);
	template <> inline __m128i sll<int16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_sll_epi16(lhs, rhs); }
	template <> inline __m128i sll<uint16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_sll_epi16(lhs, rhs); }
	template <> inline __m128i sll<int32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_sll_epi32(lhs, rhs); }
	template <> inline __m128i sll<uint32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_sll_epi32(lhs, rhs); }
	template <> inline __m128i sll<int64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_sll_epi64(lhs, rhs); }
	template <> inline __m128i sll<uint64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_sll_epi64(lhs, rhs); }
	template <> inline __m256i sll<int16_t, 16>(__m256i lhs, __m256i rhs) { return _mm256_sllv_epi16(lhs, rhs); } // AVX512
	template <> inline __m256i sll<uint16_t, 16>(__m256i lhs, __m256i rhs) { return _mm256_sllv_epi16(lhs, rhs); } // AVX512
	template <> inline __m256i sll<int32_t, 8>(__m256i lhs, __m256i rhs) { return _mm256_sllv_epi32(lhs, rhs); }
	template <> inline __m256i sll<uint32_t, 8>(__m256i lhs, __m256i rhs) { return _mm256_sllv_epi32(lhs, rhs); }
	template <> inline __m256i sll<int64_t, 4>(__m256i lhs, __m256i rhs) { return _mm256_sllv_epi64(lhs, rhs); }
	template <> inline __m256i sll<uint64_t, 4>(__m256i lhs, __m256i rhs) { return _mm256_sllv_epi64(lhs, rhs); }

	template <typename T, size_t Count>
	inline register_t<T, Count> sra(register_t<T, Count> lhs, register_t<T, Count> rhs);
	template <> inline __m128i sra<int16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_sra_epi16(lhs, rhs); }
	template <> inline __m128i sra<uint16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_sra_epi16(lhs, rhs); }
	template <> inline __m128i sra<int32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_sra_epi32(lhs, rhs); }
	template <> inline __m128i sra<uint32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_sra_epi32(lhs, rhs); }
	template <> inline __m128i sra<int64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_sra_epi64(lhs, rhs); }
	template <> inline __m128i sra<uint64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_sra_epi64(lhs, rhs); }
	template <> inline __m256i sra<int16_t, 16>(__m256i lhs, __m256i rhs) { return _mm256_srav_epi16(lhs, rhs); }
	template <> inline __m256i sra<uint16_t, 16>(__m256i lhs, __m256i rhs) { return _mm256_srav_epi16(lhs, rhs); }
	template <> inline __m256i sra<int32_t, 8>(__m256i lhs, __m256i rhs) { return _mm256_srav_epi32(lhs, rhs); }
	template <> inline __m256i sra<uint32_t, 8>(__m256i lhs, __m256i rhs) { return _mm256_srav_epi32(lhs, rhs); }
	template <> inline __m256i sra<int64_t, 4>(__m256i lhs, __m256i rhs) { return _mm256_srav_epi64(lhs, rhs); }
	template <> inline __m256i sra<uint64_t, 4>(__m256i lhs, __m256i rhs) { return _mm256_srav_epi64(lhs, rhs); }

	template <typename T, size_t Count>
	inline register_t<T, Count> slli(register_t<T, Count> lhs, int rhs);
	template <> inline __m128i slli<int16_t, 8>(__m128i lhs, int rhs) { return _mm_slli_epi16(lhs, rhs); }
	template <> inline __m128i slli<uint16_t, 8>(__m128i lhs, int rhs) { return _mm_slli_epi16(lhs, rhs); }
	template <> inline __m128i slli<int32_t, 4>(__m128i lhs, int rhs) { return _mm_slli_epi32(lhs, rhs); }
	template <> inline __m128i slli<uint32_t, 4>(__m128i lhs, int rhs) { return _mm_slli_epi32(lhs, rhs); }
	template <> inline __m128i slli<int64_t, 2>(__m128i lhs, int rhs) { return _mm_slli_epi64(lhs, rhs); }
	template <> inline __m128i slli<uint64_t, 2>(__m128i lhs, int rhs) { return _mm_slli_epi64(lhs, rhs); }
	template <> inline __m256i slli<int16_t, 16>(__m256i lhs, int rhs) { return _mm256_slli_epi16(lhs, rhs); }
	template <> inline __m256i slli<uint16_t, 16>(__m256i lhs, int rhs) { return _mm256_slli_epi16(lhs, rhs); }
	template <> inline __m256i slli<int32_t, 8>(__m256i lhs, int rhs) { return _mm256_slli_epi32(lhs, rhs); }
	template <> inline __m256i slli<uint32_t, 8>(__m256i lhs, int rhs) { return _mm256_slli_epi32(lhs, rhs); }
	template <> inline __m256i slli<int64_t, 4>(__m256i lhs, int rhs) { return _mm256_slli_epi64(lhs, rhs); }
	template <> inline __m256i slli<uint64_t, 4>(__m256i lhs, int rhs) { return _mm256_slli_epi64(lhs, rhs); }

	template <typename T, size_t Count>
	inline register_t<T, Count> srai(register_t<T, Count> lhs, int rhs);
	template <> inline __m128i srai<int16_t, 8>(__m128i lhs, int rhs) { return _mm_srai_epi16(lhs, rhs); }
	template <> inline __m128i srai<uint16_t, 8>(__m128i lhs, int rhs) { return _mm_srai_epi16(lhs, rhs); }
	template <> inline __m128i srai<int32_t, 4>(__m128i lhs, int rhs) { return _mm_srai_epi32(lhs, rhs); }
	template <> inline __m128i srai<uint32_t, 4>(__m128i lhs, int rhs) { return _mm_srai_epi32(lhs, rhs); }
	template <> inline __m128i srai<int64_t, 2>(__m128i lhs, int rhs) { return _mm_srai_epi64(lhs, rhs); }
	template <> inline __m128i srai<uint64_t, 2>(__m128i lhs, int rhs) { return _mm_srai_epi64(lhs, rhs); }
	template <> inline __m256i srai<int16_t, 16>(__m256i lhs, int rhs) { return _mm256_srai_epi16(lhs, rhs); }
	template <> inline __m256i srai<uint16_t, 16>(__m256i lhs, int rhs) { return _mm256_srai_epi16(lhs, rhs); }
	template <> inline __m256i srai<int32_t, 8>(__m256i lhs, int rhs) { return _mm256_srai_epi32(lhs, rhs); }
	template <> inline __m256i srai<uint32_t, 8>(__m256i lhs, int rhs) { return _mm256_srai_epi32(lhs, rhs); }
	template <> inline __m256i srai<int64_t, 4>(__m256i lhs, int rhs) { return _mm256_srai_epi64(lhs, rhs); }
	template <> inline __m256i srai<uint64_t, 4>(__m256i lhs, int rhs) { return _mm256_srai_epi64(lhs, rhs); }

	template <typename T>
	inline T bit_and(T lhs, T rhs);
	template <> inline __m128i bit_and(__m128i lhs, __m128i rhs) { return _mm_and_si128(lhs, rhs); }
	template <> inline __m128 bit_and(__m128 lhs, __m128 rhs) { return _mm_and_ps(lhs, rhs); }
	template <> inline __m128d bit_and(__m128d lhs, __m128d rhs) { return _mm_and_pd(lhs, rhs); }
	template <> inline __m256i bit_and(__m256i lhs, __m256i rhs) { return _mm256_and_si256(lhs, rhs); }
	template <> inline __m256 bit_and(__m256 lhs, __m256 rhs) { return _mm256_and_ps(lhs, rhs); }
	template <> inline __m256d bit_and(__m256d lhs, __m256d rhs) { return _mm256_and_pd(lhs, rhs); }
	template <typename T, size_t Count>
	inline register_t<T, Count> bit_and(register_t<T, Count> lhs, register_t<T, Count> rhs) {
		return bit_and<register_t<T, Count>>(lhs, rhs);
	}

	template <typename T>
	inline T bit_or(T lhs, T rhs);
	template <> inline __m128i bit_or(__m128i lhs, __m128i rhs) { return _mm_or_si128(lhs, rhs); }
	template <> inline __m128 bit_or(__m128 lhs, __m128 rhs) { return _mm_or_ps(lhs, rhs); }
	template <> inline __m128d bit_or(__m128d lhs, __m128d rhs) { return _mm_or_pd(lhs, rhs); }
	template <> inline __m256i bit_or(__m256i lhs, __m256i rhs) { return _mm256_or_si256(lhs, rhs); }
	template <> inline __m256 bit_or(__m256 lhs, __m256 rhs) { return _mm256_or_ps(lhs, rhs); }
	template <> inline __m256d bit_or(__m256d lhs, __m256d rhs) { return _mm256_or_pd(lhs, rhs); }
	template <typename T, size_t Count>
	inline register_t<T, Count> bit_or(register_t<T, Count> lhs, register_t<T, Count> rhs) {
		return bit_or<register_t<T, Count>>(lhs, rhs);
	}

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
	template <> inline __m256i min<int8_t, 32>(__m256i lhs, __m256i rhs) { return _mm256_min_epi8(lhs, rhs); }
	template <> inline __m256i min<uint8_t, 32>(__m256i lhs, __m256i rhs) { return _mm256_min_epu8(lhs, rhs); }
	template <> inline __m256i min<int16_t, 16>(__m256i lhs, __m256i rhs) { return _mm256_min_epi16(lhs, rhs); }
	template <> inline __m256i min<uint16_t, 16>(__m256i lhs, __m256i rhs) { return _mm256_min_epu16(lhs, rhs); }
	template <> inline __m256i min<int32_t, 8>(__m256i lhs, __m256i rhs) { return _mm256_min_epi32(lhs, rhs); }
	template <> inline __m256i min<uint32_t, 8>(__m256i lhs, __m256i rhs) { return _mm256_min_epu32(lhs, rhs); }
	template <> inline __m256i min<int64_t, 4>(__m256i lhs, __m256i rhs) { return _mm256_min_epi64(lhs, rhs); }
	template <> inline __m256i min<uint64_t, 4>(__m256i lhs, __m256i rhs) { return _mm256_min_epu64(lhs, rhs); }
	template <> inline __m256 min<float, 8>(__m256 lhs, __m256 rhs) { return _mm256_min_ps(lhs, rhs); }
	template <> inline __m256d min<double, 4>(__m256d lhs, __m256d rhs) { return _mm256_min_pd(lhs, rhs); }

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
	template <> inline __m256i max<int8_t, 32>(__m256i lhs, __m256i rhs) { return _mm256_max_epi8(lhs, rhs); }
	template <> inline __m256i max<uint8_t, 32>(__m256i lhs, __m256i rhs) { return _mm256_max_epu8(lhs, rhs); }
	template <> inline __m256i max<int16_t, 16>(__m256i lhs, __m256i rhs) { return _mm256_max_epi16(lhs, rhs); }
	template <> inline __m256i max<uint16_t, 16>(__m256i lhs, __m256i rhs) { return _mm256_max_epu16(lhs, rhs); }
	template <> inline __m256i max<int32_t, 8>(__m256i lhs, __m256i rhs) { return _mm256_max_epi32(lhs, rhs); }
	template <> inline __m256i max<uint32_t, 8>(__m256i lhs, __m256i rhs) { return _mm256_max_epu32(lhs, rhs); }
	template <> inline __m256i max<int64_t, 4>(__m256i lhs, __m256i rhs) { return _mm256_max_epi64(lhs, rhs); }
	template <> inline __m256i max<uint64_t, 4>(__m256i lhs, __m256i rhs) { return _mm256_max_epu64(lhs, rhs); }
	template <> inline __m256 max<float, 8>(__m256 lhs, __m256 rhs) { return _mm256_max_ps(lhs, rhs); }
	template <> inline __m256d max<double, 4>(__m256d lhs, __m256d rhs) { return _mm256_max_pd(lhs, rhs); }

	template <typename T, size_t Count>
	inline register_t<T, Count> cmpeq(register_t<T, Count> lhs, register_t<T, Count> rhs);
	template <> inline __m128i cmpeq<int8_t, 16>(__m128i lhs, __m128i rhs) { return _mm_cmpeq_epi8(lhs, rhs); }
	template <> inline __m128i cmpeq<uint8_t, 16>(__m128i lhs, __m128i rhs) { return _mm_cmpeq_epi8(lhs, rhs); }
	template <> inline __m128i cmpeq<int16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_cmpeq_epi16(lhs, rhs); }
	template <> inline __m128i cmpeq<uint16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_cmpeq_epi16(lhs, rhs); }
	template <> inline __m128i cmpeq<int32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_cmpeq_epi32(lhs, rhs); }
	template <> inline __m128i cmpeq<uint32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_cmpeq_epi32(lhs, rhs); }
	template <> inline __m128i cmpeq<int64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_cmpeq_epi64(lhs, rhs); }
	template <> inline __m128i cmpeq<uint64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_cmpeq_epi64(lhs, rhs); }
	template <> inline __m128 cmpeq<float, 4>(__m128 lhs, __m128 rhs) { return _mm_cmpeq_ps(lhs, rhs); }
	template <> inline __m128d cmpeq<double, 2>(__m128d lhs, __m128d rhs) { return _mm_cmpeq_pd(lhs, rhs); }
	template <> inline __m256i cmpeq<int8_t, 32>(__m256i lhs, __m256i rhs) { return _mm256_cmpeq_epi8(lhs, rhs); }
	template <> inline __m256i cmpeq<uint8_t, 32>(__m256i lhs, __m256i rhs) { return _mm256_cmpeq_epi8(lhs, rhs); }
	template <> inline __m256i cmpeq<int16_t, 16>(__m256i lhs, __m256i rhs) { return _mm256_cmpeq_epi16(lhs, rhs); }
	template <> inline __m256i cmpeq<uint16_t, 16>(__m256i lhs, __m256i rhs) { return _mm256_cmpeq_epi16(lhs, rhs); }
	template <> inline __m256i cmpeq<int32_t, 8>(__m256i lhs, __m256i rhs) { return _mm256_cmpeq_epi32(lhs, rhs); }
	template <> inline __m256i cmpeq<uint32_t, 8>(__m256i lhs, __m256i rhs) { return _mm256_cmpeq_epi32(lhs, rhs); }
	template <> inline __m256i cmpeq<int64_t, 4>(__m256i lhs, __m256i rhs) { return _mm256_cmpeq_epi64(lhs, rhs); }
	template <> inline __m256i cmpeq<uint64_t, 4>(__m256i lhs, __m256i rhs) { return _mm256_cmpeq_epi64(lhs, rhs); }
	template <> inline __m256 cmpeq<float, 8>(__m256 lhs, __m256 rhs) { return _mm256_cmp_ps(lhs, rhs, _CMP_EQ_OQ); }
	template <> inline __m256d cmpeq<double, 4>(__m256d lhs, __m256d rhs) { return _mm256_cmp_pd(lhs, rhs, _CMP_EQ_OQ); }

	template <typename T, size_t Count>
	inline register_t<T, Count> cmpgt(register_t<T, Count> lhs, register_t<T, Count> rhs);
	template <> inline __m128i cmpgt<int8_t, 16>(__m128i lhs, __m128i rhs) { return _mm_cmpgt_epi8(lhs, rhs); }
	template <> inline __m128i cmpgt<uint8_t, 16>(__m128i lhs, __m128i rhs) { return _mm_cmpgt_epi8(lhs, rhs); }
	template <> inline __m128i cmpgt<int16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_cmpgt_epi16(lhs, rhs); }
	template <> inline __m128i cmpgt<uint16_t, 8>(__m128i lhs, __m128i rhs) { return _mm_cmpgt_epi16(lhs, rhs); }
	template <> inline __m128i cmpgt<int32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_cmpgt_epi32(lhs, rhs); }
	template <> inline __m128i cmpgt<uint32_t, 4>(__m128i lhs, __m128i rhs) { return _mm_cmpgt_epi32(lhs, rhs); }
	template <> inline __m128i cmpgt<int64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_cmpgt_epi64(lhs, rhs); }
	template <> inline __m128i cmpgt<uint64_t, 2>(__m128i lhs, __m128i rhs) { return _mm_cmpgt_epi64(lhs, rhs); }
	template <> inline __m128 cmpgt<float, 4>(__m128 lhs, __m128 rhs) { return _mm_cmpgt_ps(lhs, rhs); }
	template <> inline __m128d cmpgt<double, 2>(__m128d lhs, __m128d rhs) { return _mm_cmpgt_pd(lhs, rhs); }
	template <> inline __m256i cmpgt<int8_t, 32>(__m256i lhs, __m256i rhs) { return _mm256_cmpgt_epi8(lhs, rhs); }
	template <> inline __m256i cmpgt<uint8_t, 32>(__m256i lhs, __m256i rhs) { return _mm256_cmpgt_epi8(lhs, rhs); }
	template <> inline __m256i cmpgt<int16_t, 16>(__m256i lhs, __m256i rhs) { return _mm256_cmpgt_epi16(lhs, rhs); }
	template <> inline __m256i cmpgt<uint16_t, 16>(__m256i lhs, __m256i rhs) { return _mm256_cmpgt_epi16(lhs, rhs); }
	template <> inline __m256i cmpgt<int32_t, 8>(__m256i lhs, __m256i rhs) { return _mm256_cmpgt_epi32(lhs, rhs); }
	template <> inline __m256i cmpgt<uint32_t, 8>(__m256i lhs, __m256i rhs) { return _mm256_cmpgt_epi32(lhs, rhs); }
	template <> inline __m256i cmpgt<int64_t, 4>(__m256i lhs, __m256i rhs) { return _mm256_cmpgt_epi64(lhs, rhs); }
	template <> inline __m256i cmpgt<uint64_t, 4>(__m256i lhs, __m256i rhs) { return _mm256_cmpgt_epi64(lhs, rhs); }
	template <> inline __m256 cmpgt<float, 8>(__m256 lhs, __m256 rhs) { return _mm256_cmp_ps(lhs, rhs, _CMP_GT_OQ); }
	template <> inline __m256d cmpgt<double, 4>(__m256d lhs, __m256d rhs) { return _mm256_cmp_pd(lhs, rhs, _CMP_GT_OQ); }

	template <typename T, size_t Count> inline register_t<T, Count> ceil(register_t<T, Count>);
	template <> inline __m128 ceil<float, 4>(__m128 val) { return _mm_ceil_ps(val); }
	template <> inline __m128d ceil<double, 2>(__m128d val) { return _mm_ceil_pd(val); }
	template <> inline __m256 ceil<float, 8>(__m256 val) { return _mm256_ceil_ps(val); }
	template <> inline __m256d ceil<double, 4>(__m256d val) { return _mm256_ceil_pd(val); }

	template <typename T, size_t Count> inline register_t<T, Count> floor(register_t<T, Count>);
	template <> inline __m128 floor<float, 4>(__m128 val) { return _mm_floor_ps(val); }
	template <> inline __m128d floor<double, 2>(__m128d val) { return _mm_floor_pd(val); }
	template <> inline __m256 floor<float, 8>(__m256 val) { return _mm256_floor_ps(val); }
	template <> inline __m256d floor<double, 4>(__m256d val) { return _mm256_floor_pd(val); }

	template <typename T, size_t Count> inline register_t<T, Count> round(register_t<T, Count>);
	template <> inline __m128 round<float, 4>(__m128 val) { return _mm_round_ps(val, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC); }
	template <> inline __m128d round<double, 2>(__m128d val) { return _mm_round_pd(val, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC); }

	template <typename T, size_t Count> inline register_t<T, Count> rsqrt(register_t<T, Count>);
	template <> inline __m128 rsqrt<float, 4>(__m128 val) { return _mm_rsqrt_ps(val); }
	template <> inline __m256 rsqrt<float, 8>(__m256 val) { return _mm256_rsqrt_ps(val); }

	template <typename T, size_t Count> inline register_t<T, Count> sqrt(register_t<T, Count>);
	template <> inline __m128 sqrt<float, 4>(__m128 val) { return _mm_sqrt_ps(val); }
	template <> inline __m128d sqrt<double, 2>(__m128d val) { return _mm_sqrt_pd(val); }
	template <> inline __m256 sqrt<float, 8>(__m256 val) { return _mm256_sqrt_ps(val); }
	template <> inline __m256d sqrt<double, 4>(__m256d val) { return _mm256_sqrt_pd(val); }

	template <typename U, typename T, size_t Count> inline register_t<U, Count> cvt(register_t<T, Count>);
	template <> inline __m128i cvt<int32_t, uint32_t, 4>(__m128i val) { return val; }
	template <> inline __m128i cvt<uint32_t, int32_t, 4>(__m128i val) { return val; }
	template <> inline __m128i cvt<int32_t, float, 4>(__m128 val) { return _mm_cvtps_epi32(val); }
	template <> inline __m128 cvt<float, int32_t, 4>(__m128i val) { return _mm_cvtepi32_ps(val); }
	template <> inline __m256i cvt<int32_t, uint32_t, 8>(__m256i val) { return val; }
	template <> inline __m256i cvt<uint32_t, int32_t, 8>(__m256i val) { return val; }
	template <> inline __m256i cvt<int32_t, float, 8>(__m256 val) { return _mm256_cvtps_epi32(val); }
	template <> inline __m256 cvt<float, int32_t, 8>(__m256i val) { return _mm256_cvtepi32_ps(val); }

	template <typename T, size_t Count>
	inline register_t<T, Count> mod(register_t<T, Count> a, register_t<T, Count> b) {
		register_t<float, Count> af = cvt<float, T, Count>(a);
		register_t<float, Count> bf = cvt<float, T, Count>(b);
		register_t<float, Count> qf = div<float, Count>(af, bf);
		register_t<T, Count> q = cvt<T, float, Count>(floor<float, Count>(qf));
		register_t<T, Count> qb = mul<T, Count>(q, b);
		return sub<T, Count>(a, qb);
	}

	template <typename T, size_t Count>
	struct _x_ {
		using value_type = T;
		using register_type = register_t<T, Count>;

		register_type m_value;

		inline _x_() = default;
		inline _x_(register_type Value) : m_value(Value) {}
		inline explicit _x_(T Value) : m_value(set1<T, Count>(Value)) {}
		template <typename U>
		inline explicit _x_(_x_<U, Count> Value) : m_value(cvt<T, U, Count>(Value)) {}

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
		friend inline _x_ operator==(_x_ lhs, _x_ rhs) { return cmpeq<T, Count>(lhs, rhs); }
		friend inline _x_ operator<(_x_ lhs, _x_ rhs) { return cmpgt<T, Count>(rhs, lhs); }
		friend inline _x_ operator>(_x_ lhs, _x_ rhs) { return cmpgt<T, Count>(lhs, rhs); }
		friend inline _x_ operator<=(_x_ lhs, _x_ rhs) { return (lhs < rhs) | (lhs == rhs); }
		friend inline _x_ operator>=(_x_ lhs, _x_ rhs) { return (lhs > rhs) | (lhs == rhs); }

		friend inline _x_ operator+(_x_ lhs, register_type rhs) { return add<T, Count>(lhs, rhs); }
		friend inline _x_ operator-(_x_ lhs, register_type rhs) { return sub<T, Count>(lhs, rhs); }
		friend inline _x_ operator*(_x_ lhs, register_type rhs) { return mul<T, Count>(lhs, rhs); }
		friend inline _x_ operator/(_x_ lhs, register_type rhs) { return div<T, Count>(lhs, rhs); }
		friend inline _x_ operator<<(_x_ lhs, register_type rhs) { return sll<T, Count>(lhs, rhs); }
		friend inline _x_ operator>>(_x_ lhs, register_type rhs) { return sra<T, Count>(lhs, rhs); }
		friend inline _x_ operator&(_x_ lhs, register_type rhs) { return bit_and<T, Count>(lhs, rhs); }
		friend inline _x_ operator|(_x_ lhs, register_type rhs) { return bit_or<T, Count>(lhs, rhs); }
		friend inline _x_ operator==(_x_ lhs, register_type rhs) { return cmpeq<T, Count>(lhs, rhs); }
		friend inline _x_ operator<(_x_ lhs, register_type rhs) { return cmpgt<T, Count>(rhs, lhs); }
		friend inline _x_ operator>(_x_ lhs, register_type rhs) { return cmpgt<T, Count>(lhs, rhs); }

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

	template <typename T, size_t Count>
	inline _x_<T, Count> fmadd(
		_x_<T, Count> A, _x_<T, Count> B, _x_<T, Count> C
	) {
		return fmadd<T, Count>(A.value(), B.value(), C.value());
	}

	template <typename T, typename U, size_t Count>
	inline _x_<T, Count> cast(_x_<U, Count> a) {
		return cvt<T, U, Count>(a.value());
	}
	template <typename T, size_t Count>
	inline void store(T* dst, _x_<T, Count> a) {
		store<T, Count>(dst, a.value());
	}

	template <typename T, size_t Count>
	inline _x_<T, Count> min(
		_x_<T, Count> a, _x_<T, Count> b
	) {
		return min<T, Count>(a.value(), b.value());
	}
	template <typename T, size_t Count>
	inline _x_<T, Count> max(
		_x_<T, Count> a, _x_<T, Count> b
	) {
		return max<T, Count>(a.value(), b.value());
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
	inline _x_<T, Count> sqrt(_x_<T, Count> Value) {
		return sqrt<T, Count>(Value.value());
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
	using i8x32 = _x_<int8_t, 32>;
	using u8x32 = _x_<uint8_t, 32>;
	using i16x16 = _x_<int16_t, 16>;
	using u16x16 = _x_<uint16_t, 16>;
	using i32x8 = _x_<int32_t, 8>;
	using u32x8 = _x_<uint32_t, 8>;
	using i64x4 = _x_<int64_t, 4>;
	using u64x4 = _x_<uint64_t, 4>;
	using f32x8 = _x_<float, 8>;
	using f64x4 = _x_<double, 4>;

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
