#pragma once
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "shader_macros.hpp"
#include "../color.hpp"
#include "../texture.hpp"

namespace rast::shader::deferred {
	struct first_pass {
		struct fragment {
			using input = inputs::position_normal_uv;
			struct output {
				glm::vec3 normal;
				color::rgba8 albedo;
			};

			struct uniform_buffer {
				texture<color::rgba8>::sampler texture;
			};

			inline static output shade(const input& frag, const uniform_buffer& uniforms) {
				return {
					frag.normal,
					uniforms.texture.sample(frag.uv)
				};
			}
		};

		struct vertex {
			using input = inputs::position_normal_uv;
			using output = vertex_shader_output<first_pass>;

			using uniform_buffer = uniforms::PVM_struct;

			inline static output shade(const input& vert, const uniform_buffer& uniforms) {
				return {
					uniforms.PVM * glm::vec4(vert.position, 1.0f),
					{vert.position, vert.normal, vert.uv}
				};
			}
			inline static output shade(const input& vert, const uniforms::MVP& uniforms) {
				return {
					uniforms.P * uniforms.V * uniforms.M * glm::vec4(vert.position, 1.0f),
					{vert.position, vert.normal, vert.uv}
				};
			}
		};

		using uniform_buffer = shader_uniform_buffer<first_pass>;
	};

	struct second_pass {
		struct fragment {
			using input = glm::vec2;
			using output = color::rgba8;

			struct uniform_buffer {
				texture<first_pass::fragment::output>::sampler texture;
				glm::vec3 light_direction = glm::normalize(glm::vec3(1.0f, 3.0f, 2.0f));
				glm::vec3 light_color = glm::vec3(1.0f);
				glm::vec3 ambient = glm::vec3(0.1f);
			};

			inline static output shade(const input& frag, const uniform_buffer& uniforms) {
				first_pass::fragment::output F = uniforms.texture.sample(frag);
				//return F.albedo;
				//return convert::f01_to_uint<float, uint8_t>(glm::vec4((F.normal + 1.0f) / 2.0f, 1.0f));
				glm::vec3 N = glm::normalize(F.normal);
				float nl = std::clamp(glm::dot(N, uniforms.light_direction), 0.0f, 1.0f);
				glm::vec4 color = convert::uint_to_f01<uint8_t, float>(F.albedo);
				return convert::f01_to_uint<float, uint8_t>(glm::vec4(
					color.r * (nl * uniforms.light_color.r + uniforms.ambient.r),
					color.g * (nl * uniforms.light_color.g + uniforms.ambient.g),
					color.b * (nl * uniforms.light_color.b + uniforms.ambient.b),
					color.a
				));
			}
		};


		struct vertex {
			using input = inputs::position_uv;
			using output = vertex_shader_output<second_pass>;

			struct uniform_buffer { };

			inline static output shade(const input& vert, const uniform_buffer&) {
				return { glm::vec4(vert.position, 1.0f), {vert.uv} };
			}
		};

		using uniform_buffer = shader_uniform_buffer<second_pass>;
	};
}
