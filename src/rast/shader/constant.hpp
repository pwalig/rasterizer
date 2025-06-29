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

			inline static output shade(const input&, const uniform_buffer& uniforms) {
				return uniforms.color;
			}
		};

		class vertex {
		public:
			using input = inputs::position;
			using output = vertex_shader_output<constant>;

			using uniform_buffer = uniforms::PVM_struct;

			inline static output shade(const input& vert, const uniform_buffer& uniforms) {
				return { uniforms.PVM * glm::vec4(vert, 1.0f), {} };
			}
			inline static output shade(const input& vert, const uniforms::MVP& uniforms) {
				return { uniforms.P * uniforms.V * uniforms.M * glm::vec4(vert, 1.0f), {} };
			}
		};

		using uniform_buffer = shader_uniform_buffer<constant>;
	};
}
