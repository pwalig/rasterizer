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
			math::i32vec3x<Count> vDx = math::vectorize<3, Count>(Dx);
			math::i32vec3x<Count> vDy = math::vectorize<3, Count>(Dy);

			int area = (Dy.x * Dx.z) - (Dx.x * Dy.z);
			if (area <= 0) return; // back face detected - early return

			// Dx * Y - fill_convention
			//glm::ivec3 Cy = Dx * (glm::ivec3(min.y << 4) - y012);// -fill_convention(Dx, Dy); // fixed point
			//glm::ivec3 Cx = Dy * (glm::ivec3(min.x << 4) - x012);

			math::i32x<4> x_base = math::make_i32x4(min.x, min.x + 1, min.x, min.x + 1);
			math::i32x<4> y_base = math::make_i32x4(min.y, min.y, min.y + 1, min.y + 1);
			//auto test0 = math::vectorize<3, Count>(y012);
			//auto test05 = math::i32vec3x<Count>(y_base << math::vectorize<Count>(4));
			//auto test = test05 - test0;
			//auto test1 = (glm::ivec3(min.y << 4) - y012);

			math::i32vec3x<Count> vCy = vDx * (math::i32vec3x<Count>(y_base << math::vectorize<Count>(4)) - math::vectorize<3, Count>(y012));
			math::i32vec3x<Count> vCx = vDy * (math::i32vec3x<Count>(x_base << math::vectorize<Count>(4)) - math::vectorize<3, Count>(x012));

			//assert(vCy[0][0] == Cy.x);
			//assert(vCy[1][0] == Cy.y);
			//assert(vCy[2][0] == Cy.z);

			//Dx *= 16;
			//Dy *= 16;
			vDx *= math::vectorize<Count>(16);
			vDy *= math::vectorize<Count>(16);


			auto v0 = triangle[0].data.template vectorize<Count>();
			auto v1 = triangle[1].data.template vectorize<Count>();
			auto v2 = triangle[2].data.template vectorize<Count>();
			math::f32vec3x<Count> z = math::f32vec3x<Count>(
				math::vectorize<Count>(triangle[0].rastPos.z),
				math::vectorize<Count>(triangle[1].rastPos.z),
				math::vectorize<Count>(triangle[2].rastPos.z)
			);
			math::f32vec3x<Count> w = math::f32vec3x<Count>(
				math::vectorize<Count>(triangle[0].rastPos.w),
				math::vectorize<Count>(triangle[1].rastPos.w),
				math::vectorize<Count>(triangle[2].rastPos.w)
			);
			auto varea = math::f32x<Count>(static_cast<float>(area));

			math::i32x<4> increment = math::i32x<Count>(2);
			//Dx *= 2;
			//Dy *= 2;
			vDx *= increment;
			vDy *= increment;
			math::i32x<4> zero = math::i32x<Count>(0);

			auto vmax = math::vectorize<2, Count>(max);

			for (math::i32x<4> y = y_base; y[0] < max.y; y += increment, vCy += vDx) {
				math::i32vec3x<Count> vE = vCy - vCx;
				for (math::i32x<4> x = x_base; x[0] < max.x; x += increment, vE -= vDy) {
					math::boolx<Count> mask = (vE[0] >= zero) && (vE[1] >= zero) && (vE[2] >= zero);
					if (math::or_accross(mask)) {
						auto vpartials = math::f32vec3x<Count>(
							math::cast<float>(vE[1]) / varea,
							math::cast<float>(vE[2]) / varea,
							math::cast<float>(vE[0]) / varea
						);
						output(math::cast<uint32_t>(x), math::cast<uint32_t>(y), v0, v1, v2, z, w, vpartials, mask && (y < vmax[1]) && (x < vmax[0]), args...); // can't forward args, because they are used multiple times
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
