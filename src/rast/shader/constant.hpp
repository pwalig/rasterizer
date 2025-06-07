#pragma once
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "../color.hpp"
#include "shader_macros.hpp"

namespace rast::shader {
	class constant {
	public:
		class fragment {
		public:
			class input { };
			using output = color::rgba8;

			struct uniform_buffer {
				color::rgba8 color;
			};

			inline static output shade(const input& frag, const uniform_buffer& uniforms) {
				return uniforms.color;
			}
		};

		class vertex {
		public:
			using input = inputs::position;
			using output = vertex_shader_output<constant>;

			struct uniform_buffer {
				glm::mat4 M = glm::mat4(1.0f);
				glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				glm::mat4 P = glm::perspective(glm::radians(70.0f), 16.0f / 9.0f, 0.1f, 100.0f);
			};

			inline static output shade(const input& vert, const uniform_buffer& uniforms) {
				return { uniforms.P * uniforms.V * uniforms.M * glm::vec4(vert, 1.0f), {} };
			}
		};

		rast_shader_uniform_buffer()
	};
}
