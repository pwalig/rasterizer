#pragma once
#include <algorithm>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "mesh.hpp"
#include "image.hpp"
#include "viewport.hpp"
#include "tile.hpp"
#include "perspective_divide.hpp"

namespace rast {
	template <typename VertexT>
	struct range {
		const VertexT* begin;
		const VertexT* end;
	};

	class renderer {
	public:
		template <typename VertexIter>
		inline static void perspective_divide(
			VertexIter begin,
			VertexIter end
		) {
			for (VertexIter it = begin; it != end; ++it) {
				perspective_divide::divide(it->rastPos);
			}
		}

		template <typename VertexShader, typename Clipper>
		inline static typename VertexShader::output* run_vertex_shader_array(
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

		template <typename Shader, typename Rasterizer, typename Clipper, typename Framebuffer, typename VertIter>
		inline static void draw_array(
			Framebuffer& framebuffer,
			VertIter vertex_begin,
			VertIter vertex_end,
			const typename Shader::uniform_buffer& uniform_buffer,
			const viewport& viewport,
			const tile& tile
		) {
			using output_vertex = typename Shader::vertex::output;

			for (auto vert = vertex_begin; vert != vertex_end;) {
				output_vertex verts[Clipper::maxClipVerts];

				verts[0] = Shader::vertex::shade(*(vert++), uniform_buffer.vertex);
				verts[1] = Shader::vertex::shade(*(vert++), uniform_buffer.vertex);
				verts[2] = Shader::vertex::shade(*(vert++), uniform_buffer.vertex);

				output_vertex* verts_end = Clipper::template clip<output_vertex>(verts);
				perspective_divide(verts, verts_end);
				Rasterizer::template rasterize<Shader, Framebuffer>(
					framebuffer,
					verts, verts_end,
					uniform_buffer.fragment,
					viewport, tile
				);
			}
		}

		template <typename Shader, typename Rasterizer, typename Clipper, typename Framebuffer, typename VertexBuffer>
		inline static void draw_array(
			Framebuffer& framebuffer,
			const VertexBuffer& vertex_buffer,
			const typename Shader::uniform_buffer& uniform_buffer,
			const viewport& viewport,
			const tile& tile
		) {
			draw_array<Shader, Rasterizer, Clipper>(framebuffer, vertex_buffer.begin(), vertex_buffer.end(), uniform_buffer, viewport, tile);
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

		template <typename VertexShader, typename Clipper, typename IndexBuffer, typename VertexBuffer>
		inline static typename VertexShader::output* run_vertex_shader_indexed(
			const IndexBuffer& index_buffer,
			const VertexBuffer& vertex_buffer,
			const typename VertexShader::uniform_buffer& uniform_buffer,
			typename VertexShader::output* output
		) {
			static_assert(std::is_same_v<typename IndexBuffer::value_type, uint32_t>);
			static_assert(std::is_same_v<typename VertexBuffer::value_type, typename VertexShader::input>);
			return run_vertex_shader_indexed<VertexShader, Clipper>(index_buffer.begin(), index_buffer.end(), vertex_buffer.begin(), vertex_buffer.end(), uniform_buffer, output);
		}

		template <typename VertexShader, typename Clipper>
		inline static typename VertexShader::output* run_vertex_shader_indexed(
			const mesh::indexed<typename VertexShader::input>& mesh,
			const typename VertexShader::uniform_buffer& uniform_buffer,
			typename VertexShader::output* output
		) {
			return run_vertex_shader_indexed<VertexShader, Clipper>(mesh.index_buffer.begin(), mesh.index_buffer.end(), mesh.vertex_buffer.begin(), mesh.vertex_buffer.end(), uniform_buffer, output);
		}

		template <typename Shader, typename Rasterizer, typename Clipper, typename Framebuffer, typename IndexIter, typename VertIter>
		inline static void draw_indexed(
			Framebuffer& framebuffer,
			IndexIter index_begin,
			IndexIter index_end,
			VertIter vertex_begin,
			VertIter vertex_end,
			const typename Shader::uniform_buffer& uniform_buffer,
			const viewport& viewport,
			const tile& tile
		) {
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
				Rasterizer::template rasterize<Shader, Framebuffer>(
					framebuffer,
					verts, verts_end,
					uniform_buffer.fragment,
					viewport, tile
				);
			}
		}

		template <typename Shader, typename Rasterizer, typename Clipper, typename Framebuffer, typename IndexBuffer, typename VertexBuffer>
		inline static void draw_indexed(
			Framebuffer& framebuffer,
			const IndexBuffer& index_buffer,
			const VertexBuffer& vertex_buffer,
			const typename Shader::uniform_buffer& uniform_buffer,
			const viewport& viewport,
			const tile& tile
		) {
			static_assert(std::is_same_v<typename IndexBuffer::value_type, uint32_t>);
			static_assert(std::is_same_v<typename VertexBuffer::value_type, typename Shader::vertex::input>);
			draw_indexed<Shader, Rasterizer, Clipper>(
				framebuffer,
				index_buffer.begin(), index_buffer.end(),
				vertex_buffer.begin(), vertex_buffer.end(),
				uniform_buffer, viewport, tile
			);
		}

		template <typename Shader, typename Rasterizer, typename Clipper, typename Framebuffer>
		inline static void draw_indexed(
			Framebuffer& framebuffer,
			const mesh::indexed<typename Shader::vertex::input>& mesh,
			const typename Shader::uniform_buffer uniform_buffer,
			const viewport& viewport,
			const tile& tile
		) {
			draw_indexed<Shader, Rasterizer, Clipper>(
				framebuffer,
				mesh.index_buffer.begin(), mesh.index_buffer.end(),
				mesh.vertex_buffer.begin(), mesh.vertex_buffer.end(),
				uniform_buffer, viewport, tile
			);
		}

		template <typename FragmentShader, typename ImageView>
		inline static void draw_screen_quad(
			ImageView& imageView,
			const typename FragmentShader::uniform_buffer& uniform_buffer,
			const viewport& viewport,
			const tile& tile
		) {
			glm::ivec2 max = viewport.extent / 16;
			glm::ivec2 off = viewport.offset / 16;
			for (int y = tile.min.y / 16; y < tile.max.y / 16; ++y) {
				for (int x = tile.min.x / 16; x < tile.max.x / 16; ++x) {
					glm::vec2 frag = glm::vec2((float)x / max.x, (float)y / max.y);
					imageView.at(x + off.x, y + off.y) = FragmentShader::shade(frag, uniform_buffer);
				}
			}
		}
	};
}
