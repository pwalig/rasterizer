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
	template<typename ColorFormat, typename DepthFormat>
	class vectorized {
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
		inline constexpr math::_scalar<color_format*, Count> color_data(
			math::u32x<Count> x, math::u32x<Count> y
		) {
			return math::vectorize<Count>(_color_data) + (y * math::vectorize<Count>(_width) + x);
		}
		template <size_t Count>
		inline constexpr math::_scalar<depth_format*, Count> depth_data(
			math::u32x<Count> x, math::u32x<Count> y
		) {
			return math::vectorize<Count>(_depth_data) + (y * math::vectorize<Count>(_width) + x);
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

		template <
			auto FragmentShader, auto DepthTest,
			size_t Count, typename VertexT, typename ...Args
		> inline constexpr void draw(
			math::u32x<Count> x, math::u32x<Count> y,
			const VertexT& v0, const VertexT& v1, const VertexT& v2,
			const math::f32vec3x<Count>& persp_coefs, Args... args
		) {
			auto interpolated = (v0 * persp_coefs[0]) + (v1 * persp_coefs[1]) + (v2 * persp_coefs[2]);
			auto frag = FragmentShader(interpolated, args...);
			math::store(color_data(x, y), frag);
			//math::_scalar<depth_format, Count> newDepth = get_depth<depth_format>(
			//	v0.rastPos[3], v1.rastPos[3], v2.rastPos[3], partial_coefs
			//);
			//math::_scalar<depth_format*, Count> old_depth_ptr = depth_data(x, y);
			//if (math::or_accross(DepthTest(newDepth, math::load(old_depth_ptr)))) {
				//auto frag = math::transpose(FragmentShader(v0, args...));
				//color_format frag = math::transpose(FragmentShader(
				//	interpol::interpolate(
				//		v0.data, v1.data, v2.data,
				//		interpol::coefs::perspective(partial_coefs, v0.rastPos[4], v1.rastPos[4], v2.rastPos[4])
				//	), args...
				//));
				//math::store(color_data(x, y), frag);
				//math::store(old_depth_ptr, newDepth);
			//}
		}
	};
}
