#pragma once
#include <algorithm>

#include <glm/glm.hpp>

#include "types.hpp"
#include "image.hpp"

namespace rast::framebuffer {
	template <typename T>
	inline T interpolate(const T& a, const T& b, const T& c, const glm::vec3& coefs) {
		return (T)(a * coefs.x) + (T)(b * coefs.y) + (T)(c * coefs.z);
	}

	template <typename T>
	inline T interpolate(const glm::vec<3, T>& values, const glm::vec3& coefs) {
		return (T)(values.x * coefs.x) + (T)(values.y * coefs.y) + (T)(values.z * coefs.z);
	}

	template <typename T>
	inline T interpolate2(T a, T b, T c, const glm::vec3& coefs) {
		return (T)(a * coefs.x) + (T)(b * coefs.y) + (T)(c * coefs.z);
	}

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

		template <typename Shader>
		void draw(u32 x, u32 y, const typename Shader::vertex::output* triangle, glm::vec3 coefs) {
			// depth test
			glm::vec3 w(
				triangle[0].rastPos.w,
				triangle[1].rastPos.w,
				triangle[2].rastPos.w
			);
			glm::vec3 z(
				triangle[0].rastPos.z,
				triangle[1].rastPos.z,
				triangle[2].rastPos.z
			);
			depth_format& oldDepth = depthImage.at(x, y);
			depth_format newDepth = (depth_format)((interpolate(z / w, coefs) * 0.5f + 0.5f) * std::numeric_limits<depth_format>::max());
			coefs /= w;
			float sum = coefs.x + coefs.y + coefs.z;
			coefs /= sum;
			if (newDepth < oldDepth) {
				oldDepth = newDepth;

				// output color
				colorImage.at(x, y) = Shader::fragment::shade(
					triangle[0].data, triangle[1].data, triangle[2].data, coefs
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

		template <typename Shader>
		void draw(u32 x, u32 y, const typename Shader::vertex::output* triangle, glm::vec3 coefs) {
			glm::vec3 w(
				triangle[0].rastPos.w,
				triangle[1].rastPos.w,
				triangle[2].rastPos.w
			);
			coefs /= w;
			float sum = coefs.x + coefs.y + coefs.z;
			coefs /= sum;
			// output color
			colorImage.at(x, y) = Shader::fragment::shade(
				triangle[0].data, triangle[1].data, triangle[2].data, coefs
			);
		}
	};
}
