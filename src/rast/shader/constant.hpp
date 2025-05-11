#pragma once
#include <glm/glm.hpp>

#include "../color.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

namespace rast::shader {
	class constant {
	public:
		inline static glm::mat4 M = glm::mat4(1.0f);
		inline static glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		inline static glm::mat4 P = glm::perspective(glm::radians(70.0f), 16.0f / 9.0f, 0.1f, 100.0f);

		inline static color::rgba8 color;

		class fragment {
		public:

			class input { };

			inline static color::rgba8 shade(const input& frag) {
				return color;
			}
		};

		class vertex {
		public:
			using input = glm::vec3;

			class output {
			public:
				glm::vec4 rastPos;
				fragment::input data;
			};

			inline static output shade(const input& vert) {
				return { P * V * M * glm::vec4(vert, 1.0f), {} };
			}
		};
	};
}
