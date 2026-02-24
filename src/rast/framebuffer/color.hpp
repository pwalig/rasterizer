#pragma once
#include "utils.hpp"
#include "../discard_fragment.hpp"
#include "../alpha_blend.hpp"
#include "../image.hpp"
#include "../raster/raster_output_interface.hpp"

namespace rast::framebuffer {
	template <typename ColorFormat, alpha_blend::function::type<ColorFormat> AlphaBlend = default_alpha_blend::blend>
	class color {
	public:
		using color_format = ColorFormat;

	private:
		color_format* _color_data;
		uint32_t _width;
		uint32_t _height;

		inline color_format& at(uint32_t x, uint32_t y) { return _color_data[y * _width + x]; }

	public:
		inline color(color_format* ColorData, uint32_t Width, uint32_t Height) :
			_color_data(ColorData), _width(Width), _height(Height) { }
		inline color(image<color_format>& ColorImage) :
			_color_data(ColorImage), _width(ColorImage.width()), _height(ColorImage.height()) { }

		inline uint32_t width() const { return _width; }
		inline uint32_t height() const { return _height; }
		inline const uint32_t area() const { return _width * _height; }

		void clear(color clear_value) {
			std::fill_n(_color_data, area(), clear_value);
		}

		template <typename Shader>
		void draw(
			uint32_t x, uint32_t y,
			const typename Shader::vertex::output* triangle,
			glm::vec3 partial_coefs,
			const typename Shader::fragment::uniform_buffer& uniform_buffer
		) {
			using fragment_output = typename Shader::fragment::output;
			static_assert(matching_format_v<color_format, fragment_output>);

			fragment_output frag = Shader::fragment::shade(
				interpol::interpolate(triangle[0].data, triangle[1].data, triangle[2].data, interpol::coefs::perspective(partial_coefs, triangle)),
				uniform_buffer
			);
			if constexpr (is_discardable_v<fragment_output>) {
				if (frag) at(x, y) = AlphaBlend(*frag, at(x, y));
			}
			else at(x, y) = AlphaBlend(frag, at(x, y));
		}
	};

	using rgb8 = color<rast::color::rgb8>;
	using rgba8 = color<rast::color::rgba8>;
}
namespace rast::raster {
	template<typename ColorFormat, alpha_blend::function::type<ColorFormat> AlphaBlend>
	inline constexpr output_interface output_interface_v<framebuffer::color<ColorFormat, AlphaBlend>> = output_interface::framebuffer;
}

