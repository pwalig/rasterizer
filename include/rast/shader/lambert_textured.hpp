#pragma once
#include <algorithm>

#include <glm/glm.hpp>

#include "shader_macros.hpp"
#include "../convert.hpp"
#include "../color.hpp"
#include "../sampler.hpp"
#include "../simd/glm.hpp"

namespace rast::shader {
	struct lambert_textured {
		struct uniform_buffer {
			glm::mat4 PVM;
			sampler<rast::color::rgba8> texture;
			sampler<rast::color::rgba8> normal;
		};

		template <size_t Count>
		struct simd_global_uniforms {
			inline static simd::glm::vec3<Count> light_direction = glm::normalize(glm::vec3(1.0f, 3.0f, 2.0f));
			inline static simd::glm::vec3<Count> light_color = glm::vec3(1.0f);
			inline static simd::glm::vec3<Count> ambient = glm::vec3(0.1f);
		};
		struct global_uniforms {
			inline static glm::vec3 light_direction = glm::normalize(glm::vec3(1.0f, 3.0f, 2.0f));
			inline static glm::vec3 light_color = glm::vec3(1.0f);
			inline static glm::vec3 ambient = glm::vec3(0.1f);
		};

		struct fragment {
			inline static bool linear = true;
			inline static bool linear_mipmap = true;
			inline static bool mipmap = true;
			inline static uint32_t mip_to_sample = 0;
			template <size_t Count>
			struct simd_input {
				simd::glm::vec3<Count> normal;
				simd::glm::vec2<Count> uv;
				friend inline simd_input operator* (const simd_input& rhs, simd::f32x_<Count> lhs) {
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
				inline simd_input<Count> vectorize() const {
					return {
						simd::glm::vec3<Count>(normal),
						simd::glm::vec2<Count>(uv)
					};
				}
			};
			using output = outputs::discardable<glm::vec4>;
			template <size_t Count>
			using simd_output = outputs::simd_discardable<simd::glm::vec4<Count>, Count>;

			inline static float alpha_clip_threshold = 0.25f;

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
			template <size_t Count, size_t Dim = 4>
			inline static glm::vec<Dim, simd::f32x_<Count>> color_vectorizer(const glm::vec<Dim, uint8_t>* dptr, simd::u32x_<Count> off) {
				static_assert((Count == 4) || (Count == 8));
				auto max_u8x = simd::f32x_<Count>(255.0f);
				if constexpr (Count == 4) {
					if constexpr (Dim == 2) return simd::glm::vec4<Count>(
						simd::make_x4<float>(dptr[off[3]].r, dptr[off[2]].r, dptr[off[1]].r, dptr[off[0]].r),
						simd::make_x4<float>(dptr[off[3]].g, dptr[off[2]].g, dptr[off[1]].g, dptr[off[0]].g)
					) / max_u8x;
					if constexpr (Dim == 3) return simd::glm::vec4<Count>(
						simd::make_x4<float>(dptr[off[3]].r, dptr[off[2]].r, dptr[off[1]].r, dptr[off[0]].r),
						simd::make_x4<float>(dptr[off[3]].g, dptr[off[2]].g, dptr[off[1]].g, dptr[off[0]].g),
						simd::make_x4<float>(dptr[off[3]].b, dptr[off[2]].b, dptr[off[1]].b, dptr[off[0]].b)
					) / max_u8x;
					if constexpr (Dim == 4) return simd::glm::vec4<Count>(
						simd::make_x4<float>(dptr[off[3]].r, dptr[off[2]].r, dptr[off[1]].r, dptr[off[0]].r),
						simd::make_x4<float>(dptr[off[3]].g, dptr[off[2]].g, dptr[off[1]].g, dptr[off[0]].g),
						simd::make_x4<float>(dptr[off[3]].b, dptr[off[2]].b, dptr[off[1]].b, dptr[off[0]].b),
						simd::make_x4<float>(dptr[off[3]].a, dptr[off[2]].a, dptr[off[1]].a, dptr[off[0]].a)
					) / max_u8x;
				}
				else if constexpr (Count == 8) {
					return simd::glm::vec4<Count>(
						simd::make_x8<float>(dptr[off[7]].r, dptr[off[6]].r, dptr[off[5]].r, dptr[off[4]].r, dptr[off[3]].r, dptr[off[2]].r, dptr[off[1]].r, dptr[off[0]].r),
						simd::make_x8<float>(dptr[off[7]].g, dptr[off[6]].g, dptr[off[5]].g, dptr[off[4]].g, dptr[off[3]].g, dptr[off[2]].g, dptr[off[1]].g, dptr[off[0]].g),
						simd::make_x8<float>(dptr[off[7]].b, dptr[off[6]].b, dptr[off[5]].b, dptr[off[4]].b, dptr[off[3]].b, dptr[off[2]].b, dptr[off[1]].b, dptr[off[0]].b),
						simd::make_x8<float>(dptr[off[7]].a, dptr[off[6]].a, dptr[off[5]].a, dptr[off[4]].a, dptr[off[3]].a, dptr[off[2]].a, dptr[off[1]].a, dptr[off[0]].a)
					) / max_u8x;
				}
			}
			
			inline static color::rgba8 blend(glm::vec4 src, color::rgba8 dst) {
				glm::vec4 fdst = convert::uint_to_f01<uint8_t, float>(dst);
				glm::vec4 color = (src * src.a) + (fdst * (1.0f - src.a));
				return convert::f01_to_uint<float, uint8_t>(color);
			}
			template <size_t Count>
			inline static std::array<color::rgba8, Count> simd_f32_to_u8(
				simd::glm::vec4<Count> colors
			) {
				using f32 = simd::f32x_<Count>;
				f32 max_u8x = simd::cvt<float>(simd::i32x_<Count>(
					static_cast<int32_t>(std::numeric_limits<uint8_t>::max())
				));
				f32 zero = simd::setzero<float, Count>();
				f32 one = f32(1.0f);
				colors = glm::vec<4, f32>(
					simd::clamp(colors.r, zero, one) * max_u8x,
					simd::clamp(colors.g, zero, one) * max_u8x,
					simd::clamp(colors.b, zero, one) * max_u8x,
					simd::clamp(colors.a, zero, one) * max_u8x
				);
				auto res = std::array<color::rgba8, Count>();
				for (int i = 0; i < static_cast<int>(Count); ++i) res[i] = color::rgba8(colors.r[i], colors.g[i], colors.b[i], colors.a[i]);
				return res;
			}
			template <size_t Count>
			inline static simd::glm::vec4<Count> simd_u8_to_f32(
				std::array<color::rgba8, Count> colors
			) {
				simd::f32x_<Count> max_u8x = simd::cvt<float>(simd::i32x_<Count>(
					static_cast<int32_t>(std::numeric_limits<uint8_t>::max())
				));
				static_assert((Count == 4) || (Count == 8));
				if constexpr (Count == 4) return simd::glm::vec4<Count>(
					simd::make_x4<float>(colors[3].r, colors[2].r, colors[1].r, colors[0].r) / max_u8x,
					simd::make_x4<float>(colors[3].g, colors[2].g, colors[1].g, colors[0].g) / max_u8x,
					simd::make_x4<float>(colors[3].b, colors[2].b, colors[1].b, colors[0].b) / max_u8x,
					simd::make_x4<float>(colors[3].a, colors[2].a, colors[1].a, colors[0].a) / max_u8x
				);
				if constexpr (Count == 8) return simd::glm::vec4<Count>(
					simd::make_x8<float>(colors[7].r, colors[6].r, colors[5].r, colors[4].r, colors[3].r, colors[2].r, colors[1].r, colors[0].r) / max_u8x,
					simd::make_x8<float>(colors[7].g, colors[6].g, colors[5].g, colors[4].g, colors[3].g, colors[2].g, colors[1].g, colors[0].g) / max_u8x,
					simd::make_x8<float>(colors[7].b, colors[6].b, colors[5].b, colors[4].b, colors[3].b, colors[2].b, colors[1].b, colors[0].b) / max_u8x,
					simd::make_x8<float>(colors[7].a, colors[6].a, colors[5].a, colors[4].a, colors[3].a, colors[2].a, colors[1].a, colors[0].a) / max_u8x
				);
			}
			template <size_t Count>
			inline static std::array<color::rgba8, Count> simd_blend(
				simd::glm::vec4<Count> src,
				std::array<color::rgba8, Count> dst
			) {
				auto fdst = simd_u8_to_f32<Count>(dst);
				simd::glm::vec4<Count> color = (src * src.a) + (fdst * (simd::f32x_<Count>(1.0f) - src.a));
				return simd_f32_to_u8(color);
			}

			inline static rast::color::rgba8 shade(const input& frag, const uniform_buffer& uniforms) {
				using world = global_uniforms;
				glm::vec3 N = glm::normalize(frag.normal);
				float nl = std::clamp(glm::dot(N, world::light_direction), 0.0f, 1.0f);
				glm::vec4 color;
				if (uniforms.texture) {
					//color = convert::uint_to_f01<uint8_t, float>(uniforms.texture.sample_nearest(frag.uv.x, frag.uv.y));
					if (linear) color = uniforms.texture.template sample_linear<color_interpolator>(frag.uv.x, frag.uv.y, mip_to_sample);
					else color = convert::uint_to_f01<uint8_t, float>(uniforms.texture.sample_nearest(frag.uv.x, frag.uv.y, mip_to_sample));
				}
				else color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
				color.r *= (nl * world::light_color.r + world::ambient.r);
				color.g *= (nl * world::light_color.g + world::ambient.g);
				color.b *= (nl * world::light_color.b + world::ambient.b);
				color.a = 1.0f;
				return convert::f01_to_uint<float, uint8_t>(color);
			}
			inline static output shade1(const input& frag, const uniform_buffer& uniforms) {
				using world = global_uniforms;
				glm::vec3 N = glm::normalize(frag.normal);
				float nl = std::clamp(glm::dot(N, world::light_direction), 0.0f, 1.0f);
				glm::vec4 color;
				if (uniforms.texture) {
					//color = convert::uint_to_f01<uint8_t, float>(uniforms.texture.sample_nearest(frag.uv.x, frag.uv.y));
					if (linear) color = uniforms.texture.template sample_linear<color_interpolator>(frag.uv.x, frag.uv.y, mip_to_sample);
					else color = convert::uint_to_f01<uint8_t, float>(uniforms.texture.sample_nearest(frag.uv.x, frag.uv.y, mip_to_sample));
					if (color.a <= alpha_clip_threshold) return output::discard();
				}
				else color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
				color.r *= (nl * world::light_color.r + world::ambient.r);
				color.g *= (nl * world::light_color.g + world::ambient.g);
				color.b *= (nl * world::light_color.b + world::ambient.b);
				return color;
			}
			template <size_t Count>
			inline static simd_output<Count> simd_shade(
				const simd_input<Count>& frag,
				const uniform_buffer& uniforms
			) {
				using f32 = simd::f32x_<Count>;
				using world = simd_global_uniforms<Count>;
				simd::glm::vec3<Count> N = simd::glm::normalize<3, Count>(frag.normal);
				f32 nl = simd::clamp(
					simd::glm::dot(N, world::light_direction),
					f32(0.0f), f32(1.0f)
				);
				simd::glm::vec4<Count> color;
				if (mipmap) {
					if (linear_mipmap) {
						if (linear) color = uniforms.texture.template sample<&color_vectorizer<Count, 4>, rast::mag_filter::linear, rast::min_filter::linear_mipmap_linear>(frag.uv.x, frag.uv.y);
						else color = uniforms.texture.template sample<&color_vectorizer<Count, 4>, rast::mag_filter::nearest, rast::min_filter::linear_mipmap_nearest>(frag.uv.x, frag.uv.y);
					}
					else {
						if (linear) color = uniforms.texture.template sample<&color_vectorizer<Count, 4>, rast::mag_filter::linear, rast::min_filter::nearest_mipmap_linear>(frag.uv.x, frag.uv.y);
						else color = uniforms.texture.template sample<&color_vectorizer<Count, 4>, rast::mag_filter::nearest, rast::min_filter::nearest_mipmap_nearest>(frag.uv.x, frag.uv.y);
					}
				}
				else {
					if (linear) color = uniforms.texture.template sample<&color_vectorizer<Count, 4>, rast::mag_filter::linear, rast::min_filter::linear>(frag.uv.x, frag.uv.y);
					else color = uniforms.texture.template sample<&color_vectorizer<Count, 4>, rast::mag_filter::nearest, rast::min_filter::nearest>(frag.uv.x, frag.uv.y);
				}
				color.r *= simd::fmadd(nl, world::light_color.r, world::ambient.r);
				color.g *= simd::fmadd(nl, world::light_color.g, world::ambient.g);
				color.b *= simd::fmadd(nl, world::light_color.b, world::ambient.b);
				auto discard_mask = simd::reinterpret<int, Count>(color.a > simd::f32x_<Count>(0.8f));
				color.a = simd::f32x_<Count>(1.0f);
				return simd_output<Count>(std::move(color), discard_mask);
			}
		};

		struct vertex {
			using input = inputs::position_normal_uv;
			using output = vertex_shader_output<lambert_textured>;

			inline static output shade(const input& vert, const uniform_buffer& uniforms) {
				return { uniforms.PVM * glm::vec4(vert.position, 1.0f), {vert.normal, vert.uv} };
			}
		};
	};
}
