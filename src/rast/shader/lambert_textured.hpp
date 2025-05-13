#pragma once
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "shader_macros.hpp"
#include "../color.hpp"
#include "../texture.hpp"

namespace rast::shader {
	class lambert_textured {
	public:
		class fragment {
		public:

			using input = inputs::normal_uv;
			using output = color::rgba8;

			class uniform_buffer {
			public:
				texture<rast::color::rgba8>::sampler texture;
				glm::vec3 light_direction = glm::normalize(glm::vec3(1.0f, 3.0f, 2.0f));
				color::rgb8 ambient = color::rgb8(5, 5, 5);
			};

			inline static output shade(const input& frag, const uniform_buffer& uniforms) {
				glm::vec3 N = glm::normalize(frag.normal);
				float nl = std::clamp(glm::dot(N, uniforms.light_direction), 0.0f, 1.0f);
				color::rgba8 color;
				if (uniforms.texture) color = uniforms.texture.sample(frag.uv);
				else color = color::rgba8(255, 0, 255, 255);
				return color::rgba8(color.r * nl + uniforms.ambient.r, color.g * nl + uniforms.ambient.g, color.b * nl + uniforms.ambient.b, color.a);
			}
		};


		class vertex {
		public:
			using input = inputs::position_normal_uv;
			using output = vertex_shader_output<lambert_textured>;

			class uniform_buffer {
			public:
				glm::mat4 M = glm::mat4(1.0f);
				glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				glm::mat4 P = glm::perspective(glm::radians(70.0f), 16.0f / 9.0f, 0.1f, 100.0f);
			};

			inline static output shade(const input& vert, const uniform_buffer& uniforms) {
				return { uniforms.P * uniforms.V * uniforms.M * glm::vec4(vert.position, 1.0f), {vert.normal, vert.uv} };
			}

			template <typename posIter, typename normIter, typename uvIter, typename outIter>
			inline static void format(
				posIter posBegin, posIter posEnd,
				normIter normBegin, normIter normEnd,
				uvIter uvBegin, uvIter uvEnd,
				outIter outBegin
			) {
				while (posBegin != posEnd && uvBegin != uvEnd) {
					outBegin->position = *posBegin;
					outBegin->normal = *normBegin;
					outBegin->uv = *uvBegin;
					++posBegin;
					++normBegin;
					++uvBegin;
					++outBegin;
				}
			}
		};

		rast_shader_uniform_buffer()
	};
}
