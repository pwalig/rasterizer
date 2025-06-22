#pragma once
#include <algorithm>

#include <glm/glm.hpp>

#include "image.hpp"
#include "interpolation.hpp"

namespace rast::framebuffer {
	template<typename ColorFormat, typename DepthFormat>
	class color_depth {
	public:
		using depth_format = DepthFormat;
		using color_format = ColorFormat;
		color_format* colorImage;
		depth_format* depthImage;
		uint32_t _width;
		uint32_t _height;

		inline color_depth(color_format* ColorData, depth_format* DepthData, uint32_t Width, uint32_t Height) :
			colorImage(ColorData), depthImage(DepthData), _width(Width), _height(Height) { }
		inline color_depth(image<color_format>& ColorImage, image<depth_format>& DepthImage) :
			colorImage(ColorImage.data()), depthImage(DepthImage.data()), _width(ColorImage.width()), _height(ColorImage.height()) {}
		inline color_depth(color_format* ColorData, image<depth_format>& DepthImage) :
			colorImage(ColorData), depthImage(DepthImage.data()), _width(DepthImage.width()), _height(DepthImage.height()) {}
		inline color_depth(image<color_format>& ColorImage, depth_format* DepthData) :
			colorImage(ColorImage.data()), depthImage(DepthData.data()), _width(ColorImage.width()), _height(ColorImage.height()) {}

		inline uint32_t width() const { return _width; }
		inline uint32_t height() const { return _height; }
		inline const uint32_t area() const { return _width * _height; }

		inline color_format& color(uint32_t x, uint32_t y) {
			return colorImage[y * _width + x];
		}
		inline depth_format& depth(uint32_t x, uint32_t y) {
			return depthImage[y * _width + x];
		}

		void clear_color(color_format clear_value) {
			std::fill_n(colorImage, area(), clear_value);
		}

		void clear_depth_buffer(depth_format clear_value = std::numeric_limits<depth_format>::max()) {
			std::fill_n(depthImage, area(), clear_value);
		}

		template <typename Shader>
		void draw(
			uint32_t x, uint32_t y,
			const typename Shader::vertex::output* triangle,
			const typename Shader::fragment::uniform_buffer& uniform_buffer,
			const glm::ivec3& results, int area
		) {
			// depth test
			glm::vec3 z(
				triangle[0].rastPos.z,
				triangle[1].rastPos.z,
				triangle[2].rastPos.z
			);
			depth_format& oldDepth = depth(x, y);
			depth_format newDepth = static_cast<depth_format>(((
					//area < (1 << 12) ? triangle[0].rastPos.z :
					interpol::interpolate(z, interpol::linear_coefs(results, area))
				) * 0.5f + 0.5f) * std::numeric_limits<depth_format>::max());
			//depth_format newDepth = static_cast<depth_format>(interpolate(z, coefs));
			if (newDepth < oldDepth) {
				oldDepth = newDepth;

				// output color
				//colorImage.at(x, y).r = static_cast<uint8_t>(newDepth * ((double)std::numeric_limits<uint8_t>::max() / std::numeric_limits<depth_format>::max()));
				//colorImage.at(x, y).g = newDepth / (depth_format(1) << 18) % std::numeric_limits<uint8_t>::max();
				//float sum = coefs.x + coefs.y + coefs.z;
				color(x, y) = Shader::fragment::shade(
					interpol::interpolate(triangle[0].data, triangle[1].data, triangle[2].data, interpol::persp_coefs(results, area, triangle)),
					uniform_buffer
				);
			}
		}
	};

	using rgba8_u32 = color_depth<rast::color::rgba8, uint32_t>;

	template <typename ColorFormat>
	class color {
	public:
		using color_format = ColorFormat;
		color_format* colorImage;
		uint32_t _width;
		uint32_t _height;

		inline color(color_format* ColorData, uint32_t Width, uint32_t Height) :
			colorImage(ColorData), _width(Width), _height(Height) { }
		inline color(image<color_format>& ColorImage) :
			colorImage(ColorImage), _width(ColorImage.width()), _height(ColorImage.height()) { }

		void clear(color clear_value) {
			std::fill_n(colorImage.data, colorImage.width * colorImage.height, clear_value);
		}

		inline uint32_t width() const { return _width; }
		inline uint32_t height() const { return _height; }
		inline const uint32_t area() const { return _width * _height; }
		inline color_format& at(uint32_t x, uint32_t y) { return colorImage[y * _width + x]; }

		template <typename Shader>
		void draw(
			uint32_t x, uint32_t y,
			const typename Shader::vertex::output* triangle,
			const typename Shader::fragment::uniform_buffer& uniform_buffer,
			const glm::ivec3& results, int area
		) {
			// output color
			at(x, y) = Shader::fragment::shade(
				interpol::interpolate(triangle[0].data, triangle[1].data, triangle[2].data, interpol::persp_coefs(results, area, triangle)),
				uniform_buffer
			);
		}
	};

	using rgba8 = color<rast::color::rgba8>;
}
