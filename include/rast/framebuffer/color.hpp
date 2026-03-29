#pragma once
#include "utils.hpp"
#include "../sized2d_base.hpp"
#include "../is_discardable.hpp"
#include "../alpha_blend.hpp"
#include "../image.hpp"

namespace rast::framebuffer {
	template <typename ColorFormat,
		auto FragmentShader,
		auto AlphaBlend = default_alpha_blend::blend
	> struct color : sized2d_base {
		using color_format = ColorFormat;
		using size_type = typename sized2d_base::size_type;

	private:
		color_format* _color_data;

		constexpr color_format& at(size_type x, size_type y) {
			return _color_data[y * _width + x];
		}

	public:
		constexpr color(
			color_format* ColorData, size_type Width, size_type Height
		) : noexcept _color_data(ColorData), _width(Width), _height(Height) {}

		template <typename ImageLike>
		constexpr color(ImageLike& ColorImage) : color(
			ColorImage.data(), ColorImage.width(), ColorImage.height()
		) {}

		constexpr void clear(const color& clear_value) {
			std::fill_n(_color_data, area(), clear_value);
		}

		template <typename VertexT, typename ...Args>
		inline constexpr void operator()(
			size_type x, size_type y,
			const VertexT* triangle,
			glm::vec3 partial_coefs,
			Args&&... args
		) {
			auto frag = FragmentShader(
				interpol::interpolate(triangle[0].data, triangle[1].data, triangle[2].data, interpol::coefs::perspective(partial_coefs, triangle)),
				std::forward<Args>(args)...
			);
			if constexpr (is_discardable_v<decltype(frag)>) {
				if (frag) at(x, y) = AlphaBlend(*frag, at(x, y));
			}
			else at(x, y) = AlphaBlend(frag, at(x, y));
		}
	};
}
