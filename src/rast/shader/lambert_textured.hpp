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
			struct _4input {
				math::_4fvec3 normal;
				math::_4fvec2 uv;
			};
			using output = outputs::discardable<color::rgba8>;
			using _4output = math::_4vec4<uint8_t>;

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
				//using arm = math::fixed_point_arithmetic<uint32_t, 16>;

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
			inline static constexpr _4output shade(const _4input& frag, const uniform_buffer& uniforms) {
				math::_4fvec3 N = frag.normal.normalized();
				math::_4fscalar nl = math::dot(N, math::_4fvec3(uniforms.light_direction.x, uniforms.light_direction.y, uniforms.light_direction.z));
				math::_4fvec4 color = math::_4fvec4(1.0f, 1.0f, 1.0f, 1.0f);
				color.r() *= (nl * math::_4fscalar(uniforms.light_color.x) + math::_4fscalar(uniforms.ambient.x));
				color.g() *= (nl * math::_4fscalar(uniforms.light_color.y) + math::_4fscalar(uniforms.ambient.y));
				color.b() *= (nl * math::_4fscalar(uniforms.light_color.z) + math::_4fscalar(uniforms.ambient.z));
				return _4output(
					math::cast<uint8_t>(math::clamp(color.r(), math::_4fscalar(0.0f), math::_4fscalar(1.0f)) * math::_4fscalar(std::numeric_limits<uint8_t>::max())),
					math::cast<uint8_t>(math::clamp(color.g(), math::_4fscalar(0.0f), math::_4fscalar(1.0f)) * math::_4fscalar(std::numeric_limits<uint8_t>::max())),
					math::cast<uint8_t>(math::clamp(color.b(), math::_4fscalar(0.0f), math::_4fscalar(1.0f)) * math::_4fscalar(std::numeric_limits<uint8_t>::max())),
					math::cast<uint8_t>(math::clamp(color.a(), math::_4fscalar(0.0f), math::_4fscalar(1.0f)) * math::_4fscalar(std::numeric_limits<uint8_t>::max()))
				);
			}

			inline static constexpr bool _4shade_test() {
				_4input frag = {
					math::_4fvec3(0.0f, 1.0f, 0.0f),
					math::_4fvec2(0.0f, 0.0f)
				};
				uniform_buffer ubo = {
					sampler<rast::color::rgba8>(),
					glm::vec3(0.0f, 1.0f, 0.0f),
					glm::vec3(1.0f), glm::vec3(0.0f)
				};
				auto res = shade(frag, ubo);
				return res.r()[0] == res.r()[1];
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
	static_assert(lambert_textured::fragment::_4shade_test());
}
