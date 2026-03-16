#pragma once
#include "raster_utils.hpp"

namespace rast::raster {
	struct vbbox_scan {
		template <size_t Count>
		inline static glm::vec<2, simd::i32x_<Count>> pixel_pattern() {
			static_assert((Count == 4) || (Count == 8));
			if constexpr (Count == 4) {
				return glm::vec<2, simd::i32x4>(
					simd::make_x4<int>(0, 0, 1, 1),
					simd::make_x4<int>(0, 1, 0, 1)
				);
			}
			else if constexpr (Count == 8) {
				return glm::vec<2, simd::i32x8>(
					simd::make_x8<int>(0, 0, 1, 1, 2, 2, 3, 3),
					simd::make_x8<int>(0, 1, 0, 1, 0, 1, 0, 1)
				);
			}
		}
		template <size_t Count, typename Callable, typename VertexT, typename ...Args>
		inline static void rasterize_one(
			Callable&& output,
			const VertexT* triangle,
			glm::vec<3, simd::i32x_<Count>> Cx, glm::vec<3, simd::i32x_<Count>> Cy,
			glm::vec<3, simd::i32x_<Count>> Dx, glm::vec<3, simd::i32x_<Count>> Dy,
			glm::vec<2, simd::i32x_<Count>> min, glm::vec<2, simd::i32x_<Count>> max,
			const Args&... args
		) {
			using i32 = simd::i32x_<Count>;
			using f32 = simd::f32x_<Count>;

			using fragment_input = decltype(triangle[0].data);
			using vectorized_fragment_input = std::invoke_result_t<
				decltype(&fragment_input::template vectorize<Count>),
				const fragment_input&
			>;
			struct vectorized_vertex {
				glm::vec<4, f32> rastPos;
				vectorized_fragment_input data;
			};
			vectorized_vertex vtriangle[3]{
				{
					glm::vec<4, f32>(triangle[0].rastPos),
					triangle[0].data.template vectorize<Count>()
				},
				{
					glm::vec<4, f32>(triangle[1].rastPos),
					triangle[1].data.template vectorize<Count>()
				},
				{
					glm::vec<4, f32>(triangle[2].rastPos),
					triangle[2].data.template vectorize<Count>()
				}
			};
			auto increment = glm::vec<2, i32>(i32(Count / 2), i32(2));
			auto pattern = pixel_pattern<Count>();
			min += pattern;
			Cx += Dy * pattern.x;
			Cy += Dx * pattern.y;
			Dx *= increment.y;
			Dy *= increment.x;
			i32 zero = simd::setzero<int, Count>();
			for (i32 y = min.y; simd::movemask(y < max.y); y += increment.y, Cy += Dx) {
				glm::vec<3, i32> E = Cy - Cx;
				for (i32 x = min.x; simd::movemask(x < max.x); x += increment.x, E -= Dy) {
					i32 mask = (E.x >= zero) & (E.y >= zero) & (E.z >= zero) & (y < max.y) & (x < max.x);
					if (simd::movemask(mask)) {
						output(x, y, vtriangle, E, mask, args...);
					}
				}
			}
		}

		template <cull Cull = cull_default, typename Callable, typename Vertex, typename ...Args>
		inline static void rasterize(
			Callable&& output,
			const Vertex* vertex_begin,
			const Vertex* vertex_end,
			const viewport& viewport, const tile& tile,
			const Args&... args
		) {
			filter_triangles<rasterize_one<4, Callable, Vertex, Args...>, Cull>(
				output, vertex_begin, vertex_end, viewport, tile, args...
			);
		}
	};
}
