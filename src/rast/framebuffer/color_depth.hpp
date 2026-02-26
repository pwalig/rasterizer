#pragma once
#include <algorithm>
#include <type_traits>

#include <glm/glm.hpp>

#include "../interpolation.hpp"
#include "../convert.hpp"
#include "../alpha_blend.hpp"
#include "../discard_fragment.hpp"
#include "../depth_test.hpp"
#include "../raster/raster_output_interface.hpp"
#include "utils.hpp"
#include "../sized2d_base.hpp"

namespace rast::framebuffer {
	template<
		typename ColorFormat, typename DepthFormat,
		auto FragmentShader,
		auto AlphaBlend = default_alpha_blend::blend<ColorFormat>,
		depth_test::function::type<DepthFormat> DepthTest = depth_test::less
	> class color_depth : public sized2d_base {
	public:
		using color_format = ColorFormat;
		using depth_format = DepthFormat;
		using size_type = typename sized2d_base::size_type;

	private:
		color_format* _color_data;
		depth_format* _depth_data;

	protected:
		inline constexpr color_format& color(size_type x, size_type y) {
			return _color_data[data_offset(x, y)];
		}
		inline constexpr depth_format& depth(size_type x, size_type y) {
			return _depth_data[data_offset(x, y)];
		}
		template <size_t Count>
		inline math::_scalar<depth_format*, Count> depth_data(
			math::u32x<Count> x, math::u32x<Count> y
		) {
			return math::vectorize<Count>(_depth_data) + (y * math::vectorize<Count>(_width) + x);
		}

	public:
		inline constexpr color_depth(
			color_format* ColorData, depth_format* DepthData,
			size_type Width, size_type Height
		) noexcept : sized2d_base(Width, Height), _color_data(ColorData), _depth_data(DepthData) { }

		template <typename ImageLike1, typename ImageLike2>
		inline constexpr color_depth(ImageLike1& ColorImage, ImageLike2& DepthImage) :
			color_depth(ColorImage.data(), DepthImage.data(), ColorImage.width(), ColorImage.height())
		{
			static_assert(std::is_same_v<typename ImageLike1::value_type, color_format>);
			static_assert(std::is_same_v<typename ImageLike2::value_type, DepthFormat>);
			assert(ColorImage.width() == DepthImage.width());
			assert(ColorImage.height() == DepthImage.height());
		}

		template <typename ImageLike>
		inline constexpr color_depth(color_format* ColorData, ImageLike& DepthImage) :
			color_depth(ColorData, DepthImage.data(), DepthImage.width(), DepthImage.height())
		{
			static_assert(std::is_same_v<typename ImageLike::value_type, depth_format>);
		}
		
		template <typename ImageLike>
		inline constexpr color_depth(ImageLike& ColorImage, depth_format* DepthData) :
			color_depth(ColorImage.data(), DepthData, ColorImage.width(), ColorImage.height())
		{
			static_assert(std::is_same_v<typename ImageLike::value_type, color_format>);
		}

		void clear_color(color_format clear_value) {
			std::fill_n(_color_data, area(), clear_value);
		}

		void clear_depth_buffer(depth_format clear_value = std::numeric_limits<depth_format>::max()) {
			std::fill_n(_depth_data, area(), clear_value);
		}

		template <typename VertexT, typename ...Args>
		inline void operator()(
			size_type x, size_type y,
			const VertexT* triangle,
			glm::vec3 partial_coefs,
			Args&&... args
		) {
			depth_format& oldDepth = depth(x, y);
			depth_format newDepth = get_depth<depth_format>(triangle, partial_coefs);
			if (DepthTest(newDepth, oldDepth)) {
				auto frag = FragmentShader(
					interpol::interpolate(triangle[0].data, triangle[1].data, triangle[2].data, interpol::coefs::perspective(partial_coefs, triangle)),
					std::forward<Args>(args)...
				);
				if constexpr (is_discardable_v<decltype(frag)>) {
					if (frag) {
						color(x, y) = AlphaBlend(*frag, color(x, y));
						oldDepth = newDepth;
					}
				}
				else {
					color(x, y) = AlphaBlend(frag, color(x, y));
					oldDepth = newDepth;
				}
			}
		}
	};
}
