#pragma once
#include <stb_image.h>

#include "color.hpp"

namespace rast {
	class texture {
	public:
		using size_type = unsigned int;
		using color = color::rgba8;
		
	private:
		color * data;
		size_type width;
		size_type height;

	public:
		inline texture(const char* filename) : data(nullptr), width(0), height(0) {
			int imgWidth, imgHeight, channels;
			data = (color*)stbi_load(filename, &imgWidth, &imgHeight, &channels, STBI_rgb_alpha);
			if (!data) throw std::runtime_error("failed to load texture image!");
			width = imgWidth;
			height = imgHeight;
		}

		inline ~texture() {
			stbi_image_free(data);
		}

		inline texture(color* Data, size_type Width, size_type Height) :
			data(Data), width(Width), height(Height) { }

		inline color& sample(glm::vec2 coords) {
			size_type x = (size_type)(coords.x * width);
			size_type y = (size_type)(coords.y * height);
			return data[std::min(y * width + x, width * height)];
		}
	};
}
