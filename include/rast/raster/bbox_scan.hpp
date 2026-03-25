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

		template <uint8_t Extensions, typename Callable, typename Vertex, typename ...Args>
		inline constexpr static auto get_rasterize_one_based_on_extensions() {
			if constexpr (Extensions & extensions::primitive_id) {
				if constexpr (Extensions & extensions::clockwise)
					return rasterize_one<Callable, Vertex, Args..., size_t, bool>;
				else return rasterize_one<Callable, Vertex, Args..., size_t>;
			}
			else {
				if constexpr (Extensions & extensions::clockwise)
					return rasterize_one<Callable, Vertex, Args..., bool>;
				else return rasterize_one<Callable, Vertex, Args...>;
			}
		}

		template <cull Cull = cull_default, uint8_t Extensions = extensions::none, typename Callable, typename Vertex, typename ...Args>
		inline static void rasterize(
			Callable&& output,
			const Vertex* vertex_begin,
			const Vertex* vertex_end,
			const viewport& viewport, const tile& tile,
			const Args&... args
		) {
			filter_triangles<get_rasterize_one_based_on_extensions<Extensions, Callable, Vertex, Args...>(), Cull, Extensions>(
				output, vertex_begin, vertex_end, viewport, tile, args...
			);
		}
	};
}
