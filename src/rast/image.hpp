#pragma once
#include <stdexcept>

#include <stb_image.h>

#include "../sa_vector.hpp"
#include "color.hpp"

namespace rast {
	template <typename PixelT>
	class image {
	public:
		using size_type = u32;
		using color = PixelT;
		using pixel = PixelT;

	private:
		sa_vector<color> _data;
		size_type _width;
		size_type _height;

	public:
		inline image() : _data(), _width(0), _height(0) {}
		inline image(size_type Width, size_type Height) : _data(Width * Height), _width(Width), _height(Height) {}

		inline size_type width() const { return _width; }
		inline size_type height() const { return _height; }
		inline color* data() { return _data.data(); }
		inline const color* data() const { return _data.data(); }
		inline sa_vector<color>& storage() { return _data; }
		inline const sa_vector<color>& storage() const { return _data; }

		class view {
		public:
			using size_type = u32;
			using color = PixelT;
			using pixel = PixelT;
			
			color * const data;
			const size_type width;
			const size_type height;

			inline view(color* Data, size_type Width, size_type Height) :
				data(Data), width(Width), height(Height) { }

			inline view(image& img) : view(img.data(), img.width(), img.height()) {}

			inline color& at(size_type x, size_type y) { return data[y * width + x]; }
		};

		inline static image load(const char* filename) {
			int imgWidth, imgHeight, channels;
			stbi_uc* data = stbi_load(filename, &imgWidth, &imgHeight, &channels, STBI_rgb_alpha);
			if (!data) throw std::runtime_error("failed to load texture image!");
			image res(imgWidth, imgHeight);
			std::memcpy((void*)res.data(), (void*)data, imgWidth * imgHeight * sizeof(color));
			stbi_image_free(data);
			return res;
		}
	};
}
