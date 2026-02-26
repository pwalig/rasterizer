#pragma once

#include "framebuffer/vectorized.hpp"
#include "shader/lambert_textured.hpp"
#include "raster/vbbox_scan.hpp"
#include "image.hpp"

namespace rast::static_test {
	namespace color {
		using rgba8 = math::u8vec4x1;
	}
	namespace shader1 {
		template <size_t Count>
		using input_x = math::f32vec4x<Count>;

		struct rast_input {
			struct input {
				glm::vec4 color;

				template <size_t Count>
				inline constexpr input_x<Count> vectorize() const {
					return math::vectorize<4, Count>(color);
				}
			};
			input data;
			glm::vec4 rastPos;
		};

		template <size_t Count>
		inline constexpr math::f32vec4x<Count> shade(const input_x<Count>& frag) {
			return frag;
			//auto max_u8x = math::f32x<Count>(std::numeric_limits<uint8_t>::max());
			//return math::u8vec4x<Count>(
			//	math::cast<uint8_t>(math::clamp(frag.r(), math::f32x<Count>(0.0f), math::f32x<Count>(1.0f)) * max_u8x),
			//	math::cast<uint8_t>(math::clamp(frag.g(), math::f32x<Count>(0.0f), math::f32x<Count>(1.0f)) * max_u8x),
			//	math::cast<uint8_t>(math::clamp(frag.b(), math::f32x<Count>(0.0f), math::f32x<Count>(1.0f)) * max_u8x),
			//	math::cast<uint8_t>(math::clamp(frag.a(), math::f32x<Count>(0.0f), math::f32x<Count>(1.0f)) * max_u8x)
			//);
		}
	};
	template <size_t Count>
	inline constexpr math::u8vec4x<Count> pass_blend(const math::f32vec4x<Count>&, const math::u8vec4x<Count>&) {
		return math::u8vec4x<Count>(
			math::u8x<Count>(255),
			math::u8x<Count>(255),
			math::u8x<Count>(255),
			math::u8x<Count>(255)
		);
	}

	template <size_t Count>
	inline constexpr math::u8vec4x<Count> blend(const math::f32vec4x<Count>& src, const math::u8vec4x<Count>& dst) {
		math::f32x<Count> max_u8x = math::f32x<Count>(static_cast<float>(std::numeric_limits<uint8_t>::max()));
		math::f32vec4x<Count> color = (src * src.a()) + ((math::cast<float>(dst) / max_u8x) * (math::vectorize<Count>(1.0f) - src.a()));
		return math::u8vec4x<Count>(
			math::cast<uint8_t>(math::clamp(color.r(), math::f32x<Count>(0.0f), math::f32x<Count>(1.0f)) * max_u8x),
			math::cast<uint8_t>(math::clamp(color.g(), math::f32x<Count>(0.0f), math::f32x<Count>(1.0f)) * max_u8x),
			math::cast<uint8_t>(math::clamp(color.b(), math::f32x<Count>(0.0f), math::f32x<Count>(1.0f)) * max_u8x),
			math::cast<uint8_t>(math::clamp(color.a(), math::f32x<Count>(0.0f), math::f32x<Count>(1.0f)) * max_u8x)
		);
	}

	inline constexpr std::pair<std::array<math::u8vec4x1, 16>, std::array<uint32_t, 16>> render() {
		auto clear_color = color::rgba8(0, 0, 0, 255);
		std::array<math::u8vec4x1, 16> color_data = {
			clear_color, clear_color, clear_color, clear_color, 
			clear_color, clear_color, clear_color, clear_color, 
			clear_color, clear_color, clear_color, clear_color, 
			clear_color, clear_color, clear_color, clear_color
		};
		uint32_t clear_depth = std::numeric_limits<uint32_t>::max();
		std::array<uint32_t, 16> depth_data = {
			clear_depth, clear_depth, clear_depth, clear_depth, 
			clear_depth, clear_depth, clear_depth, clear_depth, 
			clear_depth, clear_depth, clear_depth, clear_depth, 
			clear_depth, clear_depth, clear_depth, clear_depth
		};

		auto framebuff = framebuffer::vectorized<
			color::rgba8, uint32_t, shader1::shade<4>, depth_test::less<math::u32x4>, blend<4>
		>(color_data.data(), depth_data.data(), 4, 4);
		auto v0 = shader1::input_x<4>(
			math::vectorize<4>(0.0f),
			math::vectorize<4>(0.0f),
			math::vectorize<4>(0.0f),
			math::vectorize<4>(1.0f)
		);
		auto v1 = shader1::input_x<4>(
			math::vectorize<4>(0.5f),
			math::vectorize<4>(0.5f),
			math::vectorize<4>(0.5f),
			math::vectorize<4>(1.0f)
		);
		auto v2 = shader1::input_x<4>(
			math::vectorize<4>(1.0f),
			math::vectorize<4>(1.0f),
			math::vectorize<4>(1.0f),
			math::vectorize<4>(1.0f)
		);
		auto z = math::vectorize<4>(math::f32vec3x1(0.0f, 0.0f, 0.0f));
		auto w = math::vectorize<4>(math::f32vec3x1(1.0f, 1.0f, 1.0f));
		auto partial_coefs = math::f32vec3x4(
			math::make_f32x4(1.0f, 0.0f, 0.0f, 1.0f),
			math::make_f32x4(0.0f, 1.0f, 0.0f, 1.0f),
			math::make_f32x4(0.0f, 0.0f, 1.0f, 1.0f)
		);
		auto x = math::make_u32x4(0, 1, 0, 1);
		auto y = math::make_u32x4(0, 0, 1, 1);

		framebuff(x, y, v0, v1, v2, z, w, partial_coefs, math::vectorize<4>(true));

		auto triangle = std::array<shader1::rast_input, 3>();
		triangle[0].rastPos = glm::vec4(0.7f, 0.5f, 0.0f, 1.0f);
		triangle[1].rastPos = glm::vec4(0.2f, 0.5f, 0.0f, 1.0f);
		triangle[2].rastPos = glm::vec4(0.7f, 0.2f, 0.0f, 1.0f);
		triangle[0].data.color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		triangle[1].data.color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
		triangle[2].data.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

		raster::vbbox_scan::rasterize_one<4>(framebuff, triangle.data(), rast::viewport(0, 0, 4, 4), rast::tile(0, 0, 4, 4));

		return std::make_pair(color_data, depth_data);
	}
	constexpr auto buffs = render();
	constexpr auto color_image = image<math::u8vec4x1>::const_view(buffs.first.data(), 4, 4);
	constexpr auto depth_image = image<uint32_t>::const_view(buffs.second.data(), 4, 4);
	//static_assert(math::and_accross(color_image.at(0, 0) == math::u8vec4x1(0, 0, 0, 255)));
	//static_assert(math::and_accross(color_image.at(1, 0) == math::u8vec4x1(127, 127, 127, 255)));
	//static_assert(math::and_accross(color_image.at(0, 1) == math::u8vec4x1(255, 255, 255, 255)));
	//static_assert(math::and_accross(color_image.at(1, 1) == math::u8vec4x1(127, 127, 127, 255)));

	//static_assert(math::and_accross(color_image.at(2, 0) == math::u8vec4x1(0, 0, 0, 255)));
	//static_assert(math::and_accross(color_image.at(3, 0) == math::u8vec4x1(0, 0, 0, 255)));
	//static_assert(math::and_accross(color_image.at(2, 1) == math::u8vec4x1(0, 0, 0, 255)));
	//static_assert(math::and_accross(color_image.at(3, 1) == math::u8vec4x1(0, 0, 0, 255)));

	//static_assert(depth_image.at(0, 0) == std::numeric_limits<uint32_t>::max() / 2 + 1);
	//static_assert(depth_image.at(1, 0) == std::numeric_limits<uint32_t>::max() / 2 + 1);
	//static_assert(depth_image.at(0, 1) == std::numeric_limits<uint32_t>::max() / 2 + 1);
	//static_assert(depth_image.at(1, 1) == std::numeric_limits<uint32_t>::max() / 2 + 1);
}
