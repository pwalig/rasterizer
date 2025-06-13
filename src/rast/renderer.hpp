#pragma once
#include <algorithm>
#include <iostream>

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "mesh.hpp"
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

	template <typename VertexT>
	struct range {
		const VertexT* begin;
		const VertexT* end;
	};

	class renderer {
	private:
		inline static void perspective_divide(glm::vec4& vertex) {
			vertex.x /= vertex.w;
			vertex.y /= vertex.w;
			vertex.z /= vertex.w;
		}
		inline static glm::vec4 perspective_divided(const glm::vec4& vertex) {
			return glm::vec4(
				vertex.x / vertex.w,
				vertex.y / vertex.w,
				vertex.z / vertex.w,
				vertex.w
			);
		}

		inline static glm::ivec2 toScreenSpace(const glm::vec4& vertex, const scissor& viewport) {
			return glm::ivec2(
				(( vertex.x + 1.0f ) * (float)viewport.extent.x * 0.5f + (float)viewport.offset.x),
				(( -vertex.y + 1.0f ) * (float)viewport.extent.y * 0.5f + (float)viewport.offset.y)
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

							framebuffer.template draw<Shader>(x, y, vert, uniform_buffer, Cx, area);
						}
						Cx -= Dy;
					}
					Cy += Dx;
				}
			}
		}


	public:
		template <typename Shader, typename Framebuffer, typename RangesIterator>
		inline static void rasterize_ranges(
			Framebuffer& framebuffer,
			RangesIterator begin,
			RangesIterator end,
			const typename Shader::fragment::uniform_buffer& uniform_buffer,
			const scissor& viewport,
			const tile& tile
		) {
			for (RangesIterator it = begin; it != end; ++it) {
				rasterize<Shader, Framebuffer>(framebuffer, it->begin, it->end, uniform_buffer, viewport, tile);
			}
		}

		template <typename VertexIter>
		inline static void perspective_divide(
			typename VertexIter begin,
			typename VertexIter end
		) {
			for (VertexIter it = begin; it != end; ++it) {
				perspective_divide(it->rastPos);
			}
		}

		template <typename VertexShader, typename Clipper>
		inline static void run_vertex_shader_array(
			const typename VertexShader::input* vertex_begin,
			const typename VertexShader::input* vertex_end,
			const typename VertexShader::uniform_buffer& uniform_buffer,
			typename VertexShader::output* output
		) {
			using input_vertex = typename VertexShader::input;
			using output_vertex = typename VertexShader::output;

			output_vertex* end = output;
			for (const input_vertex* vert = vertex_begin; vert != vertex_end;) {
				end[0] = VertexShader::shade(*(vert++), uniform_buffer);
				end[1] = VertexShader::shade(*(vert++), uniform_buffer);
				end[2] = VertexShader::shade(*(vert++), uniform_buffer);

				end = Clipper::template clip<output_vertex>(end);
			}
			perspective_divide(output, end);
			return end;
		}

		template <typename Shader, typename Clipper, typename Framebuffer, typename VertIter>
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
				output_vertex verts[Clipper::maxClipVerts];

				verts[0] = Shader::vertex::shade(*(vert++), uniform_buffer.vertex);
				verts[1] = Shader::vertex::shade(*(vert++), uniform_buffer.vertex);
				verts[2] = Shader::vertex::shade(*(vert++), uniform_buffer.vertex);

				output_vertex* verts_end = Clipper::template clip<output_vertex>(verts);
				perspective_divide(verts, verts_end);
				rasterize<Shader, Framebuffer>(
					framebuffer,
					verts, verts_end,
					uniform_buffer.fragment,
					viewport, tile
				);
			}
		}

		template <typename Shader, typename Clipper, typename Framebuffer, typename VertexBuffer>
		inline static void draw_array(
			Framebuffer& framebuffer,
			const VertexBuffer& vertex_buffer,
			const typename Shader::uniform_buffer& uniform_buffer,
			const scissor& viewport,
			const tile& tile
		) {
			draw_array<Shader, Clipper>(framebuffer, vertex_buffer.begin(), vertex_buffer.end(), uniform_buffer, viewport, tile);
		}

		template <typename VertexShader, typename Clipper, typename IndexIter, typename VertexIter>
		inline static typename VertexShader::output* run_vertex_shader_indexed(
			IndexIter index_begin,
			IndexIter index_end,
			VertexIter vertex_begin,
			VertexIter vertex_end,
			const typename VertexShader::uniform_buffer& uniform_buffer,
			typename VertexShader::output* output
		) {
			using input_vertex = typename VertexShader::input;
			using output_vertex = typename VertexShader::output;

			std::vector<output_vertex> vertex_buffer;
			vertex_buffer.reserve(std::distance(vertex_begin, vertex_end));
			for (VertexIter vert = vertex_begin; vert != vertex_end; ++vert) {
				vertex_buffer.push_back(VertexShader::shade(*vert, uniform_buffer));
			}
			output_vertex* end = output;
			for (IndexIter i = index_begin; i != index_end;) {
				end[0] = vertex_buffer[*(i++)];
				end[1] = vertex_buffer[*(i++)];
				end[2] = vertex_buffer[*(i++)];

				end = Clipper::template clip<output_vertex>(end);
			}
			perspective_divide(output, end);
			return end;
		}

		template <typename VertexShader, typename Clipper>
		inline static typename VertexShader::output* run_vertex_shader_indexed(
			const mesh::indexed<typename VertexShader::input>& mesh,
			const typename VertexShader::uniform_buffer& uniform_buffer,
			typename VertexShader::output* output
		) {
			return run_vertex_shader_indexed<VertexShader, Clipper>(mesh.index_buffer.begin(), mesh.index_buffer.end(), mesh.vertex_buffer.begin(), mesh.vertex_buffer.end(), uniform_buffer, output);
		}

		template <typename Shader, typename Clipper, typename Framebuffer, typename IndexIter, typename VertIter>
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
				output_vertex verts[Clipper::maxClipVerts];

				verts[0] = vertex_buffer[*(i++)];
				verts[1] = vertex_buffer[*(i++)];
				verts[2] = vertex_buffer[*(i++)];

				output_vertex* verts_end = Clipper::template clip<output_vertex>(verts);
				perspective_divide(verts, verts_end);
				rasterize<Shader, Framebuffer>(
					framebuffer,
					verts, verts_end,
					uniform_buffer.fragment,
					viewport, tile
				);
			}
		}

		template <typename Shader, typename Clipper, typename Framebuffer, typename IndexBuffer, typename VertexBuffer>
		inline static void draw_indexed(
			Framebuffer& framebuffer,
			const IndexBuffer& index_buffer,
			const VertexBuffer& vertex_buffer,
			const typename Shader::uniform_buffer& uniform_buffer,
			const scissor& viewport,
			const tile& tile
		) {
			draw_indexed<Shader, Clipper>(framebuffer, index_buffer.begin(), index_buffer.end(), vertex_buffer.begin(), vertex_buffer.end(), uniform_buffer, viewport, tile);
		}

		template <typename Shader, typename Clipper, typename Framebuffer, typename VertexT>
		inline static void draw_indexed(
			Framebuffer& framebuffer,
			const mesh::indexed<VertexT>& mesh,
			const typename Shader::uniform_buffer uniform_buffer,
			const scissor& viewport,
			const tile& tile
		) {
			draw_indexed<Shader, Clipper>(framebuffer, mesh.index_buffer.begin(), mesh.index_buffer.end(), mesh.vertex_buffer.begin(), mesh.vertex_buffer.end(), uniform_buffer, viewport, tile);
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
