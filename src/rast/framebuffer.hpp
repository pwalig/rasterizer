#pragma once
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

	class rgba8_depth {
	public:
		image<color::rgba8>::view colorImage;
		image<int>::view depthImage;

		inline rgba8_depth(const image<color::rgba8>::view& ColorImage, const image<int>::view DepthImage) :
			colorImage(ColorImage), depthImage(DepthImage) { }

		template <typename Shader>
		void draw(int x, int y, const typename Shader::vertex::output* triangle, glm::vec3 coefs) {
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
			int& oldDepth = depthImage.at(x, y);
			int newDepth = (int)((interpolate(z / w, coefs) * 0.5f + 0.5f) * std::numeric_limits<int>::max());
			coefs /= w;
			float sum = coefs.x + coefs.y + coefs.z;
			coefs /= sum;
			if (newDepth < oldDepth) {
				oldDepth = newDepth;

				// output color
				colorImage.at(x, y) = Shader::fragment::shade(
					Shader::fragment::interpolate(
						triangle[0].data,
						triangle[1].data,
						triangle[2].data,
						coefs
					)
				);
			}
		}
	};
}
