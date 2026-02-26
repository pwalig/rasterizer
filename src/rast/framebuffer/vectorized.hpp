#pragma once
#include <algorithm>

#include "../interpolation.hpp"
#include "../convert.hpp"
#include "../alpha_blend.hpp"
#include "../discard_fragment.hpp"
#include "../depth_test.hpp"
#include "../raster/raster_output_interface.hpp"
#include "utils.hpp"

namespace rast::framebuffer {
	template<
		typename ColorFormat, typename DepthFormat,
		auto FragmentShader, auto DepthTest, auto AlphaBlend
	> class vectorized {
	public:
		using color_format = ColorFormat;
		using depth_format = DepthFormat;
		using size_type = uint32_t;

	private:
		color_format* _color_data;
		depth_format* _depth_data;
		size_type _width;
		size_type _height;

	protected:
		inline constexpr color_format& color(size_type x, size_type y) {
			return _color_data[y * _width + x];
		}
		inline constexpr depth_format& depth(size_type x, size_type y) {
			return _depth_data[y * _width + x];
		}
		template <size_t Count>
		inline constexpr math::u32x<Count> data_offset(
			math::u32x<Count> x, math::u32x<Count> y
		) {
			return y * math::vectorize<Count>(_width) + x;
		}
		template <size_t Count>
		inline constexpr math::_scalar<color_format*, Count> color_data(
			math::u32x<Count> x, math::u32x<Count> y
		) {
			return math::vectorize<Count>(_color_data) + data_offset(x, y);
		}
		template <size_t Count>
		inline constexpr math::_scalar<depth_format*, Count> depth_data(
			math::u32x<Count> x, math::u32x<Count> y
		) {
			return math::vectorize<Count>(_depth_data) + data_offset(x, y);
		}

	public:
		inline constexpr vectorized(
			color_format* ColorData, depth_format* DepthData,
			size_type Width, size_type Height
		) noexcept : _color_data(ColorData), _depth_data(DepthData),
			_width(Width), _height(Height) { }

		template <typename ImageLike1, typename ImageLike2>
		inline constexpr vectorized(ImageLike1& ColorImage, ImageLike2& DepthImage) :
			vectorized(ColorImage.data(), DepthImage.data(), ColorImage.width(), ColorImage.height())
		{
			static_assert(std::is_same_v<typename ImageLike1::value_type, color_format>);
			static_assert(std::is_same_v<typename ImageLike2::value_type, DepthFormat>);
			assert(ColorImage.width() == DepthImage.width());
			assert(ColorImage.height() == DepthImage.height());
		}

		template <typename ImageLike>
		inline constexpr vectorized(color_format* ColorData, ImageLike& DepthImage) :
			vectorized(ColorData, DepthImage.data(), DepthImage.width(), DepthImage.height())
		{
			static_assert(std::is_same_v<typename ImageLike::value_type, depth_format>);
		}
		
		template <typename ImageLike>
		inline constexpr vectorized(ImageLike& ColorImage, depth_format* DepthData) :
			vectorized(ColorImage.data(), DepthData, ColorImage.width(), ColorImage.height())
		{
			static_assert(std::is_same_v<typename ImageLike::value_type, color_format>);
		}

		inline constexpr size_type width() const noexcept { return _width; }
		inline constexpr size_type height() const noexcept { return _height; }
		inline constexpr const size_type area() const noexcept { return _width * _height; }

		void clear_color(color_format clear_value) {
			std::fill_n(_color_data, area(), clear_value);
		}

		void clear_depth_buffer(depth_format clear_value = std::numeric_limits<depth_format>::max()) {
			std::fill_n(_depth_data, area(), clear_value);
		}

		template<size_t Count, typename VertexT, typename ...Args>
		inline constexpr void operator()(
			math::u32x<Count> x, math::u32x<Count> y,
			const VertexT& v0, const VertexT& v1, const VertexT& v2,
			const math::f32vec3x<Count>& z, const math::f32vec3x<Count> w,
			const math::f32vec3x<Count>& partial_coefs,
			const math::boolx<Count>& mask, Args... args
		) {
			math::u32x<Count> data_off = data_offset(x, y);

			math::f32vec3x<Count> linear_coefs = interpol::coefs::linear(partial_coefs);
			math::f32x<Count> float_depth = (z[0] * linear_coefs[0]) + (z[1] * linear_coefs[1]) + (z[2] * linear_coefs[2]);
			math::_scalar<depth_format, Count> new_depth = float_depth_to_depth_format<depth_format>(float_depth);
			math::boolx<Count> depth_mask = DepthTest(new_depth, math::load(_depth_data, data_off, mask)) && mask;

			if (math::or_accross(depth_mask)) {
				math::f32vec3x<Count> persp_coefs = interpol::coefs::perspective(partial_coefs, w);
				auto interpolated = (v0 * persp_coefs[0]) + (v1 * persp_coefs[1]) + (v2 * persp_coefs[2]); // interpolated can be anything
				auto frag = FragmentShader(interpolated, args...); // frag can by anything

				if constexpr (is_discardable_v<decltype(frag)>) {
					auto nodiscard_mask = math::boolx<Count>(frag);
					if (math::or_accross(nodiscard_mask)) {
						auto new_color = AlphaBlend(*frag, math::load(_color_data, data_off, mask)); // new_color might be a math::_vec or math::_scalar
						math::store(_color_data, data_off, new_color, depth_mask && nodiscard_mask);
						math::store(_depth_data, data_off, new_depth, depth_mask);
					}
				} else {
					auto new_color = AlphaBlend(frag, math::load(_color_data, data_off, mask)); // new_color might be a math::_vec or math::_scalar
					math::store(_color_data, data_off, new_color, depth_mask);
					math::store(_depth_data, data_off, new_depth, depth_mask);
				}
			}
			//color(x[0], y[0]).r()[0] = 255;
		}
	};
}
