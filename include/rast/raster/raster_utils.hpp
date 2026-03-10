#pragma once
#include <glm/glm.hpp>
#include "../math/vec.hpp"
#include "../simd.hpp"

#include "../viewport.hpp"
#include "../tile.hpp"
#include "../cull.hpp"

namespace rast::raster {
	template <typename Int, typename Float>
	inline constexpr Int to_screen_space(Float x, Int offset, Int extent) {
		return static_cast<Int>((x + Float(1.0)) * static_cast<Float>(extent) * Float(0.5) + static_cast<Float>(offset));
	}

	inline constexpr glm::ivec2 to_screen_space(const glm::vec4& vertex, const viewport& viewport) {
		return glm::ivec2(
			to_screen_space(vertex.x, viewport.offset.x, viewport.extent.x),
			to_screen_space(-vertex.y, viewport.offset.y, viewport.extent.y)
		);
	}

	template <size_t Count>
	inline glm::vec<2, simd::i32x_<Count>> to_screen_space(
		simd::f32x_<Count> x, simd::f32x_<Count> y,
		simd::i32x_<Count> extent_x, simd::i32x_<Count> extent_y,
		simd::i32x_<Count> offset_x, simd::i32x_<Count> offset_y
	) {
		return {
			simd::cast<int32_t>(simd::fmadd(
				x + simd::f32x_<Count>(1.0f),
				simd::cast<float>(extent_x) * simd::f32x_<Count>(0.5f),
				simd::cast<float>(offset_x)
			)),
			simd::cast<int32_t>(simd::fmadd(
				simd::f32x_<Count>(1.0f) - y,
				simd::cast<float>(extent_y) * simd::f32x_<Count>(0.5f),
				simd::cast<float>(offset_y)
			))
		};
	}

	template <typename Shader, typename Callable>
	using function = void(*)(
		Callable&& output,
		const typename Shader::vertex::output* vertex_begin,
		const typename Shader::vertex::output* vertex_end,
		const viewport& viewport,
		const tile& tile
	);

	template <typename T>
	inline constexpr T fill_convention(T Dx, T Dy) {
		static_assert(std::is_integral_v<T>);
		return T(
			(Dy > 0 || (Dy == 0 && Dx < 0)) ? 1 : 0
		);
	}

	template <size_t Count>
	inline simd::i32x_<Count> fill_convention(
		simd::i32x_<Count> x,
		simd::i32x_<Count> y
	) {
		simd::i32x_<Count> zero = simd::setzero<int, Count>();
		// 0 - ((y > 0) | ((y == 0) & (x < 0)))
		// subtracting from 0 to turn -1 into 1
		// because cmp instructions fill register with ones or zeroes
		// register of just ones is -1 if treated as signed
		return zero - ((y > zero) | ((y == zero) & (x < zero)));
	}

	template <typename T>
	inline glm::vec<3, T> fill_convention(
		glm::vec<3, T> Dx,
		glm::vec<3, T> Dy
	) {
		return glm::vec<3, T>(
			fill_convention(Dx.x, Dy.x),
			fill_convention(Dx.y, Dy.y),
			fill_convention(Dx.z, Dy.z)
		);
	}

	template <size_t Count>
	inline constexpr math::i32vec3x<Count> fill_convention(math::i32vec3x<Count> Dx, math::i32vec3x<Count> Dy) {
		auto zero = math::i32x<Count>(0);
		return math::i32vec3x<Count>(
			math::cast<int>(Dy[0] > zero || (Dy[0] == zero && (Dx[0] < zero))),
			math::cast<int>(Dy[1] > zero || (Dy[1] == zero && (Dx[1] < zero))),
			math::cast<int>(Dy[2] > zero || (Dy[2] == zero && (Dx[2] < zero)))
		);
	}

	template <auto RasterizeOne, cull Cull = cull_default, typename Callable, typename VertexT, typename ...Args>
	inline static constexpr void filter_triangles(
		Callable&& output,
		const VertexT* vertex_begin,
		const VertexT* vertex_end,
		const viewport& viewport,
		const tile& tile,
		const Args&... args
	) {
		if constexpr (Cull != cull::both)
		for (const VertexT* vertices = vertex_begin; vertices + 3 <= vertex_end; vertices += 3) {
			glm::ivec2 v0 = to_screen_space(vertices[0].rastPos, viewport);
			glm::ivec2 v1 = to_screen_space(vertices[1].rastPos, viewport);
			glm::ivec2 v2 = to_screen_space(vertices[2].rastPos, viewport);

			glm::ivec2 min = glm::ivec2(
				std::max((int)std::min({ v0.x, v1.x, v2.x }), std::max(tile.min.x, viewport.offset.x)),
				std::max((int)std::min({ v0.y, v1.y, v2.y }), std::max(tile.min.y, viewport.offset.y))
			) / 16;
			glm::ivec2 max = glm::ivec2(
				std::min<int>({ std::max({ v0.x, v1.x, v2.x }) + 16, tile.max.x, viewport.offset.x + viewport.extent.x }),
				std::min<int>({ std::max({ v0.y, v1.y, v2.y }) + 16, tile.max.y, viewport.offset.y + viewport.extent.y })
			) / 16;

			if (min.x >= max.x || min.y >= max.y) continue;

			glm::ivec3 Dx, Dy, xSrc, ySrc;
			if constexpr (Cull == cull::clockwise) {
				xSrc = glm::ivec3(v2.x, v0.x, v1.x);
				ySrc = glm::ivec3(v2.y, v0.y, v1.y);
				Dx = glm::ivec3(v1.x, v2.x, v0.x) - xSrc;
				Dy = glm::ivec3(v1.y, v2.y, v0.y) - ySrc;
				int area = (Dy.x * Dx.y) - (Dx.x * Dy.y);
				if (area <= 0) continue; // back face detected - early return
			}
			else {
				xSrc = glm::ivec3(v1.x, v2.x, v0.x);
				ySrc = glm::ivec3(v1.y, v2.y, v0.y);
				Dx = glm::ivec3(v2.x, v0.x, v1.x) - xSrc;
				Dy = glm::ivec3(v2.y, v0.y, v1.y) - ySrc;
				int area = (Dx.x * Dy.y) - (Dy.x * Dx.y);
				if constexpr (Cull == cull::none) {
					if (area == 0) continue;
					else if (area < 0) {
						Dx *= -1;
						Dy *= -1;
						xSrc = glm::ivec3(v2.x, v0.x, v1.x);
						ySrc = glm::ivec3(v2.y, v0.y, v1.y);
					}
				}
				else if (area <= 0) continue; // back face detected - early return
			}
			glm::ivec3 Cy = Dx * (glm::ivec3(min.y << 4) - ySrc) - fill_convention(Dx, Dy);
			glm::ivec3 Cx = Dy * (glm::ivec3(min.x << 4) - xSrc);
			RasterizeOne(output, vertices, Cx, Cy, Dx * 16, Dy * 16, min, max, args...);
		}
	}

	template <size_t Count, auto RasterizeOne, cull Cull = cull_default, typename Callable, typename VertexT, typename ...Args>
	inline static void filter_triangles_x(
		Callable&& output,
		const VertexT* vertex_begin,
		const VertexT* vertex_end,
		const viewport& viewport,
		const tile& tile,
		const Args&... args
	) {
		static_assert(Cull != cull::none, "rast::cull::none is not yet implemented for filter_triangles_x, if you want to use cull::none use filter_triangles instead");
		if constexpr (Cull != cull::both) {
			using i32 = simd::i32x_<Count>;

			auto offset = glm::vec<2, i32>(
				viewport.offset.x, viewport.offset.y
			);
			auto extent = glm::vec<2, i32>(
				viewport.extent.x, viewport.extent.y
			);
			auto tile_min = glm::vec<2, i32>(
				tile.min.x, tile.min.y
			);
			auto tile_max = glm::vec<2, i32>(
				tile.max.x, tile.max.y
			);
			const VertexT* vertices = vertex_begin;
			for (; vertices + (3 * Count) <= vertex_end; vertices += (3 * Count)) {

				glm::vec<2, i32> v0, v1, v2;
				static_assert(Count == 4 || Count == 8);
				if constexpr (Count == 4) {
					v0 = to_screen_space(
						simd::make_x4<float>(vertices[9].rastPos.x, vertices[6].rastPos.x, vertices[3].rastPos.x, vertices[0].rastPos.x),
						simd::make_x4<float>(vertices[9].rastPos.y, vertices[6].rastPos.y, vertices[3].rastPos.y, vertices[0].rastPos.y),
						extent.x, extent.y, offset.x, offset.y
					);
					v1 = to_screen_space(
						simd::make_x4<float>(vertices[10].rastPos.x, vertices[7].rastPos.x, vertices[4].rastPos.x, vertices[1].rastPos.x),
						simd::make_x4<float>(vertices[10].rastPos.y, vertices[7].rastPos.y, vertices[4].rastPos.y, vertices[1].rastPos.y),
						extent.x, extent.y, offset.x, offset.y
					);
					v2 = to_screen_space(
						simd::make_x4<float>(vertices[11].rastPos.x, vertices[8].rastPos.x, vertices[5].rastPos.x, vertices[2].rastPos.x),
						simd::make_x4<float>(vertices[11].rastPos.y, vertices[8].rastPos.y, vertices[5].rastPos.y, vertices[2].rastPos.y),
						extent.x, extent.y, offset.x, offset.y
					);
				}
				if constexpr (Count == 8) {
					v0 = to_screen_space(
						simd::make_x8<float>(
							vertices[21].rastPos.x, vertices[18].rastPos.x, vertices[15].rastPos.x, vertices[12].rastPos.x,
							vertices[9].rastPos.x, vertices[6].rastPos.x, vertices[3].rastPos.x, vertices[0].rastPos.x
						),
						simd::make_x8<float>(
							vertices[21].rastPos.y, vertices[18].rastPos.y, vertices[15].rastPos.y, vertices[12].rastPos.y,
							vertices[9].rastPos.y, vertices[6].rastPos.y, vertices[3].rastPos.y, vertices[0].rastPos.y
						),
						extent.x, extent.y, offset.x, offset.y
					);
					v1 = to_screen_space(
						simd::make_x8<float>(
							vertices[22].rastPos.x, vertices[19].rastPos.x, vertices[16].rastPos.x, vertices[13].rastPos.x,
							vertices[10].rastPos.x, vertices[7].rastPos.x, vertices[4].rastPos.x, vertices[1].rastPos.x
						),
						simd::make_x8<float>(
							vertices[22].rastPos.y, vertices[19].rastPos.y, vertices[16].rastPos.y, vertices[13].rastPos.y,
							vertices[10].rastPos.y, vertices[7].rastPos.y, vertices[4].rastPos.y, vertices[1].rastPos.y
						),
						extent.x, extent.y, offset.x, offset.y
					);
					v2 = to_screen_space(
						simd::make_x8<float>(
							vertices[23].rastPos.x, vertices[20].rastPos.x, vertices[17].rastPos.x, vertices[14].rastPos.x,
							vertices[11].rastPos.x, vertices[8].rastPos.x, vertices[5].rastPos.x, vertices[2].rastPos.x
						),
						simd::make_x8<float>(
							vertices[23].rastPos.y, vertices[20].rastPos.y, vertices[17].rastPos.y, vertices[14].rastPos.y,
							vertices[11].rastPos.y, vertices[8].rastPos.y, vertices[5].rastPos.y, vertices[2].rastPos.y
						),
						extent.x, extent.y, offset.x, offset.y
					);
				}
				auto min = glm::vec<2, i32>(
					simd::max(simd::min(v0.x, simd::min(v1.x, v2.x)), simd::max(tile_min.x, offset.x)) >> 4,
					simd::max(simd::min(v0.y, simd::min(v1.y, v2.y)), simd::max(tile_min.y, offset.y)) >> 4
				);
				auto max = glm::vec<2, i32>(
					simd::min(
						simd::max(v0.x, simd::max(v1.x, v2.x)) + i32(16),
						simd::min(tile_max.x, offset.x + extent.x)
					) >> 4,
					simd::min(
						simd::max(v0.y, simd::max(v1.y, v2.y)) + i32(16),
						simd::min(tile_max.y, offset.y + extent.y)
					) >> 4
				);
				alignas(Count * sizeof(int)) int min_mem[2][Count];
				alignas(Count * sizeof(int)) int max_mem[2][Count];
				simd::store(min_mem[0], min.x);
				simd::store(min_mem[1], min.y);
				simd::store(max_mem[0], max.x);
				simd::store(max_mem[1], max.y);

				glm::vec<3, i32> Dx, Dy, xSrc, ySrc;
				i32 area;
				if constexpr (Cull == cull::clockwise) {
					xSrc = glm::vec<3, i32>(v2.x, v0.x, v1.x);
					ySrc = glm::vec<3, i32>(v2.y, v0.y, v1.y);
					Dx = glm::vec<3, i32>(v1.x, v2.x, v0.x) - xSrc;
					Dy = glm::vec<3, i32>(v1.y, v2.y, v0.y) - ySrc;
					area = (Dy.x * Dx.y) - (Dx.x * Dy.y);
				}
				else {
					xSrc = glm::vec<3, i32>(v1.x, v2.x, v0.x);
					ySrc = glm::vec<3, i32>(v1.y, v2.y, v0.y);
					Dx = glm::vec<3, i32>(v2.x, v0.x, v1.x) - xSrc;
					Dy = glm::vec<3, i32>(v2.y, v0.y, v1.y) - ySrc;
					area = (Dx.x * Dy.y) - (Dy.x * Dx.y);
				}
				glm::vec<3, i32> Cy = Dx * (glm::vec<3, i32>(min.y << 4) - ySrc) - fill_convention(Dx, Dy);
				glm::vec<3, i32> Cx = Dy * (glm::vec<3, i32>(min.x << 4) - xSrc);
				Dx *= simd::i32x_<Count>(16);
				Dy *= simd::i32x_<Count>(16);

				alignas(Count * sizeof(int)) int Dx_mem[3][Count];
				alignas(Count * sizeof(int)) int Dy_mem[3][Count];
				simd::store(Dx_mem[0], Dx.x);
				simd::store(Dx_mem[1], Dx.y);
				simd::store(Dx_mem[2], Dx.z);
				simd::store(Dy_mem[0], Dy.x);
				simd::store(Dy_mem[1], Dy.y);
				simd::store(Dy_mem[2], Dy.z);

				alignas(Count * sizeof(int)) int Cx_mem[3][Count];
				alignas(Count * sizeof(int)) int Cy_mem[3][Count];
				simd::store(Cx_mem[0], Cx.x);
				simd::store(Cx_mem[1], Cx.y);
				simd::store(Cx_mem[2], Cx.z);
				simd::store(Cy_mem[0], Cy.x);
				simd::store(Cy_mem[1], Cy.y);
				simd::store(Cy_mem[2], Cy.z);

				alignas(Count * sizeof(int)) int area_mem[Count];
				simd::store(area_mem, area);


				// for some reason calculating comparisons for all 4 triangles at once produces slower executable,
				// because MSVC compiler generates a waaaay slower moving of data to argument registers for RasterizeOne call,
				// as if it forgot that data is already on the stack it moves it back and fourth between stack and simd registers
				//i32 mask = _mm_or_epi32(_mm_or_epi32(
				//	_mm_cmpgt_epi32(max.x, min.x),
				//	_mm_cmpgt_epi32(max.y, min.y)
				//), _mm_cmpgt_epi32(area, _mm_setzero_si128()));
				//alignas(Count * sizeof(int)) int mask_mem[Count];
				//simd::store(mask_mem, mask);

				for (size_t i = 0; i < Count; ++i) {
					//if (mask_mem[i]) // checking against the precalculated mask causes the slowdown
					if (min_mem[0][i] < max_mem[0][i] && min_mem[1][i] < max_mem[1][i] && area_mem[i] > 0)
						RasterizeOne(
							output, vertices + (i * 3), 
							glm::ivec3(Cx_mem[0][i], Cx_mem[1][i], Cx_mem[2][i]),
							glm::ivec3(Cy_mem[0][i], Cy_mem[1][i], Cy_mem[2][i]),
							glm::ivec3(Dx_mem[0][i], Dx_mem[1][i], Dx_mem[2][i]),
							glm::ivec3(Dy_mem[0][i], Dy_mem[1][i], Dy_mem[2][i]),
							glm::ivec2(min_mem[0][i], min_mem[1][i]),
							glm::ivec2(max_mem[0][i], max_mem[1][i]),
							args...
						);
				}
			}
			if (vertices != vertex_end && vertices - (3 * Count) >= vertex_begin) vertices -= (3 * Count);
			filter_triangles<RasterizeOne, Cull>(output, vertices, vertex_end, viewport, tile, args...);
		}
	}
}
