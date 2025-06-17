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
		typename image<color_format>::view colorImage;
		typename image<depth_format>::view depthImage;

		inline color_depth(const typename image<color_format>::view& ColorImage, const typename image<depth_format>::view& DepthImage) :
			colorImage(ColorImage), depthImage(DepthImage) { }

		inline color_depth(typename image<color_format>& ColorImage, typename image<depth_format>& DepthImage) :
			colorImage(ColorImage), depthImage(DepthImage) { }

		inline color_depth(typename image<color_format>& ColorImage, const typename image<depth_format>::view& DepthImage) :
			colorImage(ColorImage), depthImage(DepthImage) { }

		inline color_depth(const typename image<color_format>::view& ColorImage, typename image<depth_format>& DepthImage) :
			colorImage(ColorImage), depthImage(DepthImage) { }

		void clear_color(color_format clear_value) {
			colorImage.clear(clear_value);
		}

		void clear_depth_buffer(depth_format clear_value = std::numeric_limits<depth_format>::max()) {
			depthImage.clear(clear_value);
		}

		inline uint32_t width() const {
			return colorImage.width;
		}
		inline uint32_t height() const {
			return colorImage.height;
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
			depth_format& oldDepth = depthImage.at(x, y);
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
				colorImage.at(x, y) = Shader::fragment::shade(
					interpol::interpolate(triangle[0].data, triangle[1].data, triangle[2].data, interpol::persp_coefs(results, area, triangle)),
					uniform_buffer
				);
			}
		}
	};

	class rgba8 {
	public:
		using color = color::rgba8;
		image<color>::view colorImage;

		inline rgba8(const image<color>::view& ColorImage) :
			colorImage(ColorImage) { }

		void clear(color clear_value) {
			std::fill_n(colorImage.data, colorImage.width * colorImage.height, clear_value);
		}

		inline uint32_t width() const {
			return colorImage.width;
		}
		inline uint32_t height() const {
			return colorImage.height;
		}

		template <typename Shader>
		void draw(
			uint32_t x, uint32_t y,
			const typename Shader::vertex::output* triangle,
			const typename Shader::fragment::uniform_buffer& uniform_buffer,
			const glm::ivec3& results, int area
		) {
			// output color
			colorImage.at(x, y) = Shader::fragment::shade(
				interpolate(triangle[0].data, triangle[1].data, triangle[2].data, interpol::persp_coefs(results, area, triangle)),
				uniform_buffer
			);
		}
	};
}
