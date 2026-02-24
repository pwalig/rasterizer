#pragma once

#include "framebuffer/vectorized.hpp"
#include "shader/lambert_textured.hpp"

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

	inline constexpr std::array<color::rgba8, 16> render() {
		auto clear_color = color::rgba8(0, 0, 0, 255);
		std::array<color::rgba8, 16> color_data = {
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
		auto v0 = shader1::input<1>(
			math::f32x1(1.0f),
			math::f32x1(1.0f),
			math::f32x1(1.0f),
			math::f32x1(1.0f)
		);
		auto v1 = shader1::input<1>(
			math::f32x1(1.0f),
			math::f32x1(1.0f),
			math::f32x1(1.0f),
			math::f32x1(1.0f)
		);
		auto v2 = shader1::input<1>(
			math::f32x1(1.0f),
			math::f32x1(1.0f),
			math::f32x1(1.0f),
			math::f32x1(1.0f)
		);
		auto partial_coefs = math::f32vec3x1(
			math::f32x1(1.0f),
			math::f32x1(1.0f),
			math::f32x1(1.0f)
		);
		framebuff.draw<shader1::shade<1>, depth_test::less<math::u32x1>>(
			math::u32x1(static_cast<uint32_t>(0)), math::u32x1(static_cast<uint32_t>(0)),
			v0, v1, v2, partial_coefs
		);
		return color_data;
	}
	constexpr auto rendered = render();
	static_assert(math::and_accross(rendered[0] == math::u8vec4x1(255, 255, 255, 255)));
	static_assert(math::and_accross(rendered[1] == math::u8vec4x1(0, 0, 0, 255)));
}
