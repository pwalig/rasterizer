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
				glm::vec3 position;
				glm::vec3 normal;
				color::rgba8 albedo;
			};

			struct uniform_buffer {
				texture<color::rgba8>::sampler texture;
			};

			inline static output shade(const input& frag, const uniform_buffer& uniforms) {
				return {
					frag.position,
					frag.normal,
					uniforms.texture.sample(frag.uv)
				};
			}
		};

		struct vertex {
			using input = inputs::position_normal_uv;
			using output = vertex_shader_output<first_pass>;

			struct uniform_buffer {
				glm::mat4 M = glm::mat4(1.0f);
				glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				glm::mat4 P = glm::perspective(glm::radians(70.0f), 16.0f / 9.0f, 0.1f, 100.0f);
			};

			inline static output shade(const input& vert, const uniform_buffer& uniforms) {
				return {
					uniforms.P * uniforms.V * uniforms.M * glm::vec4(vert.position, 1.0f),
					{vert.position, vert.normal, vert.uv}
				};
			}
		};

		rast_shader_uniform_buffer()
	};

	struct second_pass {
		struct fragment {
			using input = glm::vec2;
			using output = color::rgba8;

			struct uniform_buffer {
				texture<first_pass::fragment::output>::sampler texture;
				glm::vec3 light_direction = glm::normalize(glm::vec3(1.0f, 3.0f, 2.0f));
				color::rgb8 ambient = color::rgb8(5, 5, 5);
			};

			inline static output shade(const input& frag, const uniform_buffer& uniforms) {
				first_pass::fragment::output F = uniforms.texture.sample(frag);
				glm::vec3 N = glm::normalize(F.normal);
				float nl = std::clamp(glm::dot(N, uniforms.light_direction), 0.0f, 1.0f);
				return color::rgba8(
					F.albedo.r * nl + uniforms.ambient.r,
					F.albedo.g * nl + uniforms.ambient.g,
					F.albedo.b * nl + uniforms.ambient.b,
					F.albedo.a
				);
			}
		};


		struct vertex {
			using input = inputs::position_uv;
			using output = vertex_shader_output<second_pass>;

			struct uniform_buffer { };

			inline static output shade(const input& vert, const uniform_buffer& uniforms) {
				return { glm::vec4(vert.position, 1.0f), {vert.uv} };
			}
		};

		rast_shader_uniform_buffer()
	};
}
