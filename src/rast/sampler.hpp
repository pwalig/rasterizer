#pragma once
#include <array>
#include "color.hpp"
#include "image.hpp"
#include "math/fixed.hpp"
#include "math/vec.hpp"

namespace rast {
	enum struct min_filter {
		nearest, linear,
		nearest_mipmap_nearest,
		nearest_mipmap_linear,
		linear_mipmap_nearest,
		linear_mipmap_linear,
		bilinear = linear,
		trilinear = linear_mipmap_linear
	};
	enum struct mag_filter {
		nearest, linear, bilinear = linear
	};
	namespace wrapping {
		using size_type = uint32_t;

		enum struct mode {
			clamp, repeat
		};

		template <typename T, typename U>
		inline constexpr static U repeat(T pos, U limit) {
			return static_cast<U>((pos % static_cast<T>(limit)) + static_cast<T>(limit)) % limit;
		}
		template <typename T>
		inline constexpr static size_type clamp(T pos, size_type limit) {
			return std::clamp<T>(pos, 0, limit);
		}
		template <typename T, size_t Count>
		inline constexpr static math::u32x<Count> clamp(
			math::_scalar<T, Count> pos, math::u32x<Count> limit
		) {
			return math::cast<size_type>(
				math::clamp(pos,
				math::_scalar<T, Count>(static_cast<T>(0)),
				math::cast<T>(limit)));
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
	struct sampler {
		using size_type = uint32_t;
		using color = ColorT;
		using value_type = ColorT;
		using reference = std::add_lvalue_reference_t<value_type>;
		using pointer = std::add_pointer_t<value_type>;
		using const_reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;
		using const_pointer = std::add_pointer_t<std::add_const_t<value_type>>;

	private:
		const color* data;
		size_type width;
		size_type height;

		inline constexpr size_type valid_mip(size_type mip) const {
			return mipmapped_image<color>::valid_mip_level(width, height, mip);
		}
		inline constexpr size_type mip_length(size_type Length, size_type mip) const {
			return mipmapped_image<color>::length_at_mip_level(Length, mip);
		}
		template <size_t Count>
		inline constexpr math::u32x<Count> mip_length(
			math::u32x<Count> Length,
			math::u32x<Count> mip
		) const {
			for (size_t i = 0; i < Count; ++i) Length[i] >>= mip[i];
			return Length;
		}

	public:
		inline constexpr sampler(const color* Data, size_type Width, size_type Height) noexcept :
			data(Data), width(Width), height(Height) { }

		inline constexpr sampler() noexcept : sampler(nullptr, 0, 0) {}

		template <typename ImageLike>
		inline constexpr sampler(const ImageLike& img) :
			sampler(img.data(), img.width(), img.height()) {}

		inline constexpr const_reference sample(size_type x, size_type y, size_type mip = 0) const {
			size_type off = mipmapped_image<color>::mip_offset(width, height, mip);
			return (data + off)[y * mip_length(width, mip) + x];
		}

		template <size_t Count>
		inline constexpr math::_scalar<value_type, Count> sample(
			math::u32x<Count> x, math::u32x<Count> y,
			math::u32x<Count> mip = math::u32x<Count>(static_cast<uint32_t>(0))
		) const {
			auto res = math::_scalar<value_type, Count>();
			for (size_t i = 0; i < Count; ++i) res[i] = sample(x[i], y[i], mip[i]);
			return res;
		}

		template <typename T, size_t Dim, size_t Count>
		inline constexpr math::_vec<T, Dim, Count> transpose_sample(
			math::u32x<Count> x, math::u32x<Count> y,
			math::u32x<Count> mip = math::u32x<Count>(static_cast<uint32_t>(0))
		) const {
			auto res = math::_vec<T, Dim, Count>();
			for (size_t i = 0; i < Count; ++i) {
				auto sampled = sample(x[i], y[i], mip[i]);
#if _MSC_VER && !__INTEL_COMPILER
#pragma warning( push )
#pragma warning( disable : 4267 )
#endif
				for (size_t j = 0; j < Dim; ++j) res[j][i] = static_cast<T>(sampled[j]);
#if _MSC_VER && !__INTEL_COMPILER
#pragma warning( pop )
#endif
			}
			return res;
		}

		template <wrapping::mode Mode = wrapping::mode::repeat>
		inline constexpr const_reference sample_nearest(float u, float v, size_type mip = 0) const {
			mip = valid_mip(mip);
			size_type w = mip_length(width, mip);
			size_type h = mip_length(height, mip);
			size_type x = wrapping::wrap<Mode>(math::floor<int32_t>(u * w), w);
			size_type y = wrapping::wrap<Mode>(math::floor<int32_t>(v * h), h);
			return sample(x, y, mip);
		}

		template <wrapping::mode Mode = wrapping::mode::repeat, size_t Count>
		inline constexpr math::_scalar<value_type, Count> sample_nearest_x(
			math::f32x<Count> u, math::f32x<Count> v,
			math::u32x<Count> mip = math::u32x<Count>(static_cast<uint32_t>(0))
		) const {
			math::u32x<Count> w = mip_length(math::u32x<Count>(width), mip);
			math::u32x<Count> h = mip_length(math::u32x<Count>(height), mip);
			math::u32x<Count> x = wrapping::wrap<Mode>(math::floor<size_type>(u * math::cast<float>(w)), w);
			math::u32x<Count> y = wrapping::wrap<Mode>(math::floor<size_type>(v * math::cast<float>(h)), h);
			return sample(x, y, mip);
		}

		template <auto Interpolate>
		inline constexpr auto sample_linear(float u, float v, size_type mip = 0) const {
			mip = valid_mip(mip);
			size_type w = mip_length(width, mip);
			size_type h = mip_length(height, mip);
			u *= w;
			v *= h;
			float coefs[2] = {
				u - 0.5f - math::floor<float>(u - 0.5f),
				v - 0.5f - math::floor<float>(v - 0.5f),
			};
			size_type x[2] = {
				math::round<size_type>(u) % w,
				math::round<size_type>(u - 1.0f) % w
			};
			size_type y[2] = {
				math::round<size_type>(v) % h,
				math::round<size_type>(v - 1.0f) % h
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

		//template <auto Interpolate, size_t Count>
		//inline constexpr auto sample_linear_x(
		//	math::f32x<Count> u, math::f32x<Count> v,
		//	math::u32x<Count> mip = math::u32x<Count>(static_cast<uint32_t>(0))
		//) const {
		//	math::u32x<Count> w = mip_length(math::u32x<Count>(width), mip);
		//	math::u32x<Count> h = mip_length(math::u32x<Count>(height), mip);
		//	u *= math::cast<float>(w);
		//	v *= math::cast<float>(h);
		//	auto coefs = math::f32vec2x4(
		//		u - math::f32x4(0.5f) - math::floor<float>(u - math::f32x4(0.5f)),
		//		v - math::f32x4(0.5f) - math::floor<float>(v - math::f32x4(0.5f))
		//	);
		//	auto x = math::u32vec2x<Count>(
		//		math::round<size_type>(u) % w,
		//		math::round<size_type>(u) % w,
		//	);
		//	math::u32vec2x<Count> y = math::round<size_type>(v) % h;
		//}

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

		template <auto (sampler::*Sample)(float, float, size_type) const>
		inline constexpr auto sample_nearest_mipmap(math::f32x4 u, math::f32x4 v) const {
			math::f32x4 x = u * math::f32x4(static_cast<float>(width));
			math::f32x4 y = v * math::f32x4(static_cast<float>(height));

			float x_up = math::abs(x[0] - x[1]);
			float x_down = math::abs(x[2] - x[3]);
			float y_left = math::abs(y[0] - y[2]);
			float y_right = math::abs(y[1] - y[3]);

			auto _get_mip_level = [](float dx, float dy) {
				size_type delta = math::floor<size_type>((dx + dy) / 2.0f);
				if (delta == 0) return size_type(0);
				--delta;
				size_type mip = 0;
				while (delta > 0) {
					delta /= 2;
					mip += 1;
				}
				return mip;
				};
			auto mip_levels = math::make_x4<uint32_t>(
				_get_mip_level(x_up, y_left), _get_mip_level(x_up, y_right),
				_get_mip_level(x_down, y_left), _get_mip_level(x_down, y_right)
			);
			auto res = math::x4<value_type>();
			res[0] = (this->*Sample)(u[0], v[0], mip_levels[0]);
			res[1] = (this->*Sample)(u[1], v[1], mip_levels[1]);
			res[2] = (this->*Sample)(u[2], v[2], mip_levels[2]);
			res[3] = (this->*Sample)(u[3], v[3], mip_levels[3]);
			return res;
		}
		template <wrapping::mode Mode = wrapping::mode::repeat>
		inline constexpr auto sample_nearest_mipmap_nearest(math::f32x4 u, math::f32x4 v) const {
			return sample_nearest_mipmap<&rast::sampler<color>::sample_nearest<Mode>>(u, v);
		}
		inline constexpr auto sample_nearest_mipmap_linear(math::f32x4 u, math::f32x4 v) const {
			return sample_nearest_mipmap<&rast::sampler<color>::sample_linear<default_interpolator>>(u, v);
		}

		inline constexpr explicit operator bool() const { return data != nullptr; }
	};
	namespace sampler_test {
		constexpr float data[4] = { 0.0f, 1.0f, 2.0f, 3.0f };
		constexpr float mip_data[16 + 4 + 1] = {
			0.0f, 1.0f, 2.0f, 3.0f,
			4.0f, 5.0f, 6.0f, 7.0f,
			8.0f, 9.0f, 10.0f, 11.0f,
			12.0f, 13.0f, 14.0f, 15.0f,
			16.0f, 17.0f,
			18.0f, 19.0f,
			20.0f
		};

		constexpr auto smp = rast::sampler<float>(data, 2, 2);
		static_assert(smp.sample(0, 0) == 0.0f);
		static_assert(smp.sample(1, 0) == 1.0f);
		static_assert(smp.sample(0, 1) == 2.0f);
		static_assert(smp.sample(1, 1) == 3.0f);
		static_assert(smp.sample_nearest(0.25f, 0.25f) == 0.0f);
		static_assert(smp.sample_linear(0.5f, 0.5f) == 1.5f);

		constexpr auto sampled = smp.sample(
			math::make_x4<uint32_t>(0, 1, 0, 1),
			math::make_x4<uint32_t>(0, 0, 1, 1)
		);
		static_assert(sampled[0] == 0.0f);
		static_assert(sampled[1] == 1.0f);
		static_assert(sampled[2] == 2.0f);
		static_assert(sampled[3] == 3.0f);
		constexpr auto sampled2 = smp.sample_nearest_x(
			math::make_f32x4(0.25f, 0.5f, 0.25f, 0.5f),
			math::make_f32x4(0.25f, 0.25f, 0.5f, 0.5f)
		);
		static_assert(sampled2[0] == 0.0f);
		static_assert(sampled2[1] == 1.0f);
		static_assert(sampled2[2] == 2.0f);
		static_assert(sampled2[3] == 3.0f);

		// mip map test
		constexpr auto mipsmp = rast::sampler<float>(mip_data, 4, 4);
		constexpr auto s0 = mipsmp.sample_nearest_mipmap_nearest(
			math::make_f32x4(0.125f, 0.375f, 0.125f, 0.375f),
			math::make_f32x4(0.125f, 0.125f, 0.375f, 0.375f)
		);
		static_assert(s0[0] == 0.0f);
		static_assert(s0[1] == 1.0f);
		static_assert(s0[2] == 4.0f);
		static_assert(s0[3] == 5.0f);

		constexpr auto s1 = mipsmp.sample_nearest_mipmap_nearest(
			math::make_f32x4(0.25f, 0.75f, 0.25f, 0.75f),
			math::make_f32x4(0.25f, 0.25f, 0.75f, 0.75f)
		);
		static_assert(s1[0] == 16.0f);
		static_assert(s1[1] == 17.0f);
		static_assert(s1[2] == 18.0f);
		static_assert(s1[3] == 19.0f);

		constexpr auto s2 = mipsmp.sample_nearest_mipmap_linear(
			math::make_f32x4(0.25f, 0.5f, 0.25f, 0.5f),
			math::make_f32x4(0.25f, 0.25f, 0.5f, 0.5f)
		);
		static_assert(s2[0] == 2.5f);
		static_assert(s2[1] == 3.5f);
		static_assert(s2[2] == 6.5f);
		static_assert(s2[3] == 7.5f);

		constexpr auto s3 = mipsmp.sample_nearest_mipmap_linear(
			math::make_f32x4(0.5f, 1.0f, 0.5f, 1.0f),
			math::make_f32x4(0.5f, 0.5f, 1.0f, 1.0f)
		);
		static_assert(s3[0] == 17.5f);
	}
}
