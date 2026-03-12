#pragma once
#include <array>
#include "color.hpp"
#include "image.hpp"
#include "simd.hpp"
#include "math/vec.hpp"
#include "sized2d_base.hpp"

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
		inline constexpr U repeat(T pos, U limit) {
			return static_cast<U>((pos % static_cast<T>(limit)) + static_cast<T>(limit)) % limit;
		}
		template <size_t Count>
		inline simd::u32x_<Count> repeat(
			simd::f32x_<Count> pos,
			simd::u32x_<Count> limit
		) {
			simd::u32x_<Count> x = simd::mod<int32_t, Count>(simd::cast<int32_t>(pos), limit);
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
			return simd::cast<size_type>(
				simd::clamp(
					pos,
					simd::_x_<T, Count>(0),
					simd::cast<T>(limit)
				)
			);
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

		inline constexpr const_reference sample(size_type x, size_type y, size_type mip = 0) const {
			size_type off = mipmapped_image<color>::mip_offset(_width, _height, mip);
			return (data + off)[y * mip_length(_width, mip) + x];
		}

		template <size_t Count>
		inline static std::array<value_type, Count> default_vectorizer(const_pointer ptr, simd::u32x_<Count> offsets) {
			std::array<value_type, Count> res;
#if _MSC_VER && !__INTEL_COMPILER
#pragma warning( push )
#pragma warning( disable : 4267 )
#endif
			for (size_t i = 0; i < Count; ++i) res[i] = ptr[offsets[i]];
#if _MSC_VER && !__INTEL_COMPILER
#pragma warning( pop )
#endif
			return res;
		}

		template <auto Vectorizer, size_t Count>
		inline auto sample(
			simd::u32x_<Count> x,
			simd::u32x_<Count> y,
			size_type mip = 0
		) const {
			size_type off = mipmapped_image<color>::mip_offset(_width, _height, mip);
			auto w = simd::u32x_<Count>(mip_length(_width, mip));
			auto offsets = (y * w) + x;
			const_pointer d = data + off;
			if constexpr (!std::is_class_v<value_type> && (Count == 4)) {
				return simd::make_x4(d[offsets[0]], d[offsets[1]], d[offsets[2]], d[offsets[3]]);
			}
			if constexpr (!std::is_class_v<value_type> && (Count == 8)) {
				return simd::make_x8(
					d[offsets[0]], d[offsets[1]], d[offsets[2]], d[offsets[3]],
					d[offsets[4]], d[offsets[5]], d[offsets[6]], d[offsets[7]]
				);
			}
			else {
				return Vectorizer(d, offsets);
			}
		}

		template <size_t Count>
		inline constexpr math::_scalar<const_pointer, Count> address(
			math::u32x<Count> x, math::u32x<Count> y,
			math::u32x<Count> mip = math::u32x<Count>(static_cast<uint32_t>(0))
		) const {
			auto off = math::u32x<Count>();
			for (size_t i = 0; i < Count; ++i)
				off[i] = mipmapped_image<color>::mip_offset(_width, _height, mip[i]);

			using u32ptr = math::_scalar<const_pointer, Count>;
			return u32ptr(data) + off + (y * mip_length(math::u32x<Count>(_width), mip) + x);
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

		// sample transpose
		template <typename T, size_t Dim, size_t Count>
		inline constexpr math::_vec<T, Dim, Count> sample_t(
			math::u32x<Count> x, math::u32x<Count> y,
			math::u32x<Count> mip = math::u32x<Count>(static_cast<uint32_t>(0))
		) const {
			auto addrs = address(x, y, mip);
			auto res = math::_vec<T, Dim, Count>();
			for (size_t i = 0; i < Count; ++i) {
#if _MSC_VER && !__INTEL_COMPILER
#pragma warning( push )
#pragma warning( disable : 4267 )
#endif
				for (size_t j = 0; j < Dim; ++j) res[j][i] = static_cast<T>((*addrs[i])[j]);
#if _MSC_VER && !__INTEL_COMPILER
#pragma warning( pop )
#endif
			}
			return res;
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
			size_type mip = 0
		) const {
			mip = valid_mip(mip);
			size_type w = mip_length(_width, mip);
			size_type h = mip_length(_height, mip);
			simd::u32x_<Count> x = wrapping::wrap<Mode>(
				simd::floor(u * simd::f32x_<Count>(static_cast<float>(w))),
				simd::u32x_<Count>(w)
			);
			simd::u32x_<Count> y = wrapping::wrap<Mode>(
				simd::floor(v * simd::f32x_<Count>(static_cast<float>(h))),
				simd::u32x_<Count>(h)
			);
			return sample<Vectorizer>(x, y, mip);
		}
		template <wrapping::mode Mode = wrapping::mode::repeat, size_t Count>
		inline auto sample_nearest(
			simd::f32x_<Count> u,
			simd::f32x_<Count> v,
			size_type mip = 0
		) {
			return sample_nearest<default_vectorizer<Count>, Mode>(u, v, mip);
		}

		template <wrapping::mode Mode = wrapping::mode::repeat, size_t Count>
		inline constexpr math::_scalar<value_type, Count> sample_nearest_x(
			math::f32x<Count> u, math::f32x<Count> v,
			math::u32x<Count> mip = math::u32x<Count>(static_cast<uint32_t>(0))
		) const {
			math::u32x<Count> w = mip_length(math::u32x<Count>(_width), mip);
			math::u32x<Count> h = mip_length(math::u32x<Count>(_height), mip);
			math::u32x<Count> x = wrapping::wrap<Mode>(math::floor<int32_t>(u * w), w);
			math::u32x<Count> y = wrapping::wrap<Mode>(math::floor<int32_t>(v * h), h);
			return sample(x, y, mip);
		}

		// nearest sampling with transposition
		template <typename T, size_t Dim, wrapping::mode Mode = wrapping::mode::repeat, size_t Count>
		inline constexpr math::_vec<T, Dim, Count> sample_nearest_t(
			math::f32x<Count> u, math::f32x<Count> v,
			math::u32x<Count> mip = math::u32x<Count>(static_cast<uint32_t>(0))
		) const {
			math::u32x<Count> w = mip_length(math::u32x<Count>(_width), mip);
			math::u32x<Count> h = mip_length(math::u32x<Count>(_height), mip);
			math::u32x<Count> x = wrapping::wrap<Mode>(math::floor<int32_t>(u * w), w);
			math::u32x<Count> y = wrapping::wrap<Mode>(math::floor<int32_t>(v * h), h);
			return sample_t<T, Dim, Count>(x, y, mip);
		}

		template <auto Interpolate>
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

		template <typename T, size_t Dim, wrapping::mode Mode = wrapping::mode::repeat, size_t Count>
		inline constexpr auto sample_linear_t(
			math::f32x<Count> u, math::f32x<Count> v,
			math::u32x<Count> mip = math::u32x<Count>(static_cast<uint32_t>(0))
		) const {
			math::u32x<Count> w = mip_length(math::u32x<Count>(_width), mip);
			math::u32x<Count> h = mip_length(math::u32x<Count>(_height), mip);
			u *= w;
			v *= h;
			auto coefs = math::f32vec2x4(
				u - math::f32x4(0.5f) - math::floor<float>(u - math::f32x4(0.5f)),
				v - math::f32x4(0.5f) - math::floor<float>(v - math::f32x4(0.5f))
			);
			math::u32x<Count> x0 = wrapping::wrap<Mode>(math::round<int32_t>(u), w);
			math::u32x<Count> x1 = wrapping::wrap<Mode>(x0 - math::u32x<Count>(1), w);
			math::u32x<Count> y0 = wrapping::wrap<Mode>(math::round<int32_t>(v), h);
			math::u32x<Count> y1 = wrapping::wrap<Mode>(y0 - math::u32x<Count>(1), w);
			auto coef = math::f32vec4x4(
				coefs[0] * coefs[1],
				(math::f32x4(1.0f) - coefs[0]) * coefs[1],
				coefs[0] * (math::f32x4(1.0f) - coefs[1]),
				(math::f32x4(1.0f) - coefs[0]) * (math::f32x4(1.0f) - coefs[1])
			);
			return
				(sample_t<T, Dim, Count>(x0, y0, mip) * coef[0]) +
				(sample_t<T, Dim, Count>(x1, y0, mip) * coef[1]) +
				(sample_t<T, Dim, Count>(x0, y1, mip) * coef[2]) +
				(sample_t<T, Dim, Count>(x1, y1, mip) * coef[3]);
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

		template <auto (sampler::*Sample)(float, float, size_type) const>
		inline constexpr auto sample_nearest_mipmap(math::f32x4 u, math::f32x4 v) const {
			math::f32x4 x = u * math::u32x4(_width);
			math::f32x4 y = v * math::u32x4(_height);

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
		constexpr auto smp = rast::sampler<float>(data, 2, 2);
		static_assert(smp.sample(0, 0) == 0.0f);
		static_assert(smp.sample(1, 0) == 1.0f);
		static_assert(smp.sample(0, 1) == 2.0f);
		static_assert(smp.sample(1, 1) == 3.0f);
		static_assert(smp.sample_nearest(0.25f, 0.25f) == 0.0f);
		static_assert(smp.sample_linear(0.5f, 0.5f) == 1.5f);
		constexpr auto addrs = smp.address(
			math::make_x4<uint32_t>(0, 1, 0, 1),
			math::make_x4<uint32_t>(0, 0, 1, 1)
		);
		static_assert(addrs[0] == data);
		static_assert(addrs[1] == data + 1);
		static_assert(addrs[2] == data + 2);
		static_assert(addrs[3] == data + 3);
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
		constexpr float mip_data[16 + 4 + 1] = {
			0.0f, 1.0f, 2.0f, 3.0f,
			4.0f, 5.0f, 6.0f, 7.0f,
			8.0f, 9.0f, 10.0f, 11.0f,
			12.0f, 13.0f, 14.0f, 15.0f,
			16.0f, 17.0f,
			18.0f, 19.0f,
			20.0f
		};
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
