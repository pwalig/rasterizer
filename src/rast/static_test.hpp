#pragma once

#include "framebuffer/vectorized.hpp"
#include "shader/lambert_textured.hpp"
#include "image.hpp"

namespace rast::static_test {
	namespace color {
		using rgba8 = math::u8vec4x1;
	}
	namespace shader1 {
		template <size_t Count>
		using input = math::f32vec4x<Count>;

		template <size_t Count>
		inline constexpr math::u8vec4x<Count> shade(const input<Count>& frag) {
			auto max_u8x = math::f32x<Count>(std::numeric_limits<uint8_t>::max());
			return math::u8vec4x<Count>(
				math::cast<uint8_t>(math::clamp(frag.r(), math::f32x<Count>(0.0f), math::f32x<Count>(1.0f)) * max_u8x),
				math::cast<uint8_t>(math::clamp(frag.g(), math::f32x<Count>(0.0f), math::f32x<Count>(1.0f)) * max_u8x),
				math::cast<uint8_t>(math::clamp(frag.b(), math::f32x<Count>(0.0f), math::f32x<Count>(1.0f)) * max_u8x),
				math::cast<uint8_t>(math::clamp(frag.a(), math::f32x<Count>(0.0f), math::f32x<Count>(1.0f)) * max_u8x)
			);
		}
	};

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

		auto framebuff = framebuffer::vectorized<color::rgba8, uint32_t>(color_data.data(), depth_data.data(), 4, 4);
		auto v0 = shader1::input<4>(
			math::vectorize<4>(0.0f),
			math::vectorize<4>(0.0f),
			math::vectorize<4>(0.0f),
			math::vectorize<4>(1.0f)
		);
		auto v1 = shader1::input<4>(
			math::vectorize<4>(0.5f),
			math::vectorize<4>(0.5f),
			math::vectorize<4>(0.5f),
			math::vectorize<4>(1.0f)
		);
		auto v2 = shader1::input<4>(
			math::vectorize<4>(1.0f),
			math::vectorize<4>(1.0f),
			math::vectorize<4>(1.0f),
			math::vectorize<4>(1.0f)
		);
		auto z = math::f32vec3x4(
			math::make_f32x4(0.0f, 0.0f, 0.0f, 0.0f),
			math::make_f32x4(0.0f, 0.0f, 0.0f, 0.0f),
			math::make_f32x4(0.0f, 0.0f, 0.0f, 0.0f)
		);
		auto w = math::f32vec3x4(
			math::make_f32x4(1.0f, 1.0f, 1.0f, 1.0f),
			math::make_f32x4(1.0f, 1.0f, 1.0f, 1.0f),
			math::make_f32x4(1.0f, 1.0f, 1.0f, 1.0f)
		);
		auto partial_coefs = math::f32vec3x4(
			math::make_f32x4(1.0f, 0.0f, 0.0f, 1.0f),
			math::make_f32x4(0.0f, 1.0f, 0.0f, 1.0f),
			math::make_f32x4(0.0f, 0.0f, 1.0f, 1.0f)
		);
		auto x = math::make_u32x4(0, 1, 0, 1);
		auto y = math::make_u32x4(0, 0, 1, 1);

		framebuff.draw<shader1::shade<4>, depth_test::less<math::u32x1>>(
			x, y, v0, v1, v2, z, w, partial_coefs
		);
		return std::make_pair(color_data, depth_data);
	}
	constexpr auto buffs = render();
	constexpr auto color_image = image<math::u8vec4x1>::const_view(buffs.first.data(), 4, 4);
	constexpr auto depth_image = image<uint32_t>::const_view(buffs.second.data(), 4, 4);
	static_assert(math::and_accross(color_image.at(0, 0) == math::u8vec4x1(0, 0, 0, 255)));
	static_assert(math::and_accross(color_image.at(1, 0) == math::u8vec4x1(127, 127, 127, 255)));
	static_assert(math::and_accross(color_image.at(0, 1) == math::u8vec4x1(255, 255, 255, 255)));
	static_assert(math::and_accross(color_image.at(1, 1) == math::u8vec4x1(127, 127, 127, 255)));
	static_assert(depth_image.at(0, 0) == std::numeric_limits<uint32_t>::max() / 2 + 1);
	static_assert(depth_image.at(1, 0) == std::numeric_limits<uint32_t>::max() / 2 + 1);
	static_assert(depth_image.at(0, 1) == std::numeric_limits<uint32_t>::max() / 2 + 1);
	static_assert(depth_image.at(1, 1) == std::numeric_limits<uint32_t>::max() / 2 + 1);
}
