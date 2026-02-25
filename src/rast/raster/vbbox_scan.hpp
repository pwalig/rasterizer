#pragma once
#include "raster_utils.hpp"
#include "raster_output_interface.hpp"

namespace rast::raster {
	struct vbbox_scan {
		template <size_t Count = 4, typename Callable, typename VertexT, typename ...Args>
		inline static constexpr void rasterize_one(
			Callable&& output,
			const VertexT* triangle,
			const viewport& viewport,
			const tile& tile,
			Args&&... args
		) {
			glm::ivec2 a = to_screen_space(triangle[0].rastPos, viewport); // fixed point
			glm::ivec2 b = to_screen_space(triangle[1].rastPos, viewport);
			glm::ivec2 c = to_screen_space(triangle[2].rastPos, viewport);

			glm::ivec2 min = glm::ivec2( // int
				std::max((int)std::min({ a.x, b.x, c.x }), std::max(tile.min.x, viewport.offset.x)),
				std::max((int)std::min({ a.y, b.y, c.y }), std::max(tile.min.y, viewport.offset.y))
			) / 16;
			glm::ivec2 max = glm::ivec2( // int
				std::min<int>({ std::max({ a.x, b.x, c.x }) + 16, tile.max.x, viewport.offset.x + viewport.extent.x }),
				std::min<int>({ std::max({ a.y, b.y, c.y }) + 16, tile.max.y, viewport.offset.y + viewport.extent.y })
			) / 16;

			if (min.x >= max.x || min.y >= max.y) return;

			glm::ivec3 x012 = glm::ivec3(a.x, b.x, c.x); // fixed point
			glm::ivec3 x120 = glm::ivec3(b.x, c.x, a.x);

			glm::ivec3 y012 = glm::ivec3(a.y, b.y, c.y);
			glm::ivec3 y120 = glm::ivec3(b.y, c.y, a.y);

			glm::ivec3 Dx = x120 - x012;
			glm::ivec3 Dy = y120 - y012;

			int area = (Dy.x * Dx.z) - (Dx.x * Dy.z);
			if (area <= 0) return; // back face detected - early return

			// Dx * Y - fill_convention
			//glm::ivec3 Cy = Dx * (glm::ivec3(min.y << 4) - y012) - fill_convention(Dx, Dy); // fixed point
			//glm::ivec3 Cx = Dy * (glm::ivec3(min.x << 4) - x012);
			Dx *= 16;
			Dy *= 16;

			math::i32vec3x<Count> vDx = math::vectorize<3, Count>(Dx);
			math::i32vec3x<Count> vDy = math::vectorize<3, Count>(Dy);

			auto v0 = triangle[0].data.template vectorize<Count>();
			auto v1 = triangle[1].data.template vectorize<Count>();
			auto v2 = triangle[2].data.template vectorize<Count>();
			math::f32vec3x<Count> z = math::f32vec3x<Count>(
				math::vectorize<Count>(triangle[0].rastPos[2]),
				math::vectorize<Count>(triangle[1].rastPos[2]),
				math::vectorize<Count>(triangle[2].rastPos[2])
			);
			math::f32vec3x<Count> w = math::f32vec3x<Count>(
				math::vectorize<Count>(triangle[0].rastPos[3]),
				math::vectorize<Count>(triangle[1].rastPos[3]),
				math::vectorize<Count>(triangle[2].rastPos[3])
			);
			auto varea = math::f32x<Count>(static_cast<float>(area));

			math::i32x<4> x = math::make_i32x4(min.x, min.x + 1, min.x, min.x + 1);
			math::i32x<4> y = math::make_i32x4(min.y, min.y, min.y + 1, min.y + 1);
			math::i32vec3x<Count> vCy = vDx * (math::i32vec3x<Count>(y << math::vectorize<Count>(4)) - math::vectorize<3, Count>(y012));
			math::i32vec3x<Count> vCx = vDy * (math::i32vec3x<Count>(x << math::vectorize<Count>(4)) - math::vectorize<3, Count>(x012));

			math::i32x<4> increment = math::i32x<Count>(2);
			math::i32x<4> zero = math::i32x<Count>(0);

			for (; y[0] < max.y; y += increment, vCy += vDx) {
				math::i32vec3x<Count> E = vCy - vCx;
				for (; x[0] < max.x; x += increment, E -= vDy) {
					math::boolx<Count> mask = (E[0] >= zero) & (E[1] >= zero) & (E[2] >= zero);
					if (math::or_accross(mask)) {
						auto partial_coefs = math::f32vec3x<Count>(
							math::cast<float>(E[1]) / varea,
							math::cast<float>(E[2]) / varea,
							math::cast<float>(E[0]) / varea
						);
						output(math::cast<uint32_t>(x), math::cast<uint32_t>(y), v0, v1, v2, z, w, partial_coefs, args...); // can't forward args, because they are used multiple times
					}
				}
			}
		}

		template <typename Shader, typename Callable, typename ...Args>
		inline static constexpr void rasterize(
			Callable&& output,
			const typename Shader::vertex::output* vertex_begin,
			const typename Shader::vertex::output* vertex_end,
			const viewport& viewport, const tile& tile,
			Args&&... args
		) {
			using vertex = typename Shader::vertex::output;
			for (const vertex* triangle = vertex_begin; triangle != vertex_end; triangle += 3) {
				rasterize_one(output, triangle, viewport, tile, args...);
			}
		}
	};
}
