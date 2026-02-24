#pragma once
#include <algorithm>

#include <glm/glm.hpp>

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
		alpha_blend::function::type<ColorFormat> AlphaBlend = default_alpha_blend::blend,
		depth_test::function::type<DepthFormat> DepthTest = depth_test::less
	> class color_depth {
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
		inline color_format& color(size_type x, size_type y) {
			return _color_data[y * _width + x];
		}
		inline depth_format& depth(size_type x, size_type y) {
			return _depth_data[y * _width + x];
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
		) noexcept : _color_data(ColorData), _depth_data(DepthData),
			_width(Width), _height(Height) { }

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

		inline constexpr size_type width() const noexcept { return _width; }
		inline constexpr size_type height() const noexcept { return _height; }
		inline constexpr const size_type area() const noexcept { return _width * _height; }

		void clear_color(color_format clear_value) {
			std::fill_n(_color_data, area(), clear_value);
		}

		void clear_depth_buffer(depth_format clear_value = std::numeric_limits<depth_format>::max()) {
			std::fill_n(_depth_data, area(), clear_value);
		}

		template <typename Shader>
		void draw(
			size_type x, size_type y,
			const typename Shader::vertex::output* triangle,
			glm::vec3 partial_coefs,
			const typename Shader::fragment::uniform_buffer& uniform_buffer
		) {
			using fragment_output = typename Shader::fragment::output;
			static_assert(matching_format_v<color_format, fragment_output>);

			// depth test
			depth_format& oldDepth = depth(x, y);
			depth_format newDepth = get_depth<depth_format>(triangle, partial_coefs);
			if (DepthTest(newDepth, oldDepth)) {

				fragment_output frag = Shader::fragment::shade(
					interpol::interpolate(triangle[0].data, triangle[1].data, triangle[2].data, interpol::coefs::perspective(partial_coefs, triangle)),
					uniform_buffer
				);
				if constexpr (is_discardable_v<fragment_output>) {
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

		template <typename Shader>
		auto raster_adapter(const typename Shader::fragment::uniform_buffer& uniform_buffer) {
			return [this, &uniform_buffer](
				size_type x, size_type y, const typename Shader::vertex::output* triangle, glm::vec3 partial_coefs
				) {
					this->draw<Shader>(x, y, triangle, partial_coefs, uniform_buffer);
				};
		}
	};

	using rgba8_u8 = color_depth<rast::color::rgba8, uint8_t>;
	using rgba8_u16 = color_depth<rast::color::rgba8, uint16_t>;
	using rgba8_u32 = color_depth<rast::color::rgba8, uint32_t>;
	using rgba8_u64 = color_depth<rast::color::rgba8, uint64_t>;

	using rgb8_u8 = color_depth<rast::color::rgb8, uint8_t>;
	using rgb8_u16 = color_depth<rast::color::rgb8, uint16_t>;
	using rgb8_u32 = color_depth<rast::color::rgb8, uint32_t>;
	using rgb8_u64 = color_depth<rast::color::rgb8, uint64_t>;

	using rgba8_f = color_depth<rast::color::rgb8, float>;
	using rgba8_d = color_depth<rast::color::rgb8, double>;

	using rgb8_f = color_depth<rast::color::rgb8, float>;
	using rgb8_d = color_depth<rast::color::rgb8, double>;
}

namespace rast::raster {
	template<typename ColorFormat, typename DepthFormat, alpha_blend::function::type<ColorFormat> AlphaBlend, depth_test::function::type<DepthFormat> DepthTest>
	inline constexpr output_interface output_interface_v<framebuffer::color_depth<ColorFormat, DepthFormat, AlphaBlend, DepthTest>> = output_interface::framebuffer;
}
