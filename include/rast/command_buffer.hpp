#pragma once
#include <vector>
#include <type_traits>

#include "mesh.hpp"
#include "renderer.hpp"
#include "cull.hpp"
#include "raster/raster_utils.hpp"
#include "function_traits.hpp"

namespace rast {
	template <typename Shader, uint8_t Extensions = raster::extensions::none>
	class command_buffer {
		using vertex_output = function_return_type<decltype(Shader::vertex::shade)>;
		using vertex_input = std::remove_cvref_t<function_argument<decltype(Shader::vertex::shade)>>;
		using vertex_uniform_buffer = std::remove_cvref_t<function_argument<decltype(Shader::vertex::shade), 1>>;
		using fragment_uniform_buffer = std::remove_cvref_t<function_argument<decltype(Shader::fragment::shade), 1>>;

		struct command {
			const mesh::indexed<vertex_input>& mesh;
			range<vertex_output> raster_range;
			const typename Shader::uniform_buffer ubo;
			const viewport viewport;
		};

		std::vector<command> commands;
		std::vector<vertex_output> intermediate_buffer;

		template <typename Clipper>
		inline static constexpr size_t over_provision(size_t memory_size) {
			return memory_size * 2 + Clipper::maxClipVerts * 4;
		}

		template <typename Clipper>
		constexpr void resize_intermediate_buffer() {
			size_t sum = 0;
			for (const command& cmd : commands) sum += over_provision<Clipper>(cmd.mesh.index_buffer.size());
			intermediate_buffer.resize(sum);
		}

	public:
		void draw_indexed(
			const mesh::indexed<vertex_input>& mesh,
			const typename Shader::uniform_buffer& uniform_buffer,
			const viewport& viewport
		) {
			commands.push_back({ mesh, {}, uniform_buffer, viewport });
		}

		template<typename Rasterizer, typename Clipper, cull Cull = cull_default>
		void submit_vertex(auto& tp, const auto&... args) {
			if (commands.empty()) return;

			resize_intermediate_buffer<Clipper>();
			vertex_output* intermediate_buffer_memory = intermediate_buffer.data();

			for (command& cmd : commands) {
				tp.enque([&cmd, intermediate_buffer_memory, &args...]() {
					cmd.raster_range.begin = intermediate_buffer_memory;
					cmd.raster_range.end = rast::renderer::run_vertex_shader_indexed<
						Shader::vertex::shade, Clipper::clip
					>(
						cmd.mesh, intermediate_buffer_memory, cmd.ubo, args...
					);
				});
				intermediate_buffer_memory += cmd.mesh.index_buffer.size() * 2;
			}
		}

		template<typename Rasterizer, typename Clipper, cull Cull = cull_default>
		void submit_fragment(auto& framebuffer, auto& tp, const auto&... args) {
			float stride = (float)framebuffer.width() / tp.thread_count();
			for (size_t i = 0; i < tp.thread_count(); ++i) {
				tp.enque([&framebuffer, &cmds = (this->commands), i, stride, &args...]() {
					rast::tile tile((int)(i * stride), 0, (int)((i + 1) * stride), framebuffer.height());
					if constexpr (Extensions & raster::extensions::draw_call_id) {
						size_t draw_call_id = 0;
						for (const command& cmd : cmds)
							Rasterizer::template rasterize<Cull, Extensions>(
								framebuffer, cmd.raster_range.begin, cmd.raster_range.end,
								cmd.viewport, tile, cmd.ubo, args..., draw_call_id++
							);
					}
					else for (const command& cmd : cmds)
							Rasterizer::template rasterize<Cull, Extensions>(
								framebuffer, cmd.raster_range.begin, cmd.raster_range.end,
								cmd.viewport, tile, cmd.ubo, args...
							);
				});
			}
		}

		template<typename Rasterizer, typename Clipper, cull Cull = cull_default>
		void submit(auto& framebuffer, auto& tp, const auto&... args) {
			submit_vertex<Rasterizer, Clipper, Cull>(tp, args...);
			tp.wait(); // should be some sort of synch not wait
			submit_fragment<Rasterizer, Clipper, Cull>(framebuffer, tp, args...);
		}

		struct Submitter {
			template <typename ...VArgs>
			struct WithVertexArgs {
				command_buffer& cmd_buf;
				template<typename Rasterizer, typename Clipper, cull Cull = cull_default>
				void submit(auto& framebuffer, auto& tp, const VArgs&... vargs) {
					cmd_buf.submit_vertex<Rasterizer, Clipper, Cull>(tp, vargs...);
					tp.wait(); // should be some sort of synch not wait
					cmd_buf.submit_fragment<Rasterizer, Clipper, Cull>(framebuffer, tp);
				}
				template <typename ...FArgs>
				struct WithFragmentArgs {
					command_buffer& cmd_buf;
					template<typename Rasterizer, typename Clipper, cull Cull = cull_default>
					void submit(auto& framebuffer, auto& tp,
						const VArgs&... vargs, const FArgs&... fargs
					) {
						cmd_buf.submit_vertex<Rasterizer, Clipper, Cull>(tp, vargs...);
						tp.wait(); // should be some sort of synch not wait
						cmd_buf.submit_fragment<Rasterizer, Clipper, Cull>(framebuffer, tp, fargs...);
					}
				};
				template <typename ...Args>
				auto with_fragment_args() {
					return WithFragmentArgs<Args...>{cmd_buf};
				}
			};
			template <typename ...FArgs>
			struct WithFragmentArgs {
				command_buffer& cmd_buf;
				template<typename Rasterizer, typename Clipper, cull Cull = cull_default>
				void submit(auto& framebuffer, auto& tp, const FArgs&... args) {
					cmd_buf.submit_vertex<Rasterizer, Clipper, Cull>(tp);
					tp.wait(); // should be some sort of synch not wait
					cmd_buf.submit_fragment<Rasterizer, Clipper, Cull>(framebuffer, tp, args...);
				}
			};

			command_buffer& cmd_buf;

			template<typename Rasterizer, typename Clipper, cull Cull = cull_default>
			void submit(auto& framebuffer, auto& tp, const auto&... args) {
				cmd_buf.submit<Rasterizer, Clipper, Cull>(framebuffer, tp, args...);
			}
			template<typename Rasterizer, typename Clipper, cull Cull = cull_default>
			void submit_with_vertex_args(auto& framebuffer, auto& tp, const auto&... args) {
				cmd_buf.submit_vertex<Rasterizer, Clipper, Cull>(tp, args...);
				tp.wait(); // should be some sort of synch not wait
				cmd_buf.submit_fragment<Rasterizer, Clipper, Cull>(framebuffer, tp);
			}
			template<typename Rasterizer, typename Clipper, cull Cull = cull_default>
			void submit_with_fragment_args(auto& framebuffer, auto& tp, const auto&... args) {
				cmd_buf.submit_vertex<Rasterizer, Clipper, Cull>(tp);
				tp.wait(); // should be some sort of synch not wait
				cmd_buf.submit_fragment<Rasterizer, Clipper, Cull>(framebuffer, tp, args...);
			}

			template <typename ...Args>
			auto with_vertex_args() {
				return WithVertexArgs<Args...>{cmd_buf};
			}

			template <typename ...Args>
			auto with_fragment_args() {
				return WithFragmentArgs<Args...>{cmd_buf};
			}
		};
		Submitter submitter() { return Submitter{ *this }; }

		void reset() {
			commands.clear();
		}
	};

	template <auto FragmentShader, typename ImageView>
	inline void shade_screen_quad(
		ImageView& framebuffer, auto& tp, const auto&... args
	) {
		float stride = (float)framebuffer.width() / tp.thread_count();
		for (size_t i = 0; i < tp.thread_count(); ++i) {
			tp.enque([&framebuffer, &args..., i, stride]() {
				rast::tile tile((int)(i * stride), 0, (int)((i + 1) * stride), framebuffer.height());
				rast::renderer::draw_screen_quad<FragmentShader>(
					framebuffer,
					rast::viewport(0, 0, framebuffer.width(), framebuffer.height()),
					tile, args...
				);
			});
		}
	}
}
