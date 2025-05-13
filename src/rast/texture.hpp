#pragma once
#include "color.hpp"
#include "image.hpp"

namespace rast {
	template <typename ColorT = color::rgba8>
	class texture {
	public:
		using size_type = u32;
		using color = ColorT;

		class sampler {
		private:
			const color* data;
			size_type width;
			size_type height;

		public:
			inline sampler() : data(nullptr), width(0), height(0) {}
			inline sampler(const color* Data, size_type Width, size_type Height) :
				data(Data), width(Width), height(Height) { }
			inline sampler(const image<color>& img) : sampler(img.data(), img.width(), img.height()) {}

			inline color sample(glm::vec2 coords) const {
				size_type x = (size_type)(coords.x * (width - 1)) % width;
				size_type y = (size_type)(coords.y * (height - 1)) % height;
				return data[y * width + x];
			}

			inline explicit operator bool() const { return data != nullptr; }
		};
	};
}
