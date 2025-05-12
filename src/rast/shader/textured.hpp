#pragma once
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "../color.hpp"
#include "../texture.hpp"
#include "shader_macros.hpp"

namespace rast::shader {
	class textured {
	public:
		inline static glm::mat4 M = glm::mat4(1.0f);
		inline static glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		inline static glm::mat4 P = glm::perspective(glm::radians(70.0f), 16.0f / 9.0f, 0.1f, 100.0f);

		class fragment {
		public:

			inline static texture<rast::color::rgba8>::sampler texture;

			using input = inputs::uv;
			using output = color::rgba8;

			inline static color::rgba8 shade(const input& frag) {
				if (texture) return texture.sample(frag);
				else return color::rgba8(255, 0, 255, 255);
			}

			inline static input interpolate(
				const input& frag0,
				const input& frag1,
				const input& frag2,
				const glm::vec3& coefs
			) {
				glm::vec2 new_uv = (frag0 * coefs.x) + (frag1 * coefs.y) + (frag2 * coefs.z);
				return { new_uv };
			}

			rast_shader_fragment_shade()
		};


		class vertex {
		public:
			class input {
			public:
				glm::vec3 position;
				glm::vec2 uv;
			};

			using output = vertex_shader_output<textured>;

			inline static output shade(const input& vert) {
				return { P * V * M * glm::vec4(vert.position, 1.0f), {vert.uv} };
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
	};
}
