#pragma once
#include <stdexcept>
#include <type_traits>

#include <stb_image.h>

#include "../sa_vector.hpp"
#include "color.hpp"

namespace rast {
	enum class resize_filter : uint8_t {
		dont_care, nearest, cubic
	};

	template <typename T>
	struct mipmapped_image;

	template <typename T>
	struct image {
		using size_type = uint32_t;
		using color = T;
		using value_type = T;
		using reference = std::add_lvalue_reference_t<value_type>;
		using pointer = std::add_pointer_t<value_type>;
		using const_reference = std::add_const_t<reference>;
		using const_pointer = std::add_const_t<pointer>;

	protected:
		pointer _data;
		size_type _width;
		size_type _height;
		inline constexpr image(pointer Data, size_type Width, size_type Height) noexcept : _data(Data), _width(Width), _height(Height) {}

	public:
		inline constexpr image() noexcept : image(nullptr, 0, 0) {}
		inline image(size_type Width, size_type Height) : image(new color[Width * Height], Width, Height) {}

		inline image(const image& rhs) : _data(new color[rhs.area()]), _width(rhs._width), _height(rhs._height) {
			std::memcpy(_data, rhs._data, rhs.area() * sizeof(color));
		}
		inline image& operator=(const image& rhs) {
			if (this != &rhs) {
				if (_data) delete[] _data;
				_data = new color[rhs._width * rhs._height];
				_width = rhs._width;
				_height = rhs._height;
				std::memcpy(_data, rhs._data, _width * _height * sizeof(color));
			}
			return *this;
		}

		inline image(image&& rhs) noexcept : _data(rhs._data), _width(rhs._width), _height(rhs._height) {
			rhs._data = nullptr;
		}
		inline image& operator=(image&& rhs) noexcept {
			if (this != &rhs) {
				if (_data != nullptr) delete[] _data;
				_data = rhs._data;
				_width = rhs._width;
				_height = rhs._height;
				rhs._data = nullptr;
			}
			return *this;
		}
		inline ~image() {
			if (_data != nullptr) delete[] _data;
		}

		inline constexpr size_type width() const noexcept { return _width; }
		inline constexpr size_type height() const noexcept { return _height; }
		inline constexpr size_type area() const noexcept { return _width * _height; }
		inline constexpr size_type size() const noexcept { return _width * _height; }
		inline constexpr reference at(size_type x, size_type y) { return _data[y * _width + x]; }
		inline constexpr const_reference at(size_type x, size_type y) const { return _data[y * _width + x]; }
		inline constexpr pointer data() noexcept { return _data; }
		inline constexpr const_pointer data() const noexcept { return _data; }
		inline constexpr bool empty() const noexcept { return _data != nullptr; }
		inline constexpr explicit operator bool() { return _data != nullptr; }

		template <resize_filter filter>
		inline void resize(size_type Width, size_type Height) {
			if constexpr (filter == resize_filter::dont_care) {
				if (_data != nullptr) delete[] _data;
				_data = new color[Width * Height];
				_width = Width;
				_height = Height;
			}
			else static_assert(false);
		}

		inline void clear(color clear_color) {
			std::fill_n(_data, area(), clear_color);
		}

		template <typename U>
		struct View {
			using size_type = uint32_t;
			using color = U;
			using value_type = U;
			using reference = std::add_lvalue_reference_t<value_type>;
			using pointer = std::add_pointer_t<value_type>;
			using const_reference = std::add_const_t<reference>;
			using const_pointer = std::add_const_t<pointer>;
			
		private:
			pointer _data;
			size_type _width;
			size_type _height;

		public:
			inline constexpr View(pointer Data, size_type Width, size_type Height) noexcept :
				_data(Data), _width(Width), _height(Height) { }

			inline constexpr View(image& img) : view(img.data(), img.width(), img.height()) {}

			inline constexpr size_type width() const noexcept { return _width; }
			inline constexpr size_type height() const noexcept { return _height; }
			inline constexpr size_type area() const noexcept { return _width * _height; }
			inline constexpr size_type size() const noexcept { return _width * _height; }
			inline constexpr reference at(size_type x, size_type y) { return _data[y * _width + x]; }
			inline constexpr const_reference at(size_type x, size_type y) const { return _data[y * _width + x]; }
			inline constexpr pointer data() noexcept { return _data; }
			inline constexpr const_pointer data() const noexcept { return _data; }
			inline constexpr bool empty() { return _data == nullptr || _width <= 0 || _height <= 0; }
			inline constexpr explicit operator bool() { return _data != nullptr; }

			inline void clear(color clear_color) {
				std::fill_n(_data, _width * _height, clear_color);
			}
		};
		using view = View<value_type>;
		using const_view = View<const value_type>;

		inline view get_view() { return view(_data, _width, _height); }

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

	template <typename T>
	struct mipmapped_image : public image<T> {
		using size_type = uint32_t;
		using color = T;
		using value_type = T;
		using reference = std::add_lvalue_reference_t<value_type>;
		using pointer = std::add_pointer_t<value_type>;
		using const_reference = std::add_const_t<reference>;
		using const_pointer = std::add_const_t<pointer>;

		inline static constexpr size_type mip_levels(size_type Width, size_type Height) noexcept {
			size_type MipLevels = 0;
			while (Width > 0 && Height > 0) {
				MipLevels += 1;
				Width /= 2;
				Height /= 2;
			}
			return MipLevels;
		}

		inline static constexpr size_type size(size_type Width, size_type Height) noexcept {
			size_type res = 0;
			while (Width > 0 && Height > 0) {
				res += Width * Height;
				Width /= 2;
				Height /= 2;
			}
			return res;
		}
		inline static constexpr size_type valid_mip_level(size_type Width, size_type Height, size_type mip) noexcept {
			return std::min(mip, mip_levels(Width, Height) - 1);
		}
		inline static constexpr size_type mip_offset(size_type Width, size_type Height, size_type mip) noexcept {
			size_type offset = 0;
			mip = valid_mip_level(Width, Height, mip);
			while (mip > 0) {
				offset += Width * Height;
				Width /= 2;
				Height /= 2;
				--mip;
			}
			return offset;
		}
		inline static constexpr size_type length_at_mip_level(size_type Length, size_type mip) noexcept {
			return Length >> mip;
		}
		inline static constexpr size_type length_at_valid_mip_level(size_type Length, size_type Width, size_type Height, size_type mip) noexcept {
			return length_at_mip_level(Length, valid_mip_level(Width, Height, mip));
		}
		inline static constexpr void downsize(typename image<T>::view Dst, typename image<T>::view Src) {
			for (size_type x = 0; x < Dst.width(); ++x) {
				for (size_type y = 0; y < Dst.height(); ++y) {
					const value_type div = static_cast<value_type>(4);
					Dst.at(x, y) =
						(Src.at(x * 2, y * 2) / div) +
						(Src.at(x * 2, y * 2 + 1) / div) +
						(Src.at(x * 2 + 1, y * 2) / div) +
						(Src.at(x * 2 + 1, y * 2 + 1) / div);
				}
			}
		}
		
		inline constexpr mipmapped_image() : image<T>() {}
		inline mipmapped_image(size_type Width, size_type Height) : image<T>(new color[size(Width, Height)], Width, Height) {}
		inline explicit mipmapped_image(mipmapped_image<T>&& rhs) : image<T>(rhs.image<T>::_data, rhs.image<T>::_width, rhs.image<T>::_height) {
			rhs.image<T>::_data = nullptr;
		}
		inline explicit mipmapped_image(const image<T>& Image) : mipmapped_image(Image.width(), Image.height()) {
			std::memcpy(image<T>::_data, Image.data(), Image.area() * sizeof(color));
			for (size_type mip = 1; mip < mip_levels(); ++mip) {
				downsize(mip_view(mip), mip_view(mip - 1));
			}
		}

		inline constexpr size_type width() const { return image<T>::_width; }
		inline constexpr size_type height() const { return image<T>::_height; }
		inline constexpr size_type width(size_type mip) const { return length_at_mip_level(width(), mip); }
		inline constexpr size_type height(size_type mip) const { return length_at_mip_level(height(), mip); }
		inline constexpr size_type area() const noexcept { return image<T>::_width * image<T>::_height; }
		inline constexpr size_type area(size_type mip) const noexcept { return width(mip) * height(mip); }
		inline constexpr size_type mip_levels() const { return mip_levels(width(), height()); }

		inline constexpr size_type size() const noexcept { return size(image<T>::_width, image<T>::_height); }
		inline constexpr size_type size(size_type mip) const noexcept { return area(mip); }
		inline constexpr size_type capacity() const noexcept { return size(); }

		inline constexpr size_type mip_offset(size_type mip) const noexcept { return mip_offset(width(), height(), mip); }
		inline constexpr pointer data() noexcept { return image<T>::_data; }
		inline constexpr const_pointer data() const noexcept { return image<T>::_data; }
		inline constexpr pointer data(size_type mip) noexcept { return image<T>::_data + mip_offset(mip); }
		inline constexpr const_pointer data(size_type mip) const noexcept { return image<T>::_data + mip_offset(mip); }

		inline constexpr reference at(size_type x, size_type y) { return image<T>::_data[y * image<T>::_width + x]; }
		inline constexpr const_reference at(size_type x, size_type y) const { return image<T>::_data[y * image<T>::_width + x]; }
		inline constexpr reference at(size_type x, size_type y, size_type mip) { return data(mip)[y * width(mip) + x]; }
		inline constexpr const_reference at(size_type x, size_type y, size_type mip) const { return data(mip)[y * width(mip) + x]; }

		inline constexpr typename image<T>::view mip_view(size_type mip) { return typename image<T>::view(data(mip), width(mip), height(mip)); }
		inline constexpr typename image<T>::const_view mip_view(size_type mip) const { return typename image<T>::const_view(data(mip), width(mip), height(mip)); }

		inline constexpr typename image<T>::view operator[](size_type mip) { return mip_view(mip); }
		inline constexpr typename image<T>::const_view operator[](size_type mip) const { return mip_view(mip); }
	};
}
