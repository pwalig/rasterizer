#pragma once
#include "raster_utils.hpp"

namespace rast::raster {
	struct left_edge {
		template <typename Shader, typename Callable, typename ...Args>
		inline static void rasterize_one(
			Callable&& output,
			const typename Shader::vertex::output* triangle,
			const viewport& viewport,
			const tile& tile,
			Args&&... args
		) {
			glm::ivec2 a = to_screen_space(triangle[0].rastPos, viewport);
			glm::ivec2 b = to_screen_space(triangle[1].rastPos, viewport);
			glm::ivec2 c = to_screen_space(triangle[2].rastPos, viewport);

			glm::ivec2 min = glm::ivec2(
				std::max((int)std::min({ a.x, b.x, c.x }), std::max(tile.min.x, viewport.offset.x)),
				std::max((int)std::min({ a.y, b.y, c.y }), std::max(tile.min.y, viewport.offset.y))
			) / 16;
			glm::ivec2 max = glm::ivec2(
				std::min<int>({ std::max({ a.x, b.x, c.x }) + 16, tile.max.x, viewport.offset.x + viewport.extent.x }),
				std::min<int>({ std::max({ a.y, b.y, c.y }) + 16, tile.max.y, viewport.offset.y + viewport.extent.y })
			) / 16;

			if (min.x >= max.x || min.y >= max.y) return;

			glm::ivec3 x012 = glm::ivec3(a.x, b.x, c.x);
			glm::ivec3 x120 = glm::ivec3(b.x, c.x, a.x);

			glm::ivec3 y012 = glm::ivec3(a.y, b.y, c.y);
			glm::ivec3 y120 = glm::ivec3(b.y, c.y, a.y);

			glm::ivec3 Dx = x120 - x012;
			glm::ivec3 Dy = y120 - y012;

			int area = (Dy.x * Dx.z) - (Dx.x * Dy.z);

			// Dx * Y - fill_convention
			glm::ivec3 Cy = Dx * (glm::ivec3(min.y << 4) - y012) - fill_convention(Dx, Dy);
			glm::ivec3 CCx = Dy * (glm::ivec3(min.x << 4) - x012);
			glm::ivec3 Cx = CCx;
			int X = min.x;
			Dx *= 16;
			Dy *= 16;

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
					dispached_output<Shader>(output, x, y, triangle, partial_coefs<glm::vec3>(E, area), std::forward<Args>(args)...);
					if (E.x < 0 || E.y < 0 || E.z < 0) break;
				}
			}
		}

		template <typename Shader, typename Callable, typename ...Args>
		inline static void rasterize(
			Callable&& output,
			const typename Shader::vertex::output* vertex_begin,
			const typename Shader::vertex::output* vertex_end,
			const viewport& viewport, const tile& tile,
			Args&&... args
		) {
			using vertex = typename Shader::vertex::output;
			for (const vertex* triangle = vertex_begin; triangle != vertex_end; triangle += 3) {
				rasterize_one<Shader>(output, triangle, viewport, tile, args...);
			}
		}
	};
}
