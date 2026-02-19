#pragma once
#include <stdexcept>

#include <stb_image.h>

#include "../sa_vector.hpp"
#include "color.hpp"

namespace rast {
	enum class resize_filter : uint8_t {
		dont_care, nearest, cubic
	};

	template <typename PixelT>
	class image {
	public:
		using size_type = uint32_t;
		using color = PixelT;
		using pixel = PixelT;

	private:
		color* _data;
		size_type _width;
		size_type _height;

	public:
		inline image() : _data(nullptr), _width(0), _height(0) {}
		inline image(size_type Width, size_type Height) : _data(new color[Width * Height]), _width(Width), _height(Height) {}

		image(const image& rhs) : _data(new color[rhs.area()]), _width(rhs._width), _height(rhs._height) {
			std::memcpy(_data, rhs._data, rhs.area() * sizeof(color));
		}
		image& operator=(const image& rhs) {
			if (this != &rhs) {
				if (_data) delete[] _data;
				_data = new color[rhs._width * rhs._height];
				_width = rhs._width;
				_height = rhs._height;
				std::memcpy(_data, rhs._data, _width * _height * sizeof(color));
			}
			return *this;
		}

		image(image&& rhs) noexcept : _data(rhs._data), _width(rhs._width), _height(rhs._height) {
			rhs._data = nullptr;
		}
		image& operator=(image&& rhs) noexcept {
			if (this != &rhs) {
				std::swap(_data, rhs._data);
				_width = rhs._width;
				_height = rhs._height;
			}
			return *this;
		}
		~image() {
			if (_data) delete[] _data;
		}

		inline size_type width() const { return _width; }
		inline size_type height() const { return _height; }
		inline color& at(size_type x, size_type y) { return _data[y * _width + x]; }
		inline const color& at(size_type x, size_type y) const { return _data[y * _width + x]; }
		inline color* data() { return _data; }
		inline const color* data() const { return _data; }
		inline const size_type area() const { return _width * _height; }

		template <resize_filter filter>
		inline void resize(size_type Width, size_type Height) {
			if constexpr (filter == resize_filter::dont_care) {
				if (_data) delete[] _data;
				_data = new color[Width * Height];
				_width = Width;
				_height = Height;
			}
			else static_assert(false);
		}

		inline void clear(color clear_color) {
			std::fill_n(_data, area(), clear_color);
		}

		class view {
		public:
			using size_type = uint32_t;
			using color = PixelT;
			using pixel = PixelT;
			
			color * const data;
			const size_type width;
			const size_type height;

			inline view(color* Data, size_type Width, size_type Height) :
				data(Data), width(Width), height(Height) { }

			inline view(image& img) : view(img.data(), img.width(), img.height()) {}

			inline color& at(size_type x, size_type y) { return data[y * width + x]; }
			inline void clear(color clear_color) {
				std::fill_n(data, width * height, clear_color);
			}
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
