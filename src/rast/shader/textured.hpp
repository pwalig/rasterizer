#pragma once
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "../color.hpp"
#include "../texture.hpp"
#include "shader_macros.hpp"

namespace rast::shader {
	struct textured {
		struct fragment {
			using input = inputs::uv;
			using output = color::rgba8;

			struct uniform_buffer {
				texture<rast::color::rgba8>::sampler texture;
			};

			inline static color::rgba8 shade(const input& frag, const uniform_buffer& uniforms) {
				if (uniforms.texture) return uniforms.texture.sample(frag);
				else return color::rgba8(255, 0, 255, 255);
			}
		};


		struct vertex {
			using input = inputs::position_uv;
			using output = vertex_shader_output<textured>;

			struct uniform_buffer {
				glm::mat4 M = glm::mat4(1.0f);
				glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				glm::mat4 P = glm::perspective(glm::radians(70.0f), 16.0f / 9.0f, 0.1f, 100.0f);
			};

			inline static output shade(const input& vert, const uniform_buffer& uniforms) {
				return { uniforms.P * uniforms.V * uniforms.M * glm::vec4(vert.position, 1.0f), {vert.uv} };
			}

			template <typename posIter, typename uvIter, typename outIter>
			inline static void format(
				posIter posBegin, posIter posEnd,
				uvIter uvBegin, uvIter uvEnd,
				outIter outBegin
			) {
				while (posBegin != posEnd && uvBegin != uvEnd) {
					outBegin->position = *posBegin;
					outBegin->uv = *uvBegin;
					++posBegin;
					++uvBegin;
					++outBegin;
				}
			}
		};

		rast_shader_uniform_buffer()
	};
}
