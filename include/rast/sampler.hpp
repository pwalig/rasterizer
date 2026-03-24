#pragma once
#include <array>
#include "color.hpp"
#include "math.hpp"
#include "image.hpp"
#include "simd.hpp"
#include "simd/glm.hpp"
#include "sized2d_base.hpp"
#include <algorithm>

namespace rast {
	enum struct min_filter : uint8_t {
		nearest = 0, linear = 1,
		nearest_mipmap_nearest = 0b0010,
		nearest_mipmap_linear = 0b0011,
		linear_mipmap_nearest = 0b0110,
		linear_mipmap_linear = 0b0111,
		bilinear = linear,
		trilinear = linear_mipmap_linear
	};
	enum struct texel_filter : uint8_t {
		nearest = 0, linear = 1,
		linear_bit = 1
	};
	enum struct mipmap_filter : uint8_t {
		none = 0, nearest = 2, linear = 6,
		use_mip_bit = 2, linear_bit = 4
	};
	template <texel_filter Texel, mipmap_filter MipMap>
	inline constexpr min_filter min_filter_v = static_cast<min_filter>(
		static_cast<uint8_t>(Texel) | static_cast<uint8_t>(MipMap));

	enum struct mag_filter {
		nearest, linear, bilinear = linear
	};
	namespace wrapping {
		using size_type = uint32_t;

		enum struct mode {
			clamp, repeat
		};

		template <typename T, typename U>
		inline constexpr U repeat(T pos, U limit) {
			return static_cast<U>((pos % static_cast<T>(limit)) + static_cast<T>(limit)) % limit;
		}
		template <size_t Count>
		inline simd::u32x_<Count> repeat(
			simd::f32x_<Count> pos,
			simd::u32x_<Count> limit
		) {
			simd::u32x_<Count> x = simd::mod<int32_t, Count>(simd::cvt<int32_t>(pos), limit);
			return simd::u32x_<Count>(simd::mod<int32_t, Count>(x + limit, limit));
		}
		template <typename T>
		inline constexpr static size_type clamp(T pos, size_type limit) {
			return std::clamp<T>(pos, 0, limit);
		}
		template <typename T, size_t Count>
		inline simd::_x_<size_type, Count> clamp(
			simd::_x_<T, Count> pos,
			simd::u32x_<Count> limit
		) {
			return simd::cvt<size_type>(
				simd::clamp(
					pos,
					simd::setzero<T, Count>(0),
					simd::cvt<T>(limit)
				)
			);
		}
		template <mode Mode, typename T, typename U>
		inline constexpr static U wrap(T pos, U limit) {
			if constexpr (Mode == mode::repeat) return repeat(pos, limit);
			else if constexpr (Mode == mode::clamp) return clamp(pos, limit);
		}

		static_assert(repeat<int>(-1, 10) == 9);
		static_assert(repeat<int>(-102, 10) == 8);
		static_assert(repeat<int>(11, 10) == 1);
		static_assert(repeat<int>(102, 10) == 2);
		static_assert(clamp<int>(-1, 10) == 0);
		static_assert(clamp<int>(11, 10) == 10);
	}
	template <typename ColorT = color::rgba8>
	struct sampler : public sized2d_base {
		using size_type = uint32_t;
		using color = ColorT;
		using value_type = ColorT;
		using reference = std::add_lvalue_reference_t<value_type>;
		using pointer = std::add_pointer_t<value_type>;
		using const_reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;
		using const_pointer = std::add_pointer_t<std::add_const_t<value_type>>;

	private:
		const color* data;

		inline constexpr size_type valid_mip(size_type mip) const {
			return mipmapped_image<color>::valid_mip_level(_width, _height, mip);
		}
		inline constexpr size_type mip_length(size_type Length, size_type mip) const {
			return mipmapped_image<color>::length_at_mip_level(Length, mip);
		}
		template <typename T>
		inline constexpr T mip_length(T Length, T mip) const {
			return Length >> mip;
		}

	public:
		inline constexpr sampler(const color* Data, size_type Width, size_type Height) noexcept :
			sized2d_base(Width, Height), data(Data) { }

		inline constexpr sampler() noexcept : sampler(nullptr, 0, 0) {}

		template <typename ImageLike>
		inline constexpr sampler(const ImageLike& img) :
			sampler(img.data(), img.width(), img.height()) {}

		inline constexpr const_reference sample(size_type x, size_type y, size_type mip) const {
			size_type off = mipmapped_image<color>::mip_offset(_width, _height, mip);
			return (data + off)[y * mip_length(_width, mip) + x];
		}

		inline constexpr const_reference sample(size_type x, size_type y) const {
			return data[data_offset(x, y)];
		}

		template <size_t Count>
		inline static auto default_vectorizer(const_pointer dptr, simd::u32x_<Count> off) {
			static_assert(!std::is_class_v<value_type>);
			static_assert((Count == 2) || (Count == 4) || (Count == 8));
			if constexpr (Count == 2) return simd::make_x2(
				dptr[off[1]], dptr[off[0]]
			);
			if constexpr (Count == 4) return simd::make_x4(
				dptr[off[3]], dptr[off[2]], dptr[off[1]], dptr[off[0]]
			);
			if constexpr (Count == 8) return simd::make_x8(
				dptr[off[7]], dptr[off[6]], dptr[off[5]], dptr[off[4]],
				dptr[off[3]], dptr[off[2]], dptr[off[1]], dptr[off[0]]
			);
		}

		template <auto Vectorizer, size_t Count>
		inline auto sample(
			simd::u32x_<Count> x,
			simd::u32x_<Count> y,
			simd::u32x_<Count> mip = simd::setzero<size_type, Count>()
		) const {
			auto w = simd::u32x_<Count>(_width);
			auto h = simd::u32x_<Count>(_height);
			auto off = mipmapped_image<color>::mip_offset(w, h, mip);
			w >>= mip;
			return Vectorizer(data, y * w + x + off);
		}
		template <auto Vectorizer, size_t Count>
		inline auto sample(
			simd::u32x_<Count> x,
			simd::u32x_<Count> y
		) const {
			return Vectorizer(data, y * simd::u32x_<Count>(_width) + x);
		}

		template <wrapping::mode Mode = wrapping::mode::repeat>
		inline constexpr const_reference sample_nearest(float u, float v, size_type mip = 0) const {
			mip = valid_mip(mip);
			size_type w = mip_length(_width, mip);
			size_type h = mip_length(_height, mip);
			size_type x = wrapping::wrap<Mode>(math::floor<int32_t>(u * w), w);
			size_type y = wrapping::wrap<Mode>(math::floor<int32_t>(v * h), h);
			return sample(x, y, mip);
		}
		template <auto Vectorizer, wrapping::mode Mode = wrapping::mode::repeat, size_t Count>
		inline auto sample_nearest(
			simd::f32x_<Count> u,
			simd::f32x_<Count> v,
			simd::u32x_<Count> mip = simd::setzero<uint32_t, Count>()
		) const {
			auto w = simd::u32x_<Count>(_width);
			auto h = simd::u32x_<Count>(_height);
			mip = mipmapped_image<color>::valid_mip_level(w, h, mip);
			w >>= mip;
			h >>= mip;
			simd::u32x_<Count> x = wrapping::wrap<Mode>(
				simd::floor(u * simd::cvt<float>(w)), w
			);
			simd::u32x_<Count> y = wrapping::wrap<Mode>(
				simd::floor(v * simd::cvt<float>(h)), h
			);
			return sample<Vectorizer>(x, y, mip);
		}
		template <wrapping::mode Mode = wrapping::mode::repeat, size_t Count>
		inline auto sample_nearest(
			simd::f32x_<Count> u,
			simd::f32x_<Count> v,
			simd::u32x_<Count> mip = simd::setzero<uint32_t, Count>()
		) {
			return sample_nearest<default_vectorizer<Count>, Mode>(u, v, mip);
		}

		template <auto Interpolate, wrapping::mode Mode = wrapping::mode::repeat>
		inline constexpr auto sample_linear(float u, float v, size_type mip = 0) const {
			mip = valid_mip(mip);
			size_type w = mip_length(_width, mip);
			size_type h = mip_length(_height, mip);
			u *= w;
			v *= h;
			float coefs[2] = {
				u - 0.5f - math::floor<float>(u - 0.5f),
				v - 0.5f - math::floor<float>(v - 0.5f),
			};
			size_type x[2] = {
				wrapping::wrap<Mode>(math::round<size_type>(u), w),
				wrapping::wrap<Mode>(math::round<size_type>(u - 1.0f), w)
			};
			size_type y[2] = {
				wrapping::wrap<Mode>(math::round<size_type>(v), h),
				wrapping::wrap<Mode>(math::round<size_type>(v - 1.0f), h)
			};
			return Interpolate(
				sample(x[0], y[0], mip),
				sample(x[1], y[0], mip),
				sample(x[0], y[1], mip),
				sample(x[1], y[1], mip),
				coefs[0] * coefs[1],
				(1.0f - coefs[0]) * coefs[1],
				coefs[0] * (1.0f - coefs[1]),
				(1.0f - coefs[0]) * (1.0f - coefs[1])
			);
		}
		template <auto Vectorizer, wrapping::mode Mode = wrapping::mode::repeat, size_t Count>
		inline auto sample_linear(
			simd::f32x_<Count> u,
			simd::f32x_<Count> v,
			simd::u32x_<Count> mip = simd::setzero<uint32_t, Count>()
		) const {
			auto w = simd::u32x_<Count>(_width);
			auto h = simd::u32x_<Count>(_height);
			mip = mipmapped_image<color>::valid_mip_level(w, h, mip);
			w >>= mip;
			h >>= mip;
			u *= simd::cvt<float>(w);
			v *= simd::cvt<float>(h);
			auto one = simd::f32x_<Count>(1.0f);
			auto half = simd::f32x_<Count>(0.5f);
			simd::f32x_<Count> coefs[2] = {
				u - half - simd::floor(u - half),
				v - half - simd::floor(v - half),
			};
			simd::u32x_<Count> x[2] = {
				wrapping::wrap<Mode>(simd::round(u), w),
				wrapping::wrap<Mode>(simd::round(u - one), w)
			};
			simd::u32x_<Count> y[2] = {
				wrapping::wrap<Mode>(simd::round(v), h),
				wrapping::wrap<Mode>(simd::round(v - one), h)
			};
			simd::f32x_<Count> one_sub_coefs[2] = {
				one - coefs[0],
				one - coefs[1]
			};
			return
				(sample<Vectorizer>(x[0], y[0], mip) * coefs[0] * coefs[1]) +
				(sample<Vectorizer>(x[1], y[0], mip) * one_sub_coefs[0] * coefs[1]) +
				(sample<Vectorizer>(x[0], y[1], mip) * coefs[0] * one_sub_coefs[1]) +
				(sample<Vectorizer>(x[1], y[1], mip) * one_sub_coefs[0] * one_sub_coefs[1]);
		}

		inline static constexpr auto default_interpolator(
			const color& color0, const color& color1, const color& color2, const color& color3,
			float coef0, float coef1, float coef2, float coef3
		) {
			return (color0 * coef0) + (color1 * coef1) + (color2 * coef2) + (color3 * coef3);
		};
		template <auto MultiplyByFloat>
		inline static constexpr auto interpolate(
			const color& color0, const color& color1, const color& color2, const color& color3,
			float coef0, float coef1, float coef2, float coef3
		) {
			return
				(MultiplyByFloat(color0, coef0)) +
				(MultiplyByFloat(color1, coef1)) +
				(MultiplyByFloat(color2, coef2)) +
				(MultiplyByFloat(color3, coef3));
		}
		inline constexpr auto sample_linear(float u, float v, size_type mip = 0) const {
			return sample_linear<default_interpolator>(u, v, mip);
		}
		
		template <auto (sampler::*MagSample)(simd::f32x4, simd::f32x4, simd::u32x4) const,
			auto (sampler::*MipSample)(simd::f32x4, simd::f32x4, simd::f32x4) const>
		inline auto sample_min_mag_dispatch(simd::f32x4 u, simd::f32x4 v) const {
			simd::f32x4 x = u * simd::f32x4(static_cast<float>(_width));
			simd::f32x4 y = v * simd::f32x4(static_cast<float>(_height));

			// shuffles explained:
			//    --- fp3 - fp2 - fp1 - fp0
			//  x,y   0,0   0,1   1,0   1,1  order:   3, 2, 1, 0    // this is set in rasterizer (see: vbbox_scan::pixel_pattern)
			// x2,y2  1,0   0,0   1,1   0,1  shuffle: 1, 3, 0, 2    // neighbouring pixel
			// dx,dy  3-1   2-3   1-0   0-2  order:   3, 2, 1, 0    // delta between neighbours
			// mip+=  2-3   0-2   3-1   1-0  shuffle: 2, 0, 3, 1    // average out with second neighbour
			simd::f32x4 x2 = _mm_shuffle_ps(x, x, _MM_SHUFFLE(1, 3, 0, 2));
			simd::f32x4 y2 = _mm_shuffle_ps(y, y, _MM_SHUFFLE(1, 3, 0, 2));
			simd::f32x4 deltas = simd::glm::length<2, 4>(simd::glm::vec2<4>(x, y) - simd::glm::vec2<4>(x2, y2));
			simd::f32x4 one = simd::f32x4(1.0f);
			if (simd::movemask(deltas > one)) {
				return (this->*MipSample)(u, v, deltas);
			} else return (this->*MagSample)(u, v, simd::setzero<uint32_t, 4>());
		}

		template <auto (sampler::*Sample)(simd::f32x4, simd::f32x4, simd::u32x4) const>
		inline auto sample_no_mipmap(simd::f32x4 u, simd::f32x4 v, simd::f32x4) const {
			return (this->*Sample)(u, v, simd::setzero<uint32_t, 4>());
		}
		template <auto (sampler::*Sample)(simd::f32x4, simd::f32x4, simd::u32x4) const>
		inline auto sample_nearest_mipmap(simd::f32x4 u, simd::f32x4 v, simd::f32x4 deltas) const {
			simd::i32x4 zero = simd::setzero<int, 4>();
			simd::i32x4 delta = simd::cvt<int>(deltas) >> 1; // floor(delta) / 2; Delta is always positive, so cast works as floor. Delta = 1 coresponds to mip0, bit shit so that delta = 0 -> mip0.
			simd::i32x4 mip_levels = zero;
			simd::i32x4 mask = delta > zero;
			while (simd::movemask(mask)) {
				delta >>= 1;
				mip_levels += (zero - mask);
				mask = delta > zero;
			}
			mip_levels += _mm_shuffle_epi32(mip_levels, _MM_SHUFFLE(2, 0, 3, 1));
			mip_levels >>= 1;
			return (this->*Sample)(u, v, simd::cvt<uint32_t>(mip_levels));
		}

		template <auto (sampler::*Sample)(simd::f32x4, simd::f32x4, simd::u32x4) const>
		inline auto sample_linear_mipmap(simd::f32x4 u, simd::f32x4 v, simd::f32x4 deltas) const {
			simd::f32x4 one = simd::f32x4(1.0f);
			deltas = simd::max(deltas, one);
			alignas(16) float deltas_mem[4];
			simd::store(deltas_mem, deltas);
			deltas_mem[0] = std::log2(deltas_mem[0]); // sadly no log simd instruction
			deltas_mem[1] = std::log2(deltas_mem[1]);
			deltas_mem[2] = std::log2(deltas_mem[2]);
			deltas_mem[3] = std::log2(deltas_mem[3]);
			deltas = simd::load<float, 4>(deltas_mem);
			simd::f32x4 fmip_leves = simd::floor(deltas);
			simd::f32x4 coef = deltas - fmip_leves;
			simd::u32x4 mip_levels = simd::cvt<uint32_t>(fmip_leves);
			mip_levels += _mm_shuffle_epi32(mip_levels, _MM_SHUFFLE(2, 0, 3, 1));
			mip_levels >>= 1;
			return ((this->*Sample)(u, v, mip_levels + simd::i32x4(1)) * coef) +
				((this->*Sample)(u, v, mip_levels) * (one - coef));
		}
		template <auto Vectorizer, mag_filter MagFilter, min_filter MinFilter, wrapping::mode Mode = wrapping::mode::repeat>
		inline auto sample(simd::f32x4 u, simd::f32x4 v) const {
			if constexpr (MinFilter == min_filter::nearest) {
				if constexpr (MagFilter == mag_filter::nearest) return sample_nearest<Vectorizer, Mode, 4>(u, v);
				else return sample_min_mag_dispatch<
					&rast::sampler<color>::template sample_linear<Vectorizer, Mode, 4>,
					&rast::sampler<color>::template sample_no_mipmap<&rast::sampler<color>::template sample_nearest<Vectorizer, Mode, 4>>
				>(u, v);
			}
			else if constexpr (MinFilter == min_filter::linear) {
				if constexpr (MagFilter == mag_filter::linear) return sample_linear<Vectorizer, Mode, 4>(u, v);
				else return sample_min_mag_dispatch<
					&rast::sampler<color>::template sample_nearest<Vectorizer, Mode, 4>,
					&rast::sampler<color>::template sample_no_mipmap<&rast::sampler<color>::template sample_linear<Vectorizer, Mode, 4>>
				>(u, v);
			}
			else if constexpr (MinFilter == min_filter::nearest_mipmap_nearest) {
				if constexpr (MagFilter == mag_filter::nearest) return sample_min_mag_dispatch<
					&rast::sampler<color>::template sample_nearest<Vectorizer, Mode, 4>,
					&rast::sampler<color>::template sample_nearest_mipmap<&rast::sampler<color>::template sample_nearest<Vectorizer, Mode, 4>>
				>(u, v);
				else return sample_min_mag_dispatch<
					&rast::sampler<color>::template sample_linear<Vectorizer, Mode, 4>,
					&rast::sampler<color>::template sample_nearest_mipmap<&rast::sampler<color>::template sample_nearest<Vectorizer, Mode, 4>>
				>(u, v);
			}
			else if constexpr (MinFilter == min_filter::nearest_mipmap_linear) {
				if constexpr (MagFilter == mag_filter::nearest) return sample_min_mag_dispatch<
					&rast::sampler<color>::template sample_nearest<Vectorizer, Mode, 4>,
					&rast::sampler<color>::template sample_nearest_mipmap<&rast::sampler<color>::template sample_linear<Vectorizer, Mode, 4>>
				>(u, v);
				else return sample_min_mag_dispatch<
					&rast::sampler<color>::template sample_linear<Vectorizer, Mode, 4>,
					&rast::sampler<color>::template sample_nearest_mipmap<&rast::sampler<color>::template sample_linear<Vectorizer, Mode, 4>>
				>(u, v);
			}
			else if constexpr (MinFilter == min_filter::linear_mipmap_nearest) {
				if constexpr (MagFilter == mag_filter::nearest) return sample_min_mag_dispatch<
					&rast::sampler<color>::template sample_nearest<Vectorizer, Mode, 4>,
					&rast::sampler<color>::template sample_linear_mipmap<&rast::sampler<color>::template sample_nearest<Vectorizer, Mode, 4>>
				>(u, v);
				else return sample_min_mag_dispatch<
					&rast::sampler<color>::template sample_linear<Vectorizer, Mode, 4>,
					&rast::sampler<color>::template sample_linear_mipmap<&rast::sampler<color>::template sample_nearest<Vectorizer, Mode, 4>>
				>(u, v);
			}
			else if constexpr (MinFilter == min_filter::linear_mipmap_linear) {
				if constexpr (MagFilter == mag_filter::nearest) return sample_min_mag_dispatch<
					&rast::sampler<color>::template sample_nearest<Vectorizer, Mode, 4>,
					&rast::sampler<color>::template sample_linear_mipmap<&rast::sampler<color>::template sample_linear<Vectorizer, Mode, 4>>
				>(u, v);
				else return sample_min_mag_dispatch<
					&rast::sampler<color>::template sample_linear<Vectorizer, Mode, 4>,
					&rast::sampler<color>::template sample_linear_mipmap<&rast::sampler<color>::template sample_linear<Vectorizer, Mode, 4>>
				>(u, v);
			}
		}

		inline constexpr explicit operator bool() const { return data != nullptr; }
	};
}
