#pragma once
#include "raster_utils.hpp"

namespace rast::raster {
	struct bbox_scan {
		template <typename Callable, typename VertexT, typename ...Args>
		inline static void rasterize_one(
			Callable&& output,
			const VertexT* triangle,
			glm::ivec3 Cx, glm::ivec3 Cy,
			glm::ivec3 Dx, glm::ivec3 Dy,
			glm::ivec2 min, glm::ivec2 max,
			const Args&... args
		) {
			for (int y = min.y; y < max.y; ++y, Cy += Dx) {
				glm::ivec3 E = Cy - Cx;
				for (int x = min.x; x < max.x; ++x, E -= Dy) {
					if (E.x >= 0 && E.y >= 0 && E.z >= 0) {
						output(x, y, triangle, E, args...);
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
			filter_triangles_x<4, rasterize_one<Callable, Vertex, Args...>, Cull>(output, vertex_begin, vertex_end, viewport, tile, args...);
		}
	};
}
