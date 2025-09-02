#pragma once
#include <vector>
#include <functional>

#include "mesh.hpp"
#include "renderer.hpp"

namespace rast {
	template <typename Shader>
	class command_buffer {
		using vertex_output = typename Shader::vertex::output;
		struct command {
			const mesh::indexed<typename Shader::vertex::input>& mesh;
			range<vertex_output> raster_range;
			const typename Shader::uniform_buffer ubo;
			const viewport viewport;
		};

		std::vector<command> commands;
		std::vector<vertex_output> intermediate_buffer;

		template <typename Clipper>
		static inline constexpr size_t over_provision(size_t memory_size) {
			return memory_size * 2 + Clipper::maxClipVerts * 4;
		}

		template <typename Clipper>
		inline void resize_intermediate_buffer() {
			size_t sum = 0;
			for (const command& cmd : commands) sum += over_provision<Clipper>(cmd.mesh.index_buffer.size());
			intermediate_buffer.resize(sum);
		}

	public:
		void draw_indexed(
			const mesh::indexed<typename Shader::vertex::input>& mesh,
			const typename Shader::uniform_buffer& uniform_buffer,
			const viewport& viewport
		) {
			commands.push_back({ mesh, {}, uniform_buffer, viewport });
		}

		template <typename Rasterizer, typename Clipper, typename Framebuffer, typename ThreadPool>
		void submit(
			Framebuffer& framebuffer,
			ThreadPool& tp
		) {
			if (commands.empty()) return;

			resize_intermediate_buffer<Clipper>();
			vertex_output* intermediate_buffer_memory = intermediate_buffer.data();

			for (command& cmd : commands) {
				tp.enque([&cmd, intermediate_buffer_memory]() {
					cmd.raster_range.begin = intermediate_buffer_memory;
					cmd.raster_range.end = rast::renderer::run_vertex_shader_indexed<typename Shader::vertex, Clipper>(cmd.mesh, cmd.ubo.vertex, intermediate_buffer_memory);
				});
				intermediate_buffer_memory += cmd.mesh.index_buffer.size() * 2;
			}

			tp.wait(); // should be some sort of synch not wait

			float stride = (float)framebuffer.width() / tp.thread_count();
			for (int i = 0; i < tp.thread_count(); ++i) {
				tp.enque([&framebuffer, &cmds = (this->commands), i, stride]() {
					rast::tile tile((int)(i * stride), 0, (int)((i + 1) * stride), framebuffer.height());
					for (const command& cmd : cmds) {
						Rasterizer::template rasterize<Shader, framebuffer::raster_adapter<Framebuffer, Shader>>(
							framebuffer::raster_adapter<Framebuffer, Shader>(framebuffer, cmd.ubo.fragment),
							cmd.raster_range.begin, cmd.raster_range.end, cmd.viewport, tile
						);
					}
				});
			}
		}

		inline void reset() {
			commands.clear();
		}
	};

	template <typename FragmentShader, typename ImageView, typename ThreadPool>
	void shade_screen_quad(
		typename FragmentShader::uniform_buffer ubo,
		ImageView& framebuffer,
		ThreadPool& tp
	) {
		float stride = (float)framebuffer.width / tp.thread_count();
		for (int i = 0; i < tp.thread_count(); ++i) {
			tp.enque([&framebuffer, &ubo, i, stride]() {
				rast::tile tile((int)(i * stride), 0, (int)((i + 1) * stride), framebuffer.height);
				rast::renderer::draw_screen_quad<FragmentShader>(
					framebuffer,
					ubo,
					rast::viewport(0, 0, framebuffer.width, framebuffer.height),
					tile
				);
			});
		}
	}
}
