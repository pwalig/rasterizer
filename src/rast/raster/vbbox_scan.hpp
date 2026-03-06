#pragma once
#include "raster_utils.hpp"

namespace rast::raster {
	struct vbbox_scan {
		template <typename Callable, typename VertexT, typename ...Args>
		inline static void rasterize_one(
			Callable&& output,
			const VertexT* triangle,
			glm::vec<3, math::simd::i32x4> Cx, glm::vec<3, math::simd::i32x4> Cy,
			glm::vec<3, math::simd::i32x4> Dx, glm::vec<3, math::simd::i32x4> Dy,
			glm::vec<2, math::simd::i32x4> min, glm::ivec2 max,
			const Args&... args
		) {
			using fragment_input = decltype(triangle[0].data);
			using vectorized_fragment_input = std::invoke_result_t<
				decltype(&fragment_input::template vectorize<4>),
				const fragment_input&
			>;
			struct vectorized_vertex {
				glm::vec<4, math::simd::f32x4> rastPos;
				vectorized_fragment_input data;
			};
			vectorized_vertex vtriangle[3]{
				{
					glm::vec<4, math::simd::f32x4>(triangle[0].rastPos),
					triangle[0].data.template vectorize<4>()
				},
				{
					glm::vec<4, math::simd::f32x4>(triangle[1].rastPos),
					triangle[1].data.template vectorize<4>()
				},
				{
					glm::vec<4, math::simd::f32x4>(triangle[2].rastPos),
					triangle[2].data.template vectorize<4>()
				}
			};
			auto increment = math::simd::i32x4(2);
			auto pattern = glm::vec<2, math::simd::i32x4>(
				math::simd::make_x4<int>(0, 0, 1, 1),
				math::simd::make_x4<int>(0, 1, 0, 1)
			);
			min += pattern;
			Cx += Dy * pattern.x;
			Cy += Dx * pattern.y;
			Dx *= increment;
			Dy *= increment;
			__m128i zero = _mm_setzero_si128();
			for (math::simd::i32x4 y = min.y; y[3] < max.y; y += increment, Cy += Dx) {
				glm::vec<3, math::simd::i32x4> E = Cy - Cx;
				for (math::simd::i32x4 x = min.x; x[3] < max.x; x += increment, E -= Dy) {
					math::simd::i32x4 mask = _mm_and_si128(_mm_and_si128(
						_mm_or_si128(_mm_cmpgt_epi32(E.x, zero), _mm_cmpeq_epi32(E.x, zero)),
						_mm_or_si128(_mm_cmpgt_epi32(E.y, zero), _mm_cmpeq_epi32(E.y, zero))
					), _mm_or_si128(_mm_cmpgt_epi32(E.z, zero), _mm_cmpeq_epi32(E.z, zero)));
					alignas(16) int mask_mem[4];
					math::simd::store(mask_mem, mask);
					math::boolx4 bmask = math::make_x4<bool>(mask_mem[0], mask_mem[1], mask_mem[2], mask_mem[3]);
					if (math::or_accross(bmask)) {
						output(x, y, vtriangle, E, bmask, args...);
					}
				}
			}
		}

		template <typename Callable, typename Vertex, typename ...Args>
		inline static void rasterize(
			Callable&& output,
			const Vertex* vertex_begin,
			const Vertex* vertex_end,
			const viewport& viewport, const tile& tile,
			const Args&... args
		) {
			filter_triangles_x4<rasterize_one<Callable, Vertex, Args...>>(output, vertex_begin, vertex_end, viewport, tile, args...);
		}
	};
}
