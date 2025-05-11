#pragma once
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "../color.hpp"

namespace rast::shader {
	class vertex_colored {
	public:
		inline static glm::mat4 M = glm::mat4(1.0f);
		inline static glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		inline static glm::mat4 P = glm::perspective(glm::radians(70.0f), 16.0f / 9.0f, 0.1f, 100.0f);

		class fragment {
		public:

			using input = glm::vec4;

			inline static color::rgba8 shade(const input& frag) {
				return color::rgba8(
					std::clamp(frag.x, 0.0f, 1.0f) * 255,
					std::clamp(frag.y, 0.0f, 1.0f) * 255,
					std::clamp(frag.z, 0.0f, 1.0f) * 255,
					std::clamp(frag.w, 0.0f, 1.0f) * 255
				);
			}

			inline static input interpolate(
				const input& frag0,
				const input& frag1,
				const input& frag2,
				const glm::vec3& coefs
			) {
				return (frag0 * coefs.x) + (frag1 * coefs.y) + (frag2 * coefs.z);
			}
		};


		class vertex {
		public:
			class input {
			public:
				glm::vec3 position;
				glm::vec4 color;
			};

			class output {
			public:
				glm::vec4 rastPos;
				fragment::input data;
			};

			inline static output shade(const input& vert) {
				return { P * V * M * glm::vec4(vert.position, 1.0f), {vert.color} };
			}
		};
	};
}
