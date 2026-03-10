#pragma once
#include "raster_utils.hpp"

namespace rast::raster {
	struct left_edge {
		template <typename Callable, typename VertexT, typename ...Args>
		inline static void rasterize_one(
			Callable&& output,
			const VertexT* triangle,
			glm::ivec3 Cx, glm::ivec3 Cy,
			glm::ivec3 Dx, glm::ivec3 Dy,
			glm::ivec2 min, glm::ivec2 max,
			const Args&... args
		) {
			// Dx * Y - fill_convention
			glm::ivec3 CCx = Cx;
			int X = min.x;

			for (int y = min.y; y < max.y; ++y, Cy += Dx) {
				glm::ivec3 E = Cy - Cx;
				if (E.x < 0 || E.y < 0 || E.z < 0) {
					for (; X < max.x; ++X, E -= Dy) {
						if (E.x >= 0 && E.y >= 0 && E.z >= 0) break;
					}
					if (X == max.x) {
						X = min.x;
						Cx = CCx;
						continue;
					}
				}
				else {
					for (; X >= min.x; --X, E += Dy) {
						if (E.x < 0 || E.y < 0 || E.z < 0) break;
					}
					++X;
					E -= Dy;
				}
				Cx = Cy - E;
				for (int x = X; x < max.x; ++x, E -= Dy) {
					output(x, y, triangle, E, args...);
					if (E.x < 0 || E.y < 0 || E.z < 0) break;
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
			filter_triangles_x<4, rasterize_one<Callable, Vertex, Args...>>(output, vertex_begin, vertex_end, viewport, tile, args...);
		}
	};
}
