#pragma once
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "shader_macros.hpp"
#include "../convert.hpp"
#include "../color.hpp"
#include "../sampler.hpp"
#include "../math/vec.hpp"

namespace rast::shader {
	struct lambert_textured {
		struct fragment {
			inline static bool linear = false;
			inline static uint32_t mip_to_sample = 0;
			using input = inputs::normal_uv;
			struct input_x4 {
				math::f32vec3x4 normal;
				math::f32vec2x4 uv;
			};
			using output = outputs::discardable<color::rgba8>;
			using output_x4 = math::vec4x4<uint8_t>;

			inline static float alpha_clip_threshold = 0.25f;

			struct uniform_buffer {
				sampler<rast::color::rgba8> texture;
				glm::vec3 light_direction = glm::normalize(glm::vec3(1.0f, 3.0f, 2.0f));
				glm::vec3 light_color = glm::vec3(1.0f);
				glm::vec3 ambient = glm::vec3(0.1f);
			};

			inline static constexpr rast::color::rgba32f color_interpolator(
				rast::color::rgba8 color0, rast::color::rgba8 color1, rast::color::rgba8 color2, rast::color::rgba8 color3,
				float coef0, float coef1, float coef2, float coef3
			) {
				return
					(convert::uint_to_f01<uint8_t, float>(color0) * coef0) +
					(convert::uint_to_f01<uint8_t, float>(color1) * coef1) +
					(convert::uint_to_f01<uint8_t, float>(color2) * coef2) +
					(convert::uint_to_f01<uint8_t, float>(color3) * coef3);
			};


			inline static output shade(const input& frag, const uniform_buffer& uniforms) {
				glm::vec3 N = glm::normalize(frag.normal);
				float nl = std::clamp(glm::dot(N, uniforms.light_direction), 0.0f, 1.0f);
				glm::vec4 color;
				if (uniforms.texture) {
					//color = convert::uint_to_f01<uint8_t, float>(uniforms.texture.sample_nearest(frag.uv.x, frag.uv.y));
					if (linear) color = uniforms.texture.sample_linear<color_interpolator>(frag.uv.x, frag.uv.y, mip_to_sample);
					else color = convert::uint_to_f01<uint8_t, float>(uniforms.texture.sample_nearest(frag.uv.x, frag.uv.y, mip_to_sample));
					if (color.a <= alpha_clip_threshold) return output::discard();
				}
				else color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
				return convert::f01_to_uint<float, uint8_t>(glm::vec4(
					color.r * (nl * uniforms.light_color.r + uniforms.ambient.r),
					color.g * (nl * uniforms.light_color.g + uniforms.ambient.g),
					color.b * (nl * uniforms.light_color.b + uniforms.ambient.b),
					color.a
				));
			}
			inline static constexpr output_x4 shade(const input_x4& frag, const uniform_buffer& uniforms) {
				math::f32vec3x4 N = frag.normal.normalized();
				math::f32vec3x4 L = math::f32vec3x4(uniforms.light_direction.x, uniforms.light_direction.y, uniforms.light_direction.z);
				math::f32x4 nl = math::clamp<float>(math::dot(N, L), math::f32x4(0.0f), math::f32x4(1.0f));
				auto texture_read = math::transpose<4>(uniforms.texture.sample_nearest_x(frag.uv.x(), frag.uv.y()));
				auto max_u8x4 = math::f32x4(static_cast<float>(std::numeric_limits<uint8_t>::max()));
				auto color = math::cast<float>(texture_read) / max_u8x4;
				color.r() *= (nl * math::f32x4(uniforms.light_color.x) + math::f32x4(uniforms.ambient.x));
				color.g() *= (nl * math::f32x4(uniforms.light_color.y) + math::f32x4(uniforms.ambient.y));
				color.b() *= (nl * math::f32x4(uniforms.light_color.z) + math::f32x4(uniforms.ambient.z));
				return output_x4(
					math::cast<uint8_t>(math::clamp(color.r(), math::f32x4(0.0f), math::f32x4(1.0f)) * max_u8x4),
					math::cast<uint8_t>(math::clamp(color.g(), math::f32x4(0.0f), math::f32x4(1.0f)) * max_u8x4),
					math::cast<uint8_t>(math::clamp(color.b(), math::f32x4(0.0f), math::f32x4(1.0f)) * max_u8x4),
					math::cast<uint8_t>(math::clamp(color.a(), math::f32x4(0.0f), math::f32x4(1.0f)) * max_u8x4)
				);
			}
		};


		struct vertex {
			using input = inputs::position_normal_uv;
			using output = vertex_shader_output<lambert_textured>;

			using uniform_buffer = uniforms::PVM_struct;

			inline static output shade(const input& vert, const uniform_buffer& uniforms) {
				return { uniforms.PVM * glm::vec4(vert.position, 1.0f), {vert.normal, vert.uv} };
			}
			inline static output shade(const input& vert, const uniforms::MVP& uniforms) {
				return { uniforms.P * uniforms.V * uniforms.M * glm::vec4(vert.position, 1.0f), {vert.normal, vert.uv} };
			}
		};

		using uniform_buffer = shader_uniform_buffer<lambert_textured>;
	};
	namespace lambert_textured_test {
		using color = rast::color::rgba8;
		constexpr color mip_data[16 + 4 + 1] = {
			color(0, 0, 0, 255), color(64, 0, 0, 255), color(128, 0, 0, 255), color(192, 0, 0, 255),
			color(0, 64, 0, 255), color(64, 64, 0, 255), color(128, 64, 0, 255), color(192, 64, 0, 255),
			color(0, 128, 0, 255), color(64, 128, 0, 255), color(128, 128, 0, 255), color(192, 128, 0, 255),
			color(0, 192, 0, 255), color(64, 192, 0, 255), color(128, 192, 0, 255), color(192, 192, 0, 255),
			color(0, 0, 0, 255), color(0, 0, 0, 255),
			color(0, 0, 0, 255), color(0, 0, 0, 255),
			color(0, 0, 0, 255)
		};
		constexpr lambert_textured::fragment::input_x4 frag = {
			math::f32vec3x4(0.0f, 1.0f, 0.0f),
			math::f32vec2x4(
				math::make_f32x4(0.125f, 0.375f, 0.125f, 0.375f),
				math::make_f32x4(0.125f, 0.125f, 0.375f, 0.375f)
			)
		};
		constexpr lambert_textured::fragment::uniform_buffer ubo = {
			sampler<rast::color::rgba8>(mip_data, 4, 4),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.5f), glm::vec3(0.0f)
		};
		constexpr auto texture_read = math::transpose<4>(ubo.texture.sample_nearest_x(frag.uv.x(), frag.uv.y()));
		static_assert(texture_read.r()[0] == 0);
		static_assert(texture_read.r()[1] == 64);
		static_assert(texture_read.r()[2] == 0);
		static_assert(texture_read.r()[3] == 64);

		constexpr auto texture_read2 = math::cast<float>(texture_read);
		static_assert(texture_read2.r()[0] == 0.0f);

		constexpr auto res = lambert_textured::fragment::shade(frag, ubo);
		static_assert(res.a()[0] == 255);
		static_assert(res.a()[1] == 255);
		static_assert(res.a()[2] == 255);
		static_assert(res.a()[3] == 255);
		static_assert(res.r()[0] == 0);
		static_assert(res.r()[1] == 32);
		static_assert(res.r()[2] == 0);
		static_assert(res.r()[3] == 32);
	}
}
