#pragma once
#include <glm/glm.hpp>
#include "../math/vec.hpp"

#include "../viewport.hpp"
#include "../tile.hpp"

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

	inline glm::vec<2, math::simd::i32x4> to_screen_space(
		__m128 x, __m128 y,
		__m128i extent_x, __m128i extent_y,
		__m128i offset_x, __m128i offset_y
	) {
		return {
			_mm_cvtps_epi32(_mm_fmadd_ps(
				_mm_add_ps(x, _mm_set_ps1(1.0f)),
				_mm_mul_ps(_mm_cvtepi32_ps(extent_x), _mm_set_ps1(0.5f)),
				_mm_cvtepi32_ps(offset_x)
			)),
			_mm_cvtps_epi32(_mm_fmadd_ps(
				_mm_sub_ps(_mm_set_ps1(1.0f), y),
				_mm_mul_ps(_mm_cvtepi32_ps(extent_y), _mm_set_ps1(0.5f)),
				_mm_cvtepi32_ps(offset_y)
			))
		};
	}

	template <size_t Count>
	inline constexpr math::i32vec2x<Count> to_screen_space(
		const math::f32x<Count>& x,
		const math::f32x<Count>& y,
		const viewport_x<Count>& Viewport
	) {
		return math::i32vec2x<Count>(
			(x + math::f32x<Count>(1.0f)) * math::cast<float>(Viewport.extent[0]) * math::f32x<Count>(0.5f) + math::cast<float>(Viewport.offset[0]),
			(-y + math::f32x<Count>(1.0f)) * math::cast<float>(Viewport.extent[1]) * math::f32x<Count>(0.5f) + math::cast<float>(Viewport.offset[1])
		);
	}
	template <size_t Count>
	inline constexpr math::i32vec2x<Count> to_screen_space(
		const math::f32x<Count>& x,
		const math::f32x<Count>& y,
		const viewport& Viewport
	) {
		return to_screen_space(x, y, viewport_x<Count>(Viewport.offset.x, Viewport.offset.y, Viewport.extent.x, Viewport.extent.y));
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
		static_assert(std::is_integral_v<typename T::value_type>);
		return T(
			(Dy.x > 0 || (Dy.x == 0 && Dx.x < 0)) ? 1 : 0,
			(Dy.y > 0 || (Dy.y == 0 && Dx.y < 0)) ? 1 : 0,
			(Dy.z > 0 || (Dy.z == 0 && Dx.z < 0)) ? 1 : 0
		);
	}

	inline math::simd::i32x4 fill_convention(
		math::simd::i32x4 x,
		math::simd::i32x4 y
	) {
		auto zero = _mm_setzero_si128();
		// 0 - ((y > 0) | ((y == 0) & (x < 0)))
		return _mm_sub_epi32( // subtracting from 0 to turn -1 into 1
			zero, // because cmp instructions fill register with ones or zeroes
			_mm_or_si128( // register of just ones is -1 if treated as signed
				_mm_cmpgt_epi32(y, zero),
				_mm_and_si128(
					_mm_cmpeq_epi32(y, zero),
					_mm_cmplt_epi32(x, zero)
				)
			)
		);
	}

	inline glm::vec<3, math::simd::i32x4> fill_convention(
		glm::vec<3, math::simd::i32x4> Dx,
		glm::vec<3, math::simd::i32x4> Dy
	) {
		return glm::vec<3, math::simd::i32x4>(
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

	template <typename Rasterizer, typename Shader, typename Callable>
	inline void execute(
		Callable&& output,
		const typename Shader::vertex::output* vertex_begin,
		const typename Shader::vertex::output* vertex_end,
		const viewport& viewport,
		const tile& tile
	) {
		using vertex = typename Shader::vertex::output;
		for (const vertex* triangle = vertex_begin; triangle != vertex_end; triangle += 3) {
			Rasterizer::template rasterize_one<Shader>(output, triangle, viewport, tile);
		}
	}
	template <auto RasterizeOne, typename Callable, typename VertexT, typename ...Args>
	inline static constexpr void filter_triangles(
		Callable&& output,
		const VertexT* vertex_begin,
		const VertexT* vertex_end,
		const viewport& viewport,
		const tile& tile,
		Args&&... args
	) {
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

			glm::ivec3 x120 = glm::ivec3(v1.x, v2.x, v0.x);
			glm::ivec3 x201 = glm::ivec3(v2.x, v0.x, v1.x);

			glm::ivec3 y120 = glm::ivec3(v1.y, v2.y, v0.y);
			glm::ivec3 y201 = glm::ivec3(v2.y, v0.y, v1.y);

			glm::ivec3 Dx = x201 - x120;
			glm::ivec3 Dy = y201 - y120;

			int area = (Dy.x * Dx.z) - (Dx.x * Dy.z);
			if (area <= 0) continue; // back face detected - early return

			glm::ivec3 Cy = Dx * (glm::ivec3(min.y << 4) - y120) - fill_convention(Dx, Dy);
			glm::ivec3 Cx = Dy * (glm::ivec3(min.x << 4) - x120);
			Dx *= 16;
			Dy *= 16;

			RasterizeOne(output, vertices, Cx, Cy, Dx, Dy, min, max, args...);
		}
	}

	template <auto RasterizeOne, typename Callable, typename VertexT, typename ...Args>
	inline static void filter_triangles_x4(
		Callable&& output,
		const VertexT* vertex_begin,
		const VertexT* vertex_end,
		const viewport& viewport,
		const tile& tile,
		Args&&... args
	) {
		constexpr size_t Count = 4;
		using i32 = math::simd::i32x_<Count>;

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

			glm::vec<2, i32> v0 = to_screen_space(
				math::simd::make_x4<float>(vertices[9].rastPos.x, vertices[6].rastPos.x, vertices[3].rastPos.x, vertices[0].rastPos.x),
				math::simd::make_x4<float>(vertices[9].rastPos.y, vertices[6].rastPos.y, vertices[3].rastPos.y, vertices[0].rastPos.y),
				extent.x, extent.y, offset.x, offset.y
			);
			glm::vec<2, i32> v1 = to_screen_space(
				math::simd::make_x4<float>(vertices[10].rastPos.x, vertices[7].rastPos.x, vertices[4].rastPos.x, vertices[1].rastPos.x),
				math::simd::make_x4<float>(vertices[10].rastPos.y, vertices[7].rastPos.y, vertices[4].rastPos.y, vertices[1].rastPos.y),
				extent.x, extent.y, offset.x, offset.y
			);
			glm::vec<2, i32> v2 = to_screen_space(
				math::simd::make_x4<float>(vertices[11].rastPos.x, vertices[8].rastPos.x, vertices[5].rastPos.x, vertices[2].rastPos.x),
				math::simd::make_x4<float>(vertices[11].rastPos.y, vertices[8].rastPos.y, vertices[5].rastPos.y, vertices[2].rastPos.y),
				extent.x, extent.y, offset.x, offset.y
			);

			auto min = glm::vec<2, i32>(
				_mm_srai_epi32(_mm_max_epi32(_mm_min_epi32(v0.x, _mm_min_epi32(v1.x, v2.x)), _mm_max_epi32(tile_min.x, offset.x)), 4),
				_mm_srai_epi32(_mm_max_epi32(_mm_min_epi32(v0.y, _mm_min_epi32(v1.y, v2.y)), _mm_max_epi32(tile_min.y, offset.y)), 4)
			);
			auto max = glm::vec<2, i32>(
				_mm_srai_epi32(_mm_min_epi32(
					_mm_add_epi32(_mm_max_epi32(v0.x, _mm_max_epi32(v1.x, v2.x)), _mm_set1_epi32(16)),
					_mm_min_epi32(tile_max.x, _mm_add_epi32(offset.x, extent.x))
				), 4),
				_mm_srai_epi32(_mm_min_epi32(
					_mm_add_epi32(_mm_max_epi32(v0.y, _mm_max_epi32(v1.y, v2.y)), _mm_set1_epi32(16)),
					_mm_min_epi32(tile_max.y, _mm_add_epi32(offset.y, extent.y))
				), 4)
			);
			alignas(Count * sizeof(int)) int min_mem[2][Count];
			alignas(Count * sizeof(int)) int max_mem[2][Count];
			math::simd::store(min_mem[0], min.x);
			math::simd::store(min_mem[1], min.y);
			math::simd::store(max_mem[0], max.x);
			math::simd::store(max_mem[1], max.y);

			auto Dx = glm::vec<3, i32>(
				v2.x - v1.x,
				v0.x - v2.x,
				v1.x - v0.x
			);
			auto Dy = glm::vec<3, i32>(
				v2.y - v1.y,
				v0.y - v2.y,
				v1.y - v0.y
			);
			i32 area = (Dy.x * Dx.z) - (Dx.x * Dy.z);
			glm::vec<3, i32> Cy = Dx * (glm::vec<3, i32>(min.y << 4) - glm::vec<3, i32>(v1.y, v2.y, v0.y)) - fill_convention(Dx, Dy);
			glm::vec<3, i32> Cx = Dy * (glm::vec<3, i32>(min.x << 4) - glm::vec<3, i32>(v1.x, v2.x, v0.x));
			Dx *= math::simd::i32x_<Count>(16);
			Dy *= math::simd::i32x_<Count>(16);

			alignas(Count * sizeof(int)) int Dx_mem[3][Count];
			alignas(Count * sizeof(int)) int Dy_mem[3][Count];
			math::simd::store(Dx_mem[0], Dx.x);
			math::simd::store(Dx_mem[1], Dx.y);
			math::simd::store(Dx_mem[2], Dx.z);
			math::simd::store(Dy_mem[0], Dy.x);
			math::simd::store(Dy_mem[1], Dy.y);
			math::simd::store(Dy_mem[2], Dy.z);

			alignas(Count * sizeof(int)) int Cx_mem[3][Count];
			alignas(Count * sizeof(int)) int Cy_mem[3][Count];
			math::simd::store(Cx_mem[0], Cx.x);
			math::simd::store(Cx_mem[1], Cx.y);
			math::simd::store(Cx_mem[2], Cx.z);
			math::simd::store(Cy_mem[0], Cy.x);
			math::simd::store(Cy_mem[1], Cy.y);
			math::simd::store(Cy_mem[2], Cy.z);

			alignas(Count * sizeof(int)) int area_mem[Count];
			math::simd::store(area_mem, area);


			// for some reason calculating comparisons for all 4 triangles at once produces slower executable,
			// because MSVC compiler generates a waaaay slower moving of data to argument registers for RasterizeOne call,
			// as if it forgot that data is already on the stack it moves it back and fourth between stack and simd registers
			//i32 mask = _mm_or_epi32(_mm_or_epi32(
			//	_mm_cmpgt_epi32(max.x, min.x),
			//	_mm_cmpgt_epi32(max.y, min.y)
			//), _mm_cmpgt_epi32(area, _mm_setzero_si128()));
			//alignas(Count * sizeof(int)) int mask_mem[Count];
			//math::simd::store(mask_mem, mask);

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
		filter_triangles<RasterizeOne>(output, vertices, vertex_end, viewport, tile, args...);
	}
}
