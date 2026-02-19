#pragma once
#include "color.hpp"
#include "image.hpp"
#include "math/fixed.hpp"

namespace rast {
	template <typename ColorT = color::rgba8>
	struct sampler {
		using size_type = uint32_t;
		using color = ColorT;

	private:
		const color* data;
		size_type width;
		size_type height;

	public:
		inline constexpr sampler() : data(nullptr), width(0), height(0) {}
		inline constexpr sampler(const color* Data, size_type Width, size_type Height) :
			data(Data), width(Width), height(Height) { }
		inline constexpr sampler(const image<color>& img) : sampler(img.data(), img.width(), img.height()) {}

		inline constexpr color sample(size_type x, size_type y) const {
			return data[y * width + x];
		}
		template <auto Converter>
		inline constexpr auto sample(size_type x, size_type y) const {
			return Converter(sample(x, y));
		}
		template <typename Callable>
		inline constexpr auto sample(size_type x, size_type y, Callable converter) const {
			return converter(sample(x, y));
		}

		inline constexpr color sample_nearest(float u, float v) const {
			size_type x = math::floor<size_type>(u * width) % width;
			size_type y = math::floor<size_type>(v * height) % height;
			return sample(x, y);
		}
		template <auto Converter>
		inline constexpr auto sample_nearest(float u, float v) const {
			return Converter(sample_nearest(u, v));
		}

		inline constexpr auto sample_linear(uint32_t u, uint32_t v) const {
			using arm = math::fixed_point_arithmetic<uint32_t, 16>;
			u *= width;
			v *= height;
			uint32_t coefs[2] = {
				u - arm::half - arm::floor(u - arm::half),
				v - arm::half - arm::floor(v - arm::half),
			};
			size_type x[2] = {
				static_cast<size_type>(arm::round(u)) % width,
				static_cast<size_type>(arm::round(u - arm::one)) % width
			};
			size_type y[2] = {
				arm::from_fixed<size_type>(arm::round(v)) % height,
				arm::from_fixed<size_type>(arm::round(v - arm::one)) % height
			};
			return (
				(sample(x[0], y[0]) * arm::multiply(coefs[0], coefs[1])) +
				(sample(x[1], y[0]) * arm::multiply((1.0f - coefs[0]), coefs[1])) +
				(sample(x[0], y[1]) * arm::multiply(coefs[0], (1.0f - coefs[1]))) +
				(sample(x[1], y[1]) * arm::multiply((1.0f - coefs[0]), (1.0f - coefs[1])))
			) / arm::one;
		}

		template <auto Interpolate>
		inline constexpr auto sample_linear(float u, float v) const {
			u *= width;
			v *= height;
			float coefs[2] = {
				u - 0.5f - math::floor<float>(u - 0.5f),
				v - 0.5f - math::floor<float>(v - 0.5f),
			};
			size_type x[2] = {
				math::round<size_type>(u) % width,
				math::round<size_type>(u - 1.0f) % width
			};
			size_type y[2] = {
				math::round<size_type>(v) % height,
				math::round<size_type>(v - 1.0f) % height
			};
			return Interpolate(
				sample(x[0], y[0]),
				sample(x[1], y[0]),
				sample(x[0], y[1]),
				sample(x[1], y[1]),
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
		inline constexpr auto sample_linear(float u, float v) const {
			return sample_linear<default_interpolator>(u, v);
		}

		inline explicit operator bool() const { return data != nullptr; }
	};
	namespace testing::sampler {
		constexpr float data[4] = { 0.0f, 1.0f, 2.0f, 3.0f };
		inline constexpr bool test() {
			constexpr auto smp = rast::sampler<float>(data, 2, 2);
			static_assert(smp.sample(0, 0) == 0.0f);
			static_assert(smp.sample(1, 0) == 1.0f);
			static_assert(smp.sample(0, 1) == 2.0f);
			static_assert(smp.sample(1, 1) == 3.0f);
			static_assert(smp.sample_nearest(0.25f, 0.25f) == 0.0f);
			static_assert(smp.sample_linear(0.5f, 0.5f) == 1.5f);
			return true;
		}
		static_assert(test());
	}
}
