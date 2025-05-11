#pragma once
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "../color.hpp"
#include "../texture.hpp"

namespace rast::shader {
	class lambert_textured {
	public:
		inline static glm::mat4 M = glm::mat4(1.0f);
		inline static glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		inline static glm::mat4 P = glm::perspective(glm::radians(70.0f), 16.0f / 9.0f, 0.1f, 100.0f);

		class fragment {
		public:

			inline static texture<rast::color::rgba8>::sampler texture;
			inline static glm::vec3 light_direction = glm::normalize(glm::vec3(1.0f, 3.0f, 2.0f));
			inline static color::rgb8 ambient = color::rgb8(5, 5, 5);

			class input {
			public:
				glm::vec3 normal;
				glm::vec2 uv;
			};

			inline static color::rgba8 shade(const input& frag) {
				glm::vec3 N = glm::normalize(frag.normal);
				float nl = std::clamp(glm::dot(N, light_direction), 0.0f, 1.0f);
				color::rgba8 color;
				if (texture) color = texture.sample(frag.uv);
				else color = color::rgba8(255, 0, 255, 255);
				return color::rgba8(color.r * nl + ambient.r, color.g * nl + ambient.g, color.b * nl + ambient.b, color.a);
			}

			inline static input interpolate(
				const input& frag0,
				const input& frag1,
				const input& frag2,
				const glm::vec3& coefs
			) {
				glm::vec2 new_uv = (frag0.uv * coefs.x) + (frag1.uv * coefs.y) + (frag2.uv * coefs.z);
				glm::vec3 new_normal = (frag0.normal * coefs.x) + (frag1.normal * coefs.y) + (frag2.normal * coefs.z);
				return { new_normal, new_uv };
			}
		};


		class vertex {
		public:
			class input {
			public:
				glm::vec3 position;
				glm::vec3 normal;
				glm::vec2 uv;
			};

			class output {
			public:
				glm::vec4 rastPos;
				fragment::input data;
			};

			inline static output shade(const input& vert) {
				return { P * V * M * glm::vec4(vert.position, 1.0f), {vert.normal, vert.uv} };
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
	};
}
