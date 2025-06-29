#pragma once
#include "raster_utils.hpp"

namespace rast::raster {
	struct bbox_scan {
		template <typename Shader, typename Framebuffer>
		inline static void rasterize_one(
			Framebuffer& framebuffer,
			const typename Shader::vertex::output* triangle,
			const typename Shader::fragment::uniform_buffer& uniform_buffer,
			const viewport& viewport,
			const tile& tile
		) {
			glm::ivec2 a = toScreenSpace(triangle[0].rastPos, viewport);
			glm::ivec2 b = toScreenSpace(triangle[1].rastPos, viewport);
			glm::ivec2 c = toScreenSpace(triangle[2].rastPos, viewport);

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
			glm::ivec3 Cx = Dy * (glm::ivec3(min.x << 4) - x012);
			Dx *= 16;
			Dy *= 16;

			for (int y = min.y; y < max.y; ++y, Cy += Dx) {
				glm::ivec3 E = Cy - Cx;
				for (int x = min.x; x < max.x; ++x, E -= Dy) {
					if (E.x >= 0 && E.y >= 0 && E.z >= 0) {
						framebuffer.template draw<Shader>(x, y, triangle, uniform_buffer, E, area);
					}
				}
			}
		}

		template<typename Shader, typename Framebuffer>
		inline const static function<Shader, Framebuffer> rasterize = execute<bbox_scan, Shader, Framebuffer>;
	};
}
