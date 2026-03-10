#pragma once
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "../color.hpp"
#include "../sampler.hpp"
#include "shader_macros.hpp"

namespace rast::shader {
	struct textured {
		struct fragment {
			using input = inputs::uv;
			using output = color::rgba8;

			struct uniform_buffer {
				sampler<rast::color::rgba8> texture;
			};

			inline static color::rgba8 shade(const input& frag, const uniform_buffer& uniforms) {
				if (uniforms.texture) return uniforms.texture.sample_nearest(frag.x, frag.y);
				else return color::rgba8(255, 0, 255, 255);
			}
		};


		struct vertex {
			using input = inputs::position_uv;
			using output = vertex_shader_output<textured>;

			using uniform_buffer = uniforms::PVM_struct;

			inline static output shade(const input& vert, const uniform_buffer& uniforms) {
				return { uniforms.PVM * glm::vec4(vert.position, 1.0f), {vert.uv} };
			}
			inline static output shade(const input& vert, const uniforms::MVP& uniforms) {
				return { uniforms.P * uniforms.V * uniforms.M * glm::vec4(vert.position, 1.0f), {vert.uv} };
			}
		};

		using uniform_buffer = shader_uniform_buffer<textured>;
	};
}
