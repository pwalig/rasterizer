#pragma once
#include <algorithm>
#include <iostream>

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "types.hpp"
#include "image.hpp"

namespace rast {
	class renderer {
	public:
		using data_len_t = u32;
	private:
		glm::ivec2 viewportDims = glm::ivec2(100, 100);
		glm::ivec2 viewportOffset = glm::ivec2(0, 0);

		glm::ivec3 toScreenSpace(const glm::vec4& vertex);

		template <typename Image, typename Shader>
		void rasterize(
			Image& image,
			const typename Shader::vertex::output* vertex_begin,
			const typename Shader::vertex::output* vertex_end
		) {
			using vertex = typename Shader::vertex::output;

			for (const vertex* vert = vertex_begin; vert != vertex_end; vert += 3) {

				glm::ivec3 a = toScreenSpace(vert->rastPos);
				glm::ivec3 b = toScreenSpace((vert + 1)->rastPos);
				glm::ivec3 c = toScreenSpace((vert + 2)->rastPos);

				glm::ivec2 min = glm::ivec2(
					std::max((int)std::min({ a.x, b.x, c.x }), 0),
					std::max((int)std::min({ a.y, b.y, c.y }), 0)
				);
				glm::ivec2 max = glm::ivec2(
					std::min<int>(std::max({ a.x, b.x, c.x }) + 16, image.width << 4),
					std::min<int>(std::max({ a.y, b.y, c.y }) + 16, image.height << 4)
				);

				glm::ivec3 x123 = glm::ivec3(a.x, b.x, c.x);
				glm::ivec3 x231 = glm::ivec3(x123.y, x123.z, x123.x);
				glm::ivec3 y123 = glm::ivec3(a.y, b.y, c.y);
				glm::ivec3 y231 = glm::ivec3(y123.y, y123.z, y123.x);

				glm::ivec3 Dx = x123 - x231;
				glm::ivec3 Dy = y123 - y231;

				max /= 16;
				min /= 16;
				glm::ivec3 fill_convention = glm::ivec3(
					(Dy.x > 0 || (Dy.x == 0 && Dx.x < 0)) ? 1 : 0,
					(Dy.y > 0 || (Dy.y == 0 && Dx.y < 0)) ? 1 : 0,
					(Dy.z > 0 || (Dy.z == 0 && Dx.z < 0)) ? 1 : 0
				);

				for (int y = min.y; y < max.y; ++y) {
					// Dx * Y + fill_convention
					glm::ivec3 Cy = Dx * (glm::ivec3(y * 16) - y123) + fill_convention;

					for (int x = min.x; x < max.x; ++x) {
						glm::ivec3 X = glm::ivec3(x * 16) - x123;

						glm::ivec3 res = (Dy * X) - Cy;
						if (res.x >= 0 && res.y >= 0 && res.z >= 0) {
							image.at(x, y) += Shader::fragment::shade(vertex_begin->data);
						}
					}
				}
			}
		}

	public:
		void setViewport(int xoffset, int yoffset, int width, int height);

		template <typename Image, typename Shader>
		void draw_array(
			Image& image,
			const typename Shader::vertex::input* vertex_begin,
			const typename Shader::vertex::input* vertex_end
		) {
			using input_vertex = typename Shader::vertex::input;
			using output_vertex = typename Shader::vertex::output;

			for (const input_vertex* vert = vertex_begin; vert < vertex_end; vert += 3) {
				output_vertex clipped_verts[3];

				clipped_verts[0] = Shader::vertex::shade(*vert);
				clipped_verts[1] = Shader::vertex::shade(*(vert + 1));
				clipped_verts[2] = Shader::vertex::shade(*(vert + 2));

				rasterize<Image, Shader>(
					image,
					clipped_verts,
					clipped_verts + 3
				);
			}
		}

	};
}
