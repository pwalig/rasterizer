#pragma once
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "../color.hpp"
#include "shader_macros.hpp"

namespace rast::shader {
	struct vertex_colored {

		struct fragment {
			struct uniform_buffer {};

			using input = glm::vec4;
			using output = color::rgba8;

			inline static color::rgba8 shade(const input& frag, const uniform_buffer& uniforms) {
				return color::rgba8(
					std::clamp(frag.x, 0.0f, 1.0f) * 255,
					std::clamp(frag.y, 0.0f, 1.0f) * 255,
					std::clamp(frag.z, 0.0f, 1.0f) * 255,
					std::clamp(frag.w, 0.0f, 1.0f) * 255
				);
			}
		};


		struct vertex {
			struct input {
				glm::vec3 position;
				glm::vec4 color;
			};
			using output = vertex_shader_output<vertex_colored>;

			using uniform_buffer = uniforms::PVM_struct;

			inline static output shade(const input& vert, const uniform_buffer& uniforms) {
				return { uniforms.PVM * glm::vec4(vert.position, 1.0f), {vert.color} };
			}
			inline static output shade(const input& vert, const uniforms::MVP& uniforms) {
				return { uniforms.P * uniforms.V * uniforms.M * glm::vec4(vert.position, 1.0f), {vert.color} };
			}
		};

		using uniform_buffer = shader_uniform_buffer<vertex_colored>;
	};
}
