#pragma once
#include <vector>
#include <functional>

#include "mesh.hpp"
#include "renderer.hpp"

namespace rast {
	template <typename Shader>
	class command_buffer {
		struct command {
			const mesh::indexed<typename Shader::vertex::input>& mesh;
			range<typename Shader::vertex::output> raster_range;
			typename Shader::vertex::output* intermediate_buffer;
			const typename Shader::uniform_buffer ubo;
			const scissor viewport;
		};

		std::vector<command> commands;

	public:
		void draw_indexed(
			const mesh::indexed<typename Shader::vertex::input>& mesh,
			const typename Shader::uniform_buffer& uniform_buffer,
			const scissor& viewport,
			typename Shader::vertex::output* intermediate_buffer
		) {
			commands.push_back({ mesh, {}, intermediate_buffer, uniform_buffer, viewport });
		}

		template <typename Clipper, typename Framebuffer, typename ThreadPool>
		void submit(
			Framebuffer& framebuffer,
			ThreadPool& tp
		) {
			if (commands.empty()) return;

			for (command& cmd : commands) {
				tp.enque([&cmd]() {
					cmd.raster_range.begin = cmd.intermediate_buffer;
					cmd.raster_range.end = rast::renderer::run_vertex_shader_indexed<typename Shader::vertex, Clipper>(cmd.mesh, cmd.ubo.vertex, cmd.intermediate_buffer);
				});
			}

			tp.wait(); // should be some sort of synch not wait

			float stride = (float)framebuffer.width() / tp.thread_count();
			for (int i = 0; i < tp.thread_count(); ++i) {
				tp.enque([&framebuffer, &cmds = (this->commands), i, stride]() {
					rast::tile tile((int)(i * stride), 0, (int)((i + 1) * stride), framebuffer.height());
					for (const command& cmd : cmds) {
						rast::renderer::rasterize<Shader>(framebuffer, cmd.raster_range.begin, cmd.raster_range.end, cmd.ubo.fragment, cmd.viewport, tile);
					}
				});
			}
		}

		inline void reset() {
			commands.clear();
		}
	};
}
