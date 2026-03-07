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
#include "../math/sse.hpp"

namespace rast::shader {
	struct lambert_textured {
		struct fragment {
			inline static bool linear = false;
			inline static uint32_t mip_to_sample = 0;
			template <size_t Count>
			struct input_x {
				math::f32vec3x<Count> normal;
				math::f32vec2x<Count> uv;
				friend inline input_x operator* (const input_x& rhs, math::f32x<Count> lhs) {
					return { rhs.normal * lhs, rhs.uv * lhs };
				}
				friend inline input_x operator+ (const input_x& rhs, const input_x& lhs) {
					return { rhs.normal + lhs.normal, rhs.uv + lhs.uv };
				}
			};
			template <size_t Count>
			struct simd_input {
				glm::vec<3, math::simd::f32x_<Count>> normal;
				glm::vec<2, math::simd::f32x_<Count>> uv;
				friend inline simd_input operator* (const simd_input& rhs, math::simd::f32x_<Count> lhs) {
					return { rhs.normal * lhs, rhs.uv * lhs };
				}
				friend inline simd_input operator+ (const simd_input& rhs, const simd_input& lhs) {
					return { rhs.normal + lhs.normal, rhs.uv + lhs.uv };
				}
			};
			struct input {
			public:
				glm::vec3 normal;
				glm::vec2 uv;
				friend inline input operator* (const input& rhs, float lhs) {
					return { rhs.normal * lhs, rhs.uv * lhs };
				}
				friend inline input operator+ (const input& rhs, const input& lhs) {
					return { rhs.normal + lhs.normal, rhs.uv + lhs.uv };
				}
				template <size_t Count>
				inline constexpr simd_input<Count> vectorize() const {
					return {
						glm::vec<3, math::simd::f32x_<Count>>(glm::normalize(normal)),
						glm::vec<2, math::simd::f32x_<Count>>(uv)
					};
				}
			};
			using output = outputs::discardable<glm::vec4>;
			using output_x4 = math::vec4x4<uint8_t>;

			inline static float alpha_clip_threshold = 0.25f;

			struct uniform_buffer {
				sampler<rast::color::rgba8> texture;
				glm::vec3 light_direction = glm::normalize(glm::vec3(1.0f, 3.0f, 2.0f));
				glm::vec3 light_color = glm::vec3(1.0f);
				glm::vec3 ambient = glm::vec3(0.1f);
			};
			template <size_t Count>
			struct simd_uniform_buffer {
				sampler<rast::color::rgba8> texture;
				glm::vec<3, math::simd::f32x_<Count>> light_direction = glm::normalize(glm::vec3(1.0f, 3.0f, 2.0f));
				glm::vec<3, math::simd::f32x_<Count>> light_color = glm::vec3(1.0f);
				glm::vec<3, math::simd::f32x_<Count>> ambient = glm::vec3(0.1f);
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
			
			inline static color::rgba8 blend(glm::vec4 src, color::rgba8 dst) {
				glm::vec4 fdst = convert::uint_to_f01<uint8_t, float>(dst);
				glm::vec4 color = (src * src.a) + (fdst * (1.0f - src.a));
				return convert::f01_to_uint<float, uint8_t>(color);
			}
			template <size_t Count>
			inline static std::array<color::rgba8, Count> simd_blend(
				glm::vec<4, math::simd::f32x_<Count>> src,
				std::array<color::rgba8, Count> dst
			) {
				math::simd::f32x_<Count> max_u8x = static_cast<math::simd::f32x_<Count>>(
					math::simd::i32x_<Count>(static_cast<int32_t>(std::numeric_limits<uint8_t>::max()))
				);
				auto fdst = glm::vec<4, math::simd::f32x_<Count>>(
					math::simd::make_x4<float>(dst[3].r, dst[2].r, dst[1].r, dst[0].r) / max_u8x,
					math::simd::make_x4<float>(dst[3].g, dst[2].g, dst[1].g, dst[0].g) / max_u8x,
					math::simd::make_x4<float>(dst[3].b, dst[2].b, dst[1].b, dst[0].b) / max_u8x,
					math::simd::make_x4<float>(dst[3].a, dst[2].a, dst[1].a, dst[0].a) / max_u8x
				);
				glm::vec<4, math::simd::f32x_<Count>> color = (src * src.a) + (fdst * (math::simd::f32x_<Count>(1.0f) - src.a));
				std::array<color::rgba8, Count> res = {
					convert::f01_to_uint<float, uint8_t>(glm::vec4(color.r[0], color.g[0], color.b[0], color.a[0])),
					convert::f01_to_uint<float, uint8_t>(glm::vec4(color.r[1], color.g[1], color.b[1], color.a[1])),
					convert::f01_to_uint<float, uint8_t>(glm::vec4(color.r[2], color.g[2], color.b[2], color.a[2])),
					convert::f01_to_uint<float, uint8_t>(glm::vec4(color.r[3], color.g[3], color.b[3], color.a[3]))
				};
				return res;
			}

			inline static output shade0(const input& frag, const uniform_buffer& uniforms) {
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
				color.r *= (nl * uniforms.light_color.r + uniforms.ambient.r);
				color.g *= (nl * uniforms.light_color.g + uniforms.ambient.g);
				color.b *= (nl * uniforms.light_color.b + uniforms.ambient.b);
				return color;
			}
			template <size_t Count>
			inline static glm::vec<4, math::simd::f32x_<Count>> simd_shade(
				const simd_input<Count>& frag,
				const simd_uniform_buffer<Count>& uniforms
			) {
				math::simd::f32x4 nl = math::simd::clamp(
					(frag.normal.x * uniforms.light_direction.x) +
					(frag.normal.y * uniforms.light_direction.y) +
					(frag.normal.z * uniforms.light_direction.z), // dot(normal, light_direction)
					math::simd::f32x4(0.0f), math::simd::f32x4(1.0f)
				);
				std::array<color::rgba8, Count> samples = uniforms.texture.sample_nearest(frag.uv.x, frag.uv.y);
				auto color = glm::vec<4, math::simd::f32x_<Count>>(
					math::simd::make_x4(static_cast<float>(samples[3].r), static_cast<float>(samples[2].r), static_cast<float>(samples[1].r), static_cast<float>(samples[0].r)),
					math::simd::make_x4(static_cast<float>(samples[3].g), static_cast<float>(samples[2].g), static_cast<float>(samples[1].g), static_cast<float>(samples[0].g)),
					math::simd::make_x4(static_cast<float>(samples[3].b), static_cast<float>(samples[2].b), static_cast<float>(samples[1].b), static_cast<float>(samples[0].b)),
					math::simd::make_x4(static_cast<float>(samples[3].a), static_cast<float>(samples[2].a), static_cast<float>(samples[1].a), static_cast<float>(samples[0].a))
				) / math::simd::f32x4(static_cast<float>(std::numeric_limits<uint8_t>::max()));
				//if (uniforms.texture) {
					//color = convert::uint_to_f01<uint8_t, float>(uniforms.texture.sample_nearest(frag.uv.x, frag.uv.y));
				//	if (linear) color = uniforms.texture.sample_linear<color_interpolator>(frag.uv.x, frag.uv.y, mip_to_sample);
				//	else color = convert::uint_to_f01<uint8_t, float>(uniforms.texture.sample_nearest(frag.uv.x, frag.uv.y, mip_to_sample));
				//	if (color.a <= alpha_clip_threshold) return output::discard();
				//}
				//else color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
				color.r *= (nl * uniforms.light_color.r + uniforms.ambient.r);
				color.g *= (nl * uniforms.light_color.g + uniforms.ambient.g);
				color.b *= (nl * uniforms.light_color.b + uniforms.ambient.b);
				return color;
			}
			template <size_t Count>
			inline static constexpr math::f32vec4x<Count> shade(const input_x<Count>& frag, const uniform_buffer& uniforms) {
				math::f32vec3x<Count> N = frag.normal.normalized();
				math::f32vec3x<Count> L = math::f32vec3x<Count>(uniforms.light_direction.x, uniforms.light_direction.y, uniforms.light_direction.z);
				math::f32x<Count> nl = math::clamp<float>(math::dot(N, L), math::f32x4(0.0f), math::f32x4(1.0f));
				auto texture_read = uniforms.texture.sample_linear_t<float, Count>(frag.uv.x(), frag.uv.y());
				auto max_u8x4 = math::f32x<Count>(static_cast<float>(std::numeric_limits<uint8_t>::max()));
				auto color = texture_read / max_u8x4;
				color.r() *= (nl * math::f32x<Count>(uniforms.light_color.x) + math::f32x<Count>(uniforms.ambient.x));
				color.g() *= (nl * math::f32x<Count>(uniforms.light_color.y) + math::f32x<Count>(uniforms.ambient.y));
				color.b() *= (nl * math::f32x<Count>(uniforms.light_color.z) + math::f32x<Count>(uniforms.ambient.z));
				return color;
				//return output_x4(
				//	math::cast<uint8_t>(math::clamp(color.r(), math::f32x4(0.0f), math::f32x4(1.0f)) * max_u8x4),
				//	math::cast<uint8_t>(math::clamp(color.g(), math::f32x4(0.0f), math::f32x4(1.0f)) * max_u8x4),
				//	math::cast<uint8_t>(math::clamp(color.b(), math::f32x4(0.0f), math::f32x4(1.0f)) * max_u8x4),
				//	math::cast<uint8_t>(math::clamp(color.a(), math::f32x4(0.0f), math::f32x4(1.0f)) * max_u8x4)
				//);
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

		//using uniform_buffer = shader_uniform_buffer<lambert_textured>;
		struct uniform_buffer {
			typename fragment::simd_uniform_buffer<4> fragment;
			typename vertex::uniform_buffer vertex;
		};
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
		constexpr lambert_textured::fragment::input_x<4> frag = {
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

		//constexpr auto res = lambert_textured::fragment::shade(frag, ubo);
		//static_assert(res.a()[0] == 255);
		//static_assert(res.a()[1] == 255);
		//static_assert(res.a()[2] == 255);
		//static_assert(res.a()[3] == 255);
		//static_assert(res.r()[0] == 0);
		//static_assert(res.r()[1] == 32);
		//static_assert(res.r()[2] == 0);
		//static_assert(res.r()[3] == 32);
	}
}
