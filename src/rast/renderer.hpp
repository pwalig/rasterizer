#pragma once
#include <algorithm>
#include <iostream>

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "types.hpp"
#include "image.hpp"

namespace rast {
	class scissor {
	public:
		glm::ivec2 offset;
		glm::ivec2 extent;
		inline scissor(int xoffset, int yoffset, int width, int height) :
			offset(xoffset << 4, yoffset << 4), extent(width << 4, height << 4) {}
	};

	class tile {
	public:
		glm::ivec2 min;
		glm::ivec2 max;
		inline tile(int minx, int miny, int maxx, int maxy) :
			min(minx << 4, miny << 4), max(maxx << 4, maxy << 4) {}
	};
	class renderer {
	private:
		inline static glm::ivec2 toScreenSpace(const glm::vec4& vertex, const scissor& viewport) {
			glm::vec2 res(
				vertex.x / vertex.w,
				vertex.y / vertex.w
			);

			return glm::ivec2(
				(( res.x + 1.0f ) * (float)viewport.extent.x * 0.5f + (float)viewport.offset.x),
				(( -res.y + 1.0f ) * (float)viewport.extent.y * 0.5f + (float)viewport.offset.y)
			);
		}
		
		template <typename Shader, typename Framebuffer>
		inline static void rasterize(
			Framebuffer& framebuffer,
			const typename Shader::vertex::output* vertex_begin,
			const typename Shader::vertex::output* vertex_end,
			const typename Shader::fragment::uniform_buffer& uniform_buffer,
			const scissor& viewport,
			const tile& tile
		) {
			using vertex = typename Shader::vertex::output;

			for (const vertex* vert = vertex_begin; vert != vertex_end; vert += 3) {

				glm::ivec2 a = toScreenSpace(vert[0].rastPos, viewport);
				glm::ivec2 b = toScreenSpace(vert[1].rastPos, viewport);
				glm::ivec2 c = toScreenSpace(vert[2].rastPos, viewport);

				glm::ivec2 min = glm::ivec2(
					std::max((int)std::min({ a.x, b.x, c.x }), std::max(tile.min.x, viewport.offset.x)),
					std::max((int)std::min({ a.y, b.y, c.y }), std::max(tile.min.y, viewport.offset.y))
				) / 16;
				glm::ivec2 max = glm::ivec2(
					std::min<int>({ std::max({ a.x, b.x, c.x }) + 16, tile.max.x, viewport.offset.x + viewport.extent.x }),
					std::min<int>({ std::max({ a.y, b.y, c.y }) + 16, tile.max.y, viewport.offset.y + viewport.extent.y })
				) / 16;

				if (min.x >= max.x || min.y >= max.y) continue;

				glm::ivec3 x012 = glm::ivec3(a.x, b.x, c.x);
				glm::ivec3 x120 = glm::ivec3(b.x, c.x, a.x);

				glm::ivec3 y012 = glm::ivec3(a.y, b.y, c.y);
				glm::ivec3 y120 = glm::ivec3(b.y, c.y, a.y);

				glm::ivec3 Dx = x120 - x012;
				glm::ivec3 Dy = y120 - y012;

				glm::ivec3 fill_convention = glm::ivec3(
					(Dy.x > 0 || (Dy.x == 0 && Dx.x < 0)) ? 1 : 0,
					(Dy.y > 0 || (Dy.y == 0 && Dx.y < 0)) ? 1 : 0,
					(Dy.z > 0 || (Dy.z == 0 && Dx.z < 0)) ? 1 : 0
				);

				int area = (Dy.x * Dx.z) - (Dx.x * Dy.z);

				// Dx * Y - fill_convention
				glm::ivec3 Cy = Dx * (glm::ivec3(min.y << 4) - y012) - fill_convention;
				glm::ivec3 CCx = Dy * (glm::ivec3(min.x << 4) - x012);
				Dx *= 16;
				Dy *= 16;

				for (int y = min.y; y < max.y; ++y) {
					glm::ivec3 Cx = Cy - CCx;
					for (int x = min.x; x < max.x; ++x) {

						if (Cx.x >= 0 && Cx.y >= 0 && Cx.z >= 0) {

							// interpolation coefitients
							glm::vec3 coefs(
								(float)Cx.y / area,
								(float)Cx.z / area,
								(float)Cx.x / area
							);

							framebuffer.template draw<Shader>(x, y, vert, uniform_buffer, coefs);
						}
						Cx -= Dy;
					}
					Cy += Dx;
				}
			}
		}

		template <typename VertT>
		inline static VertT clip_vert(
			const VertT& v0, const VertT& v1,
			float coef0, float coef1
		) {
			float t = coef0 / (coef0 - coef1);
			return (v0 * (1.0f - t)) + (v1 * t);
		}

		template <typename Shader, typename Framebuffer>
		inline static void clip_and_draw(
			Framebuffer& framebuffer,
			typename Shader::vertex::output* verts,
			const typename Shader::fragment::uniform_buffer& uniform_buffer,
			const scissor& viewport,
			const tile& tile
		) {
			if (
				verts[0].rastPos.z > verts[0].rastPos.w ||
				verts[1].rastPos.z > verts[1].rastPos.w ||
				verts[2].rastPos.z > verts[2].rastPos.w
			) return;

			glm::vec4 equation = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
			float values[3] = {
				glm::dot(verts[0].rastPos, equation),
				glm::dot(verts[1].rastPos, equation),
				glm::dot(verts[2].rastPos, equation)
			};

			u8 mask =
				(values[0] < 0.0f ? 1 : 0) |
				(values[1] < 0.0f ? 2 : 0) |
				(values[2] < 0.0f ? 4 : 0);

			auto end = verts + 3;
			switch (mask) {
			case 0b110:
				verts[1] = clip_vert(verts[1], verts[0], values[1], values[0]);
				verts[2] = clip_vert(verts[2], verts[0], values[2], values[0]);
				break;
			case 0b101:
				verts[0] = clip_vert(verts[0], verts[1], values[0], values[1]);
				verts[2] = clip_vert(verts[2], verts[1], values[2], values[1]);
				break;
			case 0b011:
				verts[0] = clip_vert(verts[0], verts[2], values[0], values[2]);
				verts[1] = clip_vert(verts[1], verts[2], values[1], values[2]);
				break;
			case 0b001:
				{
					auto v01 = clip_vert(verts[0], verts[1], values[0], values[1]);
					auto v02 = clip_vert(verts[0], verts[2], values[0], values[2]);
					verts[0] = v01;
					verts[3] = v01;
					verts[4] = verts[2];
					verts[5] = v02;
					end += 3;
				}
				break;
			case 0b010:
				{
					auto v10 = clip_vert(verts[1], verts[0], values[1], values[0]);
					auto v12 = clip_vert(verts[1], verts[2], values[1], values[2]);
					verts[1] = v10;
					verts[3] = v10;
					verts[4] = v12;
					verts[5] = verts[2];
					end += 3;
				}
				break;
			case 0b100:
				{
					auto v20 = clip_vert(verts[2], verts[0], values[2], values[0]);
					auto v21 = clip_vert(verts[2], verts[1], values[2], values[1]);
					verts[2] = v21;
					verts[3] = v21;
					verts[4] = v20;
					verts[5] = verts[0];
					end += 3;
				}
				break;
			case 0b111:
				return;
			}

			rasterize<Shader, Framebuffer>(
				framebuffer,
				verts, end,
				uniform_buffer,
				viewport,
				tile
			);
		}

	public:
		template <typename Shader, typename Framebuffer, typename VertIter>
		inline static void draw_array(
			Framebuffer& framebuffer,
			VertIter vertex_begin,
			VertIter vertex_end,
			const typename Shader::uniform_buffer& uniform_buffer,
			const scissor& viewport,
			const tile& tile
		) {
			using input_vertex = typename Shader::vertex::input;
			using output_vertex = typename Shader::vertex::output;

			for (auto vert = vertex_begin; vert != vertex_end;) {
				output_vertex verts[6];

				verts[0] = Shader::vertex::shade(*(vert++), uniform_buffer.vertex);
				verts[1] = Shader::vertex::shade(*(vert++), uniform_buffer.vertex);
				verts[2] = Shader::vertex::shade(*(vert++), uniform_buffer.vertex);

				clip_and_draw<Shader, Framebuffer>(
					framebuffer,
					verts,
					uniform_buffer.fragment,
					viewport,
					tile
				);
			}
		}

		template <typename Shader, typename Framebuffer, typename VertexBuffer>
		inline static void draw_array(
			Framebuffer& framebuffer,
			const VertexBuffer& vertex_buffer,
			const typename Shader::uniform_buffer& uniform_buffer,
			const scissor& viewport,
			const tile& tile
		) {
			draw_array<Shader>(framebuffer, vertex_buffer.begin(), vertex_buffer.end(), uniform_buffer, viewport, tile);
		}

		template <typename Shader, typename Framebuffer, typename IndexIter, typename VertIter>
		inline static void draw_indexed(
			Framebuffer& framebuffer,
			IndexIter index_begin,
			IndexIter index_end,
			VertIter vertex_begin,
			VertIter vertex_end,
			const typename Shader::uniform_buffer& uniform_buffer,
			const scissor& viewport,
			const tile& tile
		) {
			using input_vertex = typename Shader::vertex::input;
			using output_vertex = typename Shader::vertex::output;

			std::vector<output_vertex> vertex_buffer;
			vertex_buffer.reserve(vertex_end - vertex_begin);
			for (auto vert = vertex_begin; vert != vertex_end; ++vert) {
				vertex_buffer.push_back(Shader::vertex::shade(*vert, uniform_buffer.vertex));
			}

			for (auto i = index_begin; i != index_end;) {
				output_vertex verts[6];

				verts[0] = vertex_buffer[*(i++)];
				verts[1] = vertex_buffer[*(i++)];
				verts[2] = vertex_buffer[*(i++)];

				clip_and_draw<Shader, Framebuffer>(
					framebuffer,
					verts,
					uniform_buffer.fragment,
					viewport,
					tile
				);
			}
		}

		template <typename Shader, typename Framebuffer, typename IndexBuffer, typename VertexBuffer>
		inline static void draw_indexed(
			Framebuffer& framebuffer,
			const IndexBuffer& index_buffer,
			const VertexBuffer& vertex_buffer,
			const typename Shader::uniform_buffer& uniform_buffer,
			const scissor& viewport,
			const tile& tile
		) {
			draw_indexed<Shader>(framebuffer, index_buffer.begin(), index_buffer.end(), vertex_buffer.begin(), vertex_buffer.end(), uniform_buffer, viewport, tile);
		}

		template <typename Shader, typename Framebuffer, typename VertexT>
		inline static void draw_indexed(
			Framebuffer& framebuffer,
			const mesh::indexed<VertexT>& mesh,
			const typename Shader::uniform_buffer uniform_buffer,
			const scissor& viewport,
			const tile& tile
		) {
			draw_indexed<Shader>(framebuffer, mesh.index_buffer.begin(), mesh.index_buffer.end(), mesh.vertex_buffer.begin(), mesh.vertex_buffer.end(), uniform_buffer, viewport, tile);
		}

		template <typename FragmentShader, typename ImageView>
		inline static void draw_screen_quad(
			ImageView& imageView,
			const typename FragmentShader::uniform_buffer& uniform_buffer,
			const scissor& viewport,
			const tile& tile
		) {
			glm::ivec2 max = viewport.extent / 16;
			glm::ivec2 off = viewport.offset / 16;
			for (int y = tile.min.y; y < tile.max.y; ++y) {
				for (int x = tile.min.x; x < tile.max.x; ++x) {
					glm::vec2 frag = glm::vec2((float)x / max.x, (float)y / max.y);
					imageView.at(x + off.x, y + off.y) = FragmentShader::shade(frag, uniform_buffer);
				}
			}
		}
	};
}
