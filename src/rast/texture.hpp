#pragma once
#include "color.hpp"
#include "image.hpp"

namespace rast {
	template <typename ColorT = color::rgba8>
	class texture {
	public:
		using size_type = uint32_t;
		using color = ColorT;

		class sampler {
		private:
			const color* data;
			size_type width;
			size_type height;

		public:
			inline constexpr sampler() : data(nullptr), width(0), height(0) {}
			inline constexpr sampler(const color* Data, size_type Width, size_type Height) :
				data(Data), width(Width), height(Height) { }
			inline constexpr sampler(const image<color>& img) : sampler(img.data(), img.width(), img.height()) {}

			inline constexpr color sample(size_type x, size_type y) const {
				return data[y * width + x];
			}

			inline constexpr color sample_nearest(float u, float v) const {
				size_type x = static_cast<size_type>(u * width) % width; // static_cast rounds towards 0
				size_type y = static_cast<size_type>(v * height) % height;
				return sample(x, y);
			}

			inline constexpr color sample_linear(float u, float v) const {
				u *= width;
				v *= height;
				float coefs[2] = {
					std::round(u - 0.5f) - u,
					std::round(v - 0.5f) - v
				};
				size_type x[2] = {
					static_cast<size_type>(std::round(u)),
					static_cast<size_type>(std::round(u - 1.0f))
				};
				size_type y[2] = {
					static_cast<size_type>(std::round(v)),
					static_cast<size_type>(std::round(v - 1.0f))
				};
				return (((sample(x[0], y[0]) * coefs[0]) + (sample(x[1], y[0]) * (1.0f - coefs[0]))) * coefs[1]) +
					(((sample(x[0], y[1]) * coefs[0]) + (sample(x[1], y[1]) * (1.0f - coefs[0]))) * (1.0f - coefs[1]));
			}

			inline constexpr color sample(glm::vec2 coords) const {
				size_type x = static_cast<size_type>(coords.x * (width - 1)) % width;
				size_type y = static_cast<size_type>(coords.y * (height - 1)) % height;
				return sample(x, y);
			}

			inline explicit operator bool() const { return data != nullptr; }
		};
	};
}
