#pragma once
#include <array>
#include "color.hpp"
#include "image.hpp"
#include "math/fixed.hpp"

namespace rast {
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

		inline constexpr size_type mip_length(size_type Length, size_type mip) const {
			return mipmapped_image<color>::length_at_valid_mip_level(Length, width, height, mip);
		}

	public:
		inline constexpr sampler(const color* Data, size_type Width, size_type Height) noexcept :
			data(Data), width(Width), height(Height) { }
		inline constexpr sampler() noexcept : sampler(nullptr, 0, 0) {}
		template <typename ImageLike>
		inline constexpr sampler(const ImageLike& img) : sampler(img.data(), img.width(), img.height()) {}

		inline constexpr const_reference sample(size_type x, size_type y, size_type mip = 0) const {
			size_type off = mipmapped_image<color>::mip_offset(width, height, mip);
			return (data + off)[y * mip_length(width, mip) + x];
		}
		template <auto Converter>
		inline constexpr auto sample(size_type x, size_type y, size_type mip = 0) const {
			return Converter(sample(x, y));
		}
		template <typename Callable>
		inline constexpr auto sample(size_type x, size_type y, Callable converter, size_type mip = 0) const {
			return converter(sample(x, y));
		}

		inline constexpr const_reference sample_nearest(float u, float v, size_type mip = 0) const {
			size_type w = mip_length(width, mip);
			size_type h = mip_length(height, mip);
			size_type x = math::floor<size_type>(u * w) % w;
			size_type y = math::floor<size_type>(v * h) % h;
			return sample(x, y, mip);
		}
		template <auto Converter>
		inline constexpr auto sample_nearest(float u, float v, size_type mip = 0) const {
			return Converter(sample_nearest(u, v, mip));
		}

		inline constexpr void sample_nearest_mipmap_nearest(
			const float u[4], const float v[4], color dst[4]
		) const {
			float x[4] = {
				u[0] * width, u[1] * width,
				u[2] * width, u[3] * width
			};
			float y[4] = {
				v[0] * height, v[1] * height,
				v[2] * height, v[3] * height
			};
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
			size_type mip_levels[4] = {
				_get_mip_level(x_up, y_left), _get_mip_level(x_up, y_right),
				_get_mip_level(x_down, y_left), _get_mip_level(x_down, y_right)
			};
			dst[0] = sample_nearest(u[0], v[0], mip_levels[0]);
			dst[1] = sample_nearest(u[1], v[1], mip_levels[1]);
			dst[2] = sample_nearest(u[2], v[2], mip_levels[2]);
			dst[3] = sample_nearest(u[3], v[3], mip_levels[3]);
		}
		inline constexpr std::array<value_type, 4> sample_nearest_mipmap_nearest(
			const float u[4], const float v[4]
		) const {
			auto res = std::array<value_type, 4>();
			sample_nearest_mipmap_nearest(u, v, res.data());
			return res;
		}

		//inline constexpr auto sample_linear(uint32_t u, uint32_t v) const {
		//	using arm = math::fixed_point_arithmetic<uint32_t, 16>;
		//	u *= width;
		//	v *= height;
		//	uint32_t coefs[2] = {
		//		u - arm::half - arm::floor(u - arm::half),
		//		v - arm::half - arm::floor(v - arm::half),
		//	};
		//	size_type x[2] = {
		//		static_cast<size_type>(arm::round(u)) % width,
		//		static_cast<size_type>(arm::round(u - arm::one)) % width
		//	};
		//	size_type y[2] = {
		//		arm::from_fixed<size_type>(arm::round(v)) % height,
		//		arm::from_fixed<size_type>(arm::round(v - arm::one)) % height
		//	};
		//	return (
		//		(sample(x[0], y[0]) * arm::multiply(coefs[0], coefs[1])) +
		//		(sample(x[1], y[0]) * arm::multiply((1.0f - coefs[0]), coefs[1])) +
		//		(sample(x[0], y[1]) * arm::multiply(coefs[0], (1.0f - coefs[1]))) +
		//		(sample(x[1], y[1]) * arm::multiply((1.0f - coefs[0]), (1.0f - coefs[1])))
		//	) / arm::one;
		//}

		template <auto Interpolate>
		inline constexpr auto sample_linear(float u, float v, size_type mip = 0) const {
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

		inline static constexpr auto default_interpolator(
				const color& color0, const color& color1, const color& color2, const color& color3,
				float coef0, float coef1, float coef2, float coef3
			) {
				return (color0 * coef0) + (color1 * coef1) + (color2 * coef2) + (color3 * coef3);
			};
		inline constexpr auto sample_linear(float u, float v, size_type mip = 0) const {
			return sample_linear<default_interpolator>(u, v, mip);
		}

		inline explicit operator bool() const { return data != nullptr; }
	};
	namespace testing::sampler {
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
		inline constexpr bool test() {
			constexpr auto smp = rast::sampler<float>(data, 2, 2);
			static_assert(smp.sample(0, 0) == 0.0f);
			static_assert(smp.sample(1, 0) == 1.0f);
			static_assert(smp.sample(0, 1) == 2.0f);
			static_assert(smp.sample(1, 1) == 3.0f);
			static_assert(smp.sample_nearest(0.25f, 0.25f) == 0.0f);
			static_assert(smp.sample_linear(0.5f, 0.5f) == 1.5f);

			// mip map test
			constexpr auto mipsmp = rast::sampler<float>(mip_data, 4, 4);
			constexpr float u0[4] = { 0.125f, 0.375f, 0.125f, 0.375f };
			constexpr float v0[4] = { 0.125f, 0.125f, 0.375f, 0.375f };
			constexpr auto s0 = mipsmp.sample_nearest_mipmap_nearest(u0, v0);
			static_assert(s0[0] == 0.0f);
			static_assert(s0[1] == 1.0f);
			static_assert(s0[2] == 4.0f);
			static_assert(s0[3] == 5.0f);

			constexpr float u1[4] = { 0.25f, 0.75f, 0.25f, 0.75f };
			constexpr float v1[4] = { 0.25f, 0.25f, 0.75f, 0.75f };
			constexpr auto s1 = mipsmp.sample_nearest_mipmap_nearest(u1, v1);
			static_assert(s1[0] == 16.0f);
			static_assert(s1[1] == 17.0f);
			static_assert(s1[2] == 18.0f);
			static_assert(s1[3] == 19.0f);
			return true;
		}
		static_assert(test());
	}
}
