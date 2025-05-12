#pragma once
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "shader_macros.hpp"
#include "../color.hpp"
#include "../texture.hpp"

namespace rast::shader::deferred {
	class first_pass {
	public:
		inline static glm::mat4 M = glm::mat4(1.0f);
		inline static glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		inline static glm::mat4 P = glm::perspective(glm::radians(70.0f), 16.0f / 9.0f, 0.1f, 100.0f);

		class fragment {
		public:
			inline static texture<color::rgba8>::sampler texture;

			using input = inputs::position_normal_uv;
			class output {
			public:
				glm::vec3 position;
				glm::vec3 normal;
				color::rgba8 albedo;
			};

			inline static output shade(const input& frag) {
				return {
					frag.position,
					frag.normal,
					texture.sample(frag.uv)
				};
			}

			inline static input interpolate(
				const input& frag0,
				const input& frag1,
				const input& frag2,
				const glm::vec3& coefs
			) {
				glm::vec2 new_uv = (frag0.uv * coefs.x) + (frag1.uv * coefs.y) + (frag2.uv * coefs.z);
				glm::vec3 new_normal = (frag0.normal * coefs.x) + (frag1.normal * coefs.y) + (frag2.normal * coefs.z);
				glm::vec3 new_position = (frag0.position * coefs.x) + (frag1.position * coefs.y) + (frag2.position * coefs.z);
				return { new_position, new_normal, new_uv };
			}

			rast_shader_fragment_shade()
		};


		class vertex {
		public:
			class input {
			public:
				glm::vec3 position;
				glm::vec3 normal;
				glm::vec2 uv;
			};

			using output = vertex_shader_output<first_pass>;

			inline static output shade(const input& vert) {
				return { P * V * M * glm::vec4(vert.position, 1.0f), {vert.position, vert.normal, vert.uv} };
			}
		};
	};

	class second_pass {
	public:
		class fragment {
		public:

			inline static texture<first_pass::fragment::output>::sampler texture;
			inline static glm::vec3 light_direction = glm::normalize(glm::vec3(1.0f, 3.0f, 2.0f));
			inline static color::rgb8 ambient = color::rgb8(5, 5, 5);

			using input = glm::vec2;
			using output = color::rgba8;

			inline static output shade(const input& frag) {
				first_pass::fragment::output F = texture.sample(frag);
				glm::vec3 N = glm::normalize(F.normal);
				float nl = std::clamp(glm::dot(N, light_direction), 0.0f, 1.0f);
				return color::rgba8(F.albedo.r * nl + ambient.r, F.albedo.g * nl + ambient.g, F.albedo.b * nl + ambient.b, 255);
			}

			inline static input interpolate(
				const input& frag0,
				const input& frag1,
				const input& frag2,
				const glm::vec3& coefs
			) {
				return (frag0 * coefs.x) + (frag1 * coefs.y) + (frag2 * coefs.z);
			}

			rast_shader_fragment_shade()
		};


		class vertex {
		public:
			using input = inputs::position_uv;

			using output = vertex_shader_output<second_pass>;

			inline static output shade(const input& vert) {
				return { glm::vec4(vert.position, 1.0f), {vert.uv} };
			}
		};
	};
}
