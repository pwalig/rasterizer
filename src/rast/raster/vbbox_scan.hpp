#pragma once
#include "raster_utils.hpp"

namespace rast::raster {
	struct vbbox_scan {
		template <typename Callable, typename VertexT, typename ...Args>
		inline static void rasterize_one(
			Callable&& output,
			const VertexT* triangle,
			const viewport& viewport,
			const tile& tile,
			const Args&... args
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
			if (area <= 0) return; // back face detected - early return

			__m128 varea = _mm_set_ps1(static_cast<float>(area));
			math::sse::ivec3 vDx = {
				_mm_set1_epi32(Dx.x),
				_mm_set1_epi32(Dx.y),
				_mm_set1_epi32(Dx.z)
			};
			math::sse::ivec3 vDy = {
				_mm_set1_epi32(Dy.x),
				_mm_set1_epi32(Dy.y),
				_mm_set1_epi32(Dy.z)
			};

			__m128i miny = _mm_add_epi32(_mm_set1_epi32(min.y), _mm_set_epi32(0, 0, 1, 1));
			__m128i minx = _mm_add_epi32(_mm_set1_epi32(min.x), _mm_set_epi32(0, 1, 0, 1));
			__m128i increment = _mm_set1_epi32(2);

			// Dx * Y - fill_convention
			glm::ivec3 Cy = Dx * (glm::ivec3(min.y << 4) - y012) - fill_convention(Dx, Dy);
			glm::ivec3 Cx = Dy * (glm::ivec3(min.x << 4) - x012);
			glm::ivec3 fill_conv = fill_convention(Dx, Dy);
			math::sse::ivec3 vCy = {
				_mm_sub_epi32(_mm_mul_epi32(vDx.x, _mm_sub_epi32(_mm_slli_epi32(miny, 4), _mm_set1_epi32(y012.x))), _mm_set1_epi32(fill_conv.x)),
				_mm_sub_epi32(_mm_mul_epi32(vDx.y, _mm_sub_epi32(_mm_slli_epi32(miny, 4), _mm_set1_epi32(y012.y))), _mm_set1_epi32(fill_conv.y)),
				_mm_sub_epi32(_mm_mul_epi32(vDx.z, _mm_sub_epi32(_mm_slli_epi32(miny, 4), _mm_set1_epi32(y012.z))), _mm_set1_epi32(fill_conv.z)),
			};
			math::sse::ivec3 vCx = {
				_mm_mul_epi32(vDy.x, _mm_sub_epi32(_mm_slli_epi32(minx, 4), _mm_set1_epi32(x012.x))),
				_mm_mul_epi32(vDy.y, _mm_sub_epi32(_mm_slli_epi32(minx, 4), _mm_set1_epi32(x012.y))),
				_mm_mul_epi32(vDy.z, _mm_sub_epi32(_mm_slli_epi32(minx, 4), _mm_set1_epi32(x012.z)))
			};
			Dx *= 16;
			Dy *= 16;
			_mm_slli_epi32(vDx.x, 5);
			_mm_slli_epi32(vDx.y, 5);
			_mm_slli_epi32(vDx.z, 5);
			_mm_slli_epi32(vDy.x, 5);
			_mm_slli_epi32(vDy.y, 5);
			_mm_slli_epi32(vDy.z, 5);

			for (__m128i y = miny; _mm_extract_epi32(y, 0) < max.y; y = _mm_add_epi32(y, increment), vCy += vDx, Cy += Dx * 2) {
				glm::ivec3 E = Cy - Cx;
				math::sse::ivec3 vE = vCy - vCx;
				for (__m128i x = minx; _mm_extract_epi32(x, 0) < max.x; x = _mm_add_epi32(x, increment), vE -= vDy, E -= Dy * 2) {
					if (E.x >= 0 && E.y >= 0 && E.z >= 0) {
						output(x, y, triangle, partial_coefs(vE.x, vE.y, vE.z, varea), args...);
					}
				}
			}
		}
		template <typename Callable, typename VertexT, typename ...Args>
		inline static void rasterize_one(
			Callable&& output,
			const VertexT* triangle,
			__m128 area,
			//__m128i v0x, __m128i v1x, __m128i v2x, 
			//__m128i v0y, __m128i v1y, __m128i v2y, 
			glm::ivec3 x012, glm::ivec3 y012,
			glm::ivec3 Dx, glm::ivec3 Dy,
			glm::ivec2 min, glm::ivec2 max,
			const Args&... args
		) {
			//glm::ivec2 a = to_screen_space(triangle[0].rastPos, viewport);
			//glm::ivec2 b = to_screen_space(triangle[1].rastPos, viewport);
			//glm::ivec2 c = to_screen_space(triangle[2].rastPos, viewport);

			//glm::ivec2 min = glm::ivec2(
			//	std::max((int)std::min({ a.x, b.x, c.x }), std::max(tile.min.x, viewport.offset.x)),
			//	std::max((int)std::min({ a.y, b.y, c.y }), std::max(tile.min.y, viewport.offset.y))
			//) / 16;
			//glm::ivec2 max = glm::ivec2(
			//	std::min<int>({ std::max({ a.x, b.x, c.x }) + 16, tile.max.x, viewport.offset.x + viewport.extent.x }),
			//	std::min<int>({ std::max({ a.y, b.y, c.y }) + 16, tile.max.y, viewport.offset.y + viewport.extent.y })
			//) / 16;

			//if (min.x >= max.x || min.y >= max.y) return;

			//glm::ivec3 x012 = glm::ivec3(a.x, b.x, c.x);
			//glm::ivec3 x120 = glm::ivec3(b.x, c.x, a.x);

			//glm::ivec3 y012 = glm::ivec3(a.y, b.y, c.y);
			//glm::ivec3 y120 = glm::ivec3(b.y, c.y, a.y);

			//glm::ivec3 Dx = x120 - x012;
			//glm::ivec3 Dy = y120 - y012;

			//int area = (Dy.x * Dx.z) - (Dx.x * Dy.z);
			//if (area <= 0) return; // back face detected - early return

			//__m128i varea = _mm_set1_epi32(area);
			math::sse::ivec3 vDx = {
				_mm_set1_epi32(Dx.x),
				_mm_set1_epi32(Dx.y),
				_mm_set1_epi32(Dx.z)
			};
			math::sse::ivec3 vDy = {
				_mm_set1_epi32(Dy.x),
				_mm_set1_epi32(Dy.y),
				_mm_set1_epi32(Dy.z)
			};

			__m128i miny = _mm_add_epi32(_mm_set1_epi32(min.y), _mm_set_epi32(0, 0, 1, 1));
			__m128i minx = _mm_add_epi32(_mm_set1_epi32(min.x), _mm_set_epi32(0, 1, 0, 1));
			__m128i maxy = _mm_set1_epi32(max.y);
			__m128i maxx = _mm_set1_epi32(max.x);
			__m128i increment = _mm_set1_epi32(2);

			// Dx * Y - fill_convention
			glm::ivec3 Cy = Dx * (glm::ivec3(min.y << 4) - y012) - fill_convention(Dx, Dy);
			glm::ivec3 Cx = Dy * (glm::ivec3(min.x << 4) - x012);
			glm::ivec3 fill_conv = fill_convention(Dx, Dy);
			math::sse::ivec3 vCy = {
				//_mm_mullo_epi32(vDx.x, _mm_sub_epi32(_mm_slli_epi32(miny, 4), _mm_set1_epi32(y012.x))),
				//_mm_mullo_epi32(vDx.y, _mm_sub_epi32(_mm_slli_epi32(miny, 4), _mm_set1_epi32(y012.y))),
				//_mm_mullo_epi32(vDx.z, _mm_sub_epi32(_mm_slli_epi32(miny, 4), _mm_set1_epi32(y012.z)))
				_mm_sub_epi32(_mm_mullo_epi32(vDx.x, _mm_sub_epi32(_mm_slli_epi32(miny, 4), _mm_set1_epi32(y012.x))), _mm_set1_epi32(fill_conv.x)),
				_mm_sub_epi32(_mm_mullo_epi32(vDx.y, _mm_sub_epi32(_mm_slli_epi32(miny, 4), _mm_set1_epi32(y012.y))), _mm_set1_epi32(fill_conv.y)),
				_mm_sub_epi32(_mm_mullo_epi32(vDx.z, _mm_sub_epi32(_mm_slli_epi32(miny, 4), _mm_set1_epi32(y012.z))), _mm_set1_epi32(fill_conv.z)),
			};
			math::sse::ivec3 vCx = {
				_mm_mullo_epi32(vDy.x, _mm_sub_epi32(_mm_slli_epi32(minx, 4), _mm_set1_epi32(x012.x))),
				_mm_mullo_epi32(vDy.y, _mm_sub_epi32(_mm_slli_epi32(minx, 4), _mm_set1_epi32(x012.y))),
				_mm_mullo_epi32(vDy.z, _mm_sub_epi32(_mm_slli_epi32(minx, 4), _mm_set1_epi32(x012.z)))
			};


			Dx *= 16;
			Dy *= 16;

			//for (int y = min.y; y < max.y; y += 2, Cy += Dx * 2) {
			//	glm::ivec3 E00 = Cy - Cx;
			//	glm::ivec3 E01 = Cy + Dx - Cx;
			//	glm::ivec3 E10 = Cy - Cx - Dy;
			//	glm::ivec3 E11 = Cy + Dx - Cx - Dy;
			//	for (int x = min.x; x < max.x; x += 2, E00 -= Dy * 2, E01 -= Dy * 2, E10 -= Dy * 2, E11 -= Dy * 2) {
			//		if (E00.x >= 0 && E00.y >= 0 && E00.z >= 0 && x < max.x && y < max.y)
			//			output(x, y, triangle, partial_coefs<glm::vec3>(E00, _mm_extract_ps(area, 0)), args...);

			//		if (E01.x >= 0 && E01.y >= 0 && E01.z >= 0 && x < max.x && y + 1 < max.y)
			//			output(x, y + 1, triangle, partial_coefs<glm::vec3>(E01, _mm_extract_ps(area, 0)), args...);

			//		if (E10.x >= 0 && E10.y >= 0 && E10.z >= 0 && x + 1 < max.x && y < max.y)
			//			output(x + 1, y, triangle, partial_coefs<glm::vec3>(E10, _mm_extract_ps(area, 0)), args...);

			//		if (E11.x >= 0 && E11.y >= 0 && E11.z >= 0 && x + 1 < max.x && y + 1 < max.y)
			//			output(x + 1, y + 1, triangle, partial_coefs<glm::vec3>(E11, _mm_extract_ps(area, 0)), args...);
			//	}
			//}
			//return;
			vDx.x = _mm_slli_epi32(vDx.x, 5);
			vDx.y = _mm_slli_epi32(vDx.y, 5);
			vDx.z = _mm_slli_epi32(vDx.z, 5);
			vDy.x = _mm_slli_epi32(vDy.x, 5);
			vDy.y = _mm_slli_epi32(vDy.y, 5);
			vDy.z = _mm_slli_epi32(vDy.z, 5);

			__m128i zero = _mm_setzero_si128();
			alignas(16) int mask_mem[4];
			for (__m128i y = miny; _mm_extract_epi32(y, 3) < max.y; y = _mm_add_epi32(y, increment), vCy += vDx, Cy += Dx) {
				glm::ivec3 E = Cy - Cx;
				math::sse::ivec3 vE = vCy - vCx;
				for (__m128i x = minx; _mm_extract_epi32(x, 3) < max.x; x = _mm_add_epi32(x, increment), vE -= vDy, E -= Dy) {
					__m128i mask = _mm_and_si128(
						_mm_and_si128(
							_mm_or_si128(_mm_cmpgt_epi32(vE.x, zero), _mm_cmpeq_epi32(vE.x, zero)),
							_mm_or_si128(_mm_cmpgt_epi32(vE.y, zero), _mm_cmpeq_epi32(vE.y, zero))
						),
						_mm_and_si128(
							_mm_or_si128(_mm_cmpgt_epi32(vE.z, zero), _mm_cmpeq_epi32(vE.z, zero)),
							_mm_and_si128(_mm_cmplt_epi32(y, maxy), _mm_cmplt_epi32(x, maxx))
						)
					);
					//mask = _mm_and_si128(_mm_cmplt_epi32(y, maxy), _mm_cmplt_epi32(x, maxx));
					_mm_store_si128(reinterpret_cast<__m128i*>(mask_mem), mask);
					auto bmask = math::make_x4<bool>(
						mask_mem[0] != 0,
						mask_mem[1] != 0,
						mask_mem[2] != 0,
						mask_mem[3] != 0
					);
					//bool yeet = (E.x >= 0) && (E.y >= 0) && (E.z >= 0);
					//auto bmask2 = math::make_x4<bool>(
					//	yeet && bmask[0],
					//	yeet && bmask[1],
					//	yeet && bmask[2],
					//	yeet && bmask[3]
					//);


					if (math::or_accross(bmask)) {
						output(x, y, triangle, partial_coefs(vE.x, vE.y, vE.z, area), bmask, args...);
					}
				}
			}
		}

		template <typename Callable, typename VertexT, typename ...Args>
		inline static void rasterize(
			Callable&& output,
			const VertexT* vertex_begin,
			const VertexT* vertex_end,
			const viewport& viewport,
			const tile& tile,
			Args&&... args
		) {
			math::sse::ivec2 offset = {
				_mm_set1_epi32(viewport.offset.x),
				_mm_set1_epi32(viewport.offset.y)
			};
			math::sse::ivec2 extent = {
				_mm_set1_epi32(viewport.extent.x),
				_mm_set1_epi32(viewport.extent.y)
			};
			math::sse::ivec2 tile_min = {
				_mm_set1_epi32(tile.min.x),
				_mm_set1_epi32(tile.min.y)
			};
			math::sse::ivec2 tile_max = {
				_mm_set1_epi32(tile.max.x),
				_mm_set1_epi32(tile.max.y)
			};
			const VertexT* vertices = vertex_begin;
			for (; vertices + 12 <= vertex_end; vertices += 12) {

				glm::ivec2 v00 = to_screen_space(vertices[0].rastPos, viewport);
				glm::ivec2 v01 = to_screen_space(vertices[1].rastPos, viewport);
				glm::ivec2 v02 = to_screen_space(vertices[2].rastPos, viewport);

				glm::ivec2 v10 = to_screen_space(vertices[3].rastPos, viewport);
				glm::ivec2 v11 = to_screen_space(vertices[4].rastPos, viewport);
				glm::ivec2 v12 = to_screen_space(vertices[5].rastPos, viewport);

				glm::ivec2 v20 = to_screen_space(vertices[6].rastPos, viewport);
				glm::ivec2 v21 = to_screen_space(vertices[7].rastPos, viewport);
				glm::ivec2 v22 = to_screen_space(vertices[8].rastPos, viewport);

				glm::ivec2 v30 = to_screen_space(vertices[9].rastPos, viewport);
				glm::ivec2 v31 = to_screen_space(vertices[10].rastPos, viewport);
				glm::ivec2 v32 = to_screen_space(vertices[11].rastPos, viewport);

				math::sse::ivec2 v0 = to_screen_space(
					_mm_set_ps(vertices[9].rastPos.x, vertices[6].rastPos.x, vertices[3].rastPos.x, vertices[0].rastPos.x),
					_mm_set_ps(vertices[9].rastPos.y, vertices[6].rastPos.y, vertices[3].rastPos.y, vertices[0].rastPos.y),
					extent.x, extent.y, offset.x, offset.y
				);
				math::sse::ivec2 v1 = to_screen_space(
					_mm_set_ps(vertices[10].rastPos.x, vertices[7].rastPos.x, vertices[4].rastPos.x, vertices[1].rastPos.x),
					_mm_set_ps(vertices[10].rastPos.y, vertices[7].rastPos.y, vertices[4].rastPos.y, vertices[1].rastPos.y),
					extent.x, extent.y, offset.x, offset.y
				);
				math::sse::ivec2 v2 = to_screen_space(
					_mm_set_ps(vertices[11].rastPos.x, vertices[8].rastPos.x, vertices[5].rastPos.x, vertices[2].rastPos.x),
					_mm_set_ps(vertices[11].rastPos.y, vertices[8].rastPos.y, vertices[5].rastPos.y, vertices[2].rastPos.y),
					extent.x, extent.y, offset.x, offset.y
				);
				//math::sse::ivec2 v0 = {
				//	_mm_set_epi32(v00.x, v10.x, v20.x, v30.x),
				//	_mm_set_epi32(v00.y, v10.y, v20.y, v30.y)
				//};
				//math::sse::ivec2 v1 = {
				//	_mm_set_epi32(v01.x, v11.x, v21.x, v31.x),
				//	_mm_set_epi32(v01.y, v11.y, v21.y, v31.y)
				//};
				//math::sse::ivec2 v2 = {
				//	_mm_set_epi32(v02.x, v12.x, v22.x, v32.x),
				//	_mm_set_epi32(v02.y, v12.y, v22.y, v32.y)
				//};
				alignas(16) int x_mem[3][4];
				alignas(16) int y_mem[3][4];
				_mm_store_si128(reinterpret_cast<__m128i*>(x_mem[0]), v0.x);
				_mm_store_si128(reinterpret_cast<__m128i*>(x_mem[1]), v1.x);
				_mm_store_si128(reinterpret_cast<__m128i*>(x_mem[2]), v2.x);
				_mm_store_si128(reinterpret_cast<__m128i*>(y_mem[0]), v0.y);
				_mm_store_si128(reinterpret_cast<__m128i*>(y_mem[1]), v1.y);
				_mm_store_si128(reinterpret_cast<__m128i*>(y_mem[2]), v2.y);

				glm::ivec2 min0 = glm::ivec2(
					std::max((int)std::min({ v00.x, v01.x, v02.x }), std::max(tile.min.x, viewport.offset.x)),
					std::max((int)std::min({ v00.y, v01.y, v02.y }), std::max(tile.min.y, viewport.offset.y))
				) / 16;
				glm::ivec2 max0 = glm::ivec2(
					std::min<int>({ std::max({ v00.x, v01.x, v02.x }) + 16, tile.max.x, viewport.offset.x + viewport.extent.x }),
					std::min<int>({ std::max({ v00.y, v01.y, v02.y }) + 16, tile.max.y, viewport.offset.y + viewport.extent.y })
				) / 16;
				glm::ivec2 min1 = glm::ivec2(
					std::max((int)std::min({ v10.x, v11.x, v12.x }), std::max(tile.min.x, viewport.offset.x)),
					std::max((int)std::min({ v10.y, v11.y, v12.y }), std::max(tile.min.y, viewport.offset.y))
				) / 16;
				glm::ivec2 max1 = glm::ivec2(
					std::min<int>({ std::max({ v10.x, v11.x, v12.x }) + 16, tile.max.x, viewport.offset.x + viewport.extent.x }),
					std::min<int>({ std::max({ v10.y, v11.y, v12.y }) + 16, tile.max.y, viewport.offset.y + viewport.extent.y })
				) / 16;
				glm::ivec2 min2 = glm::ivec2(
					std::max((int)std::min({ v20.x, v21.x, v22.x }), std::max(tile.min.x, viewport.offset.x)),
					std::max((int)std::min({ v20.y, v21.y, v22.y }), std::max(tile.min.y, viewport.offset.y))
				) / 16;
				glm::ivec2 max2 = glm::ivec2(
					std::min<int>({ std::max({ v20.x, v21.x, v22.x }) + 16, tile.max.x, viewport.offset.x + viewport.extent.x }),
					std::min<int>({ std::max({ v20.y, v21.y, v22.y }) + 16, tile.max.y, viewport.offset.y + viewport.extent.y })
				) / 16;
				glm::ivec2 min3 = glm::ivec2(
					std::max((int)std::min({ v30.x, v31.x, v32.x }), std::max(tile.min.x, viewport.offset.x)),
					std::max((int)std::min({ v30.y, v31.y, v32.y }), std::max(tile.min.y, viewport.offset.y))
				) / 16;
				glm::ivec2 max3 = glm::ivec2(
					std::min<int>({ std::max({ v30.x, v31.x, v32.x }) + 16, tile.max.x, viewport.offset.x + viewport.extent.x }),
					std::min<int>({ std::max({ v30.y, v31.y, v32.y }) + 16, tile.max.y, viewport.offset.y + viewport.extent.y })
				) / 16;

				math::sse::ivec2 min = {
					_mm_srai_epi32(_mm_max_epi32(_mm_min_epi32(v0.x, _mm_min_epi32(v1.x, v2.x)), _mm_max_epi32(tile_min.x, offset.x)), 4),
					_mm_srai_epi32(_mm_max_epi32(_mm_min_epi32(v0.y, _mm_min_epi32(v1.y, v2.y)), _mm_max_epi32(tile_min.y, offset.y)), 4)
					//_mm_set_epi32(min0.x, min1.x, min2.x, min3.x),
					//_mm_set_epi32(min0.y, min1.y, min2.y, min3.y)
				};
				math::sse::ivec2 max = {
					_mm_srai_epi32(_mm_min_epi32(
						_mm_add_epi32(_mm_max_epi32(v0.x, _mm_max_epi32(v1.x, v2.x)), _mm_set1_epi32(16)),
						_mm_min_epi32(tile_max.x, _mm_add_epi32(offset.x, extent.x))
					), 4),
					_mm_srai_epi32(_mm_min_epi32(
						_mm_add_epi32(_mm_max_epi32(v0.y, _mm_max_epi32(v1.y, v2.y)), _mm_set1_epi32(16)),
						_mm_min_epi32(tile_max.y, _mm_add_epi32(offset.y, extent.y))
					), 4)
					//_mm_set_epi32(max0.x, max1.x, max2.x, max3.x),
					//_mm_set_epi32(max0.y, max1.y, max2.y, max3.y)
				};
				alignas(16) int min_mem[2][4];
				alignas(16) int max_mem[2][4];
				_mm_store_si128(reinterpret_cast<__m128i*>(min_mem[0]), min.x);
				_mm_store_si128(reinterpret_cast<__m128i*>(min_mem[1]), min.y);
				_mm_store_si128(reinterpret_cast<__m128i*>(max_mem[0]), max.x);
				_mm_store_si128(reinterpret_cast<__m128i*>(max_mem[1]), max.y);

				glm::ivec3 Dx0 = glm::ivec3(
					v01.x - v00.x,
					v02.x - v01.x,
					v00.x - v02.x
				);
				glm::ivec3 Dy0 = glm::ivec3(
					v01.y - v00.y,
					v02.y - v01.y,
					v00.y - v02.y
				);
				glm::ivec3 Dx1 = glm::ivec3(
					v11.x - v10.x,
					v12.x - v11.x,
					v10.x - v12.x
				);
				glm::ivec3 Dy1 = glm::ivec3(
					v11.y - v10.y,
					v12.y - v11.y,
					v10.y - v12.y
				);
				glm::ivec3 Dx2 = glm::ivec3(
					v21.x - v20.x,
					v22.x - v21.x,
					v20.x - v22.x
				);
				glm::ivec3 Dy2 = glm::ivec3(
					v21.y - v20.y,
					v22.y - v21.y,
					v20.y - v22.y
				);
				glm::ivec3 Dx3 = glm::ivec3(
					v31.x - v30.x,
					v32.x - v31.x,
					v30.x - v32.x
				);
				glm::ivec3 Dy3 = glm::ivec3(
					v31.y - v30.y,
					v32.y - v31.y,
					v30.y - v32.y
				);
				math::sse::ivec3 Dx = {
					_mm_sub_epi32(v1.x, v0.x),
					_mm_sub_epi32(v2.x, v1.x),
					_mm_sub_epi32(v0.x, v2.x)
				};
				math::sse::ivec3 Dy = {
					_mm_sub_epi32(v1.y, v0.y),
					_mm_sub_epi32(v2.y, v1.y),
					_mm_sub_epi32(v0.y, v2.y)
				};



				alignas(16) int Dx_mem[3][4];
				alignas(16) int Dy_mem[3][4];
				_mm_store_si128(reinterpret_cast<__m128i*>(Dx_mem[0]), Dx.x);
				_mm_store_si128(reinterpret_cast<__m128i*>(Dx_mem[1]), Dx.y);
				_mm_store_si128(reinterpret_cast<__m128i*>(Dx_mem[2]), Dx.z);
				_mm_store_si128(reinterpret_cast<__m128i*>(Dy_mem[0]), Dy.x);
				_mm_store_si128(reinterpret_cast<__m128i*>(Dy_mem[1]), Dy.y);
				_mm_store_si128(reinterpret_cast<__m128i*>(Dy_mem[2]), Dy.z);

				__m128 area = _mm_cvtepi32_ps(_mm_sub_epi32(_mm_mullo_epi32(Dy.x, Dx.z), _mm_mullo_epi32(Dx.x, Dy.z)));
				alignas(16) float area_mem[4];
				_mm_store_ps(area_mem, area);

				//if (min0.x < max0.x && min0.y < max0.y && area_mem[0] > 0.0f)
				//	rasterize_one(output, vertices + 0, _mm_set_ps1(area_mem[0]),
				//		glm::ivec3(v00.x, v01.x, v02.x),
				//		glm::ivec3(v00.y, v01.y, v02.y),
				//		Dx0, Dy0,
				//		min0, max0, args...
				//	);

				for (size_t i = 0; i < 4; ++i) {
					if (min_mem[0][i] < max_mem[0][i] && min_mem[1][i] < max_mem[1][i] && area_mem[i] > 0.0f)
					rasterize_one(output, vertices + (i * 3), _mm_set_ps1(area_mem[i]),
						glm::ivec3(x_mem[0][i], x_mem[1][i], x_mem[2][i]),
						glm::ivec3(y_mem[0][i], y_mem[1][i], y_mem[2][i]),
						glm::ivec3(Dx_mem[0][i], Dx_mem[1][i], Dx_mem[2][i]),
						glm::ivec3(Dy_mem[0][i], Dy_mem[1][i], Dy_mem[2][i]),
						glm::ivec2(min_mem[0][i], min_mem[1][i]),
						glm::ivec2(max_mem[0][i], max_mem[1][i]),
						args...
					);
				}
			}
			if (vertices != vertex_end && vertices - 12 >= vertex_begin) vertices -= 12;
			for (; vertices < vertex_end; vertices += 3) {
				glm::ivec2 a = to_screen_space(vertices[0].rastPos, viewport);
				glm::ivec2 b = to_screen_space(vertices[1].rastPos, viewport);
				glm::ivec2 c = to_screen_space(vertices[2].rastPos, viewport);

				glm::ivec2 min = glm::ivec2(
					std::max((int)std::min({ a.x, b.x, c.x }), std::max(tile.min.x, viewport.offset.x)),
					std::max((int)std::min({ a.y, b.y, c.y }), std::max(tile.min.y, viewport.offset.y))
				) / 16;
				glm::ivec2 max = glm::ivec2(
					std::min<int>({ std::max({ a.x, b.x, c.x }) + 16, tile.max.x, viewport.offset.x + viewport.extent.x }),
					std::min<int>({ std::max({ a.y, b.y, c.y }) + 16, tile.max.y, viewport.offset.y + viewport.extent.y })
				) / 16;

				if (min.x >= max.x || min.y >= max.y) continue;

				glm::ivec3 x012 = glm::ivec3(a.x, b.x, c.x);
				glm::ivec3 x120 = glm::ivec3(b.x, c.x, a.x);

				glm::ivec3 y012 = glm::ivec3(a.y, b.y, c.y);
				glm::ivec3 y120 = glm::ivec3(b.y, c.y, a.y);

				glm::ivec3 Dx = x120 - x012;
				glm::ivec3 Dy = y120 - y012;

				int area = (Dy.x * Dx.z) - (Dx.x * Dy.z);
				if (area <= 0) continue; // back face detected - early return
				rasterize_one(output, vertices, _mm_set_ps1(static_cast<float>(area)),
					x012, y012, Dx, Dy, min, max, args...);
			}
		}

		template <typename Callable, typename Vertex, typename ...Args>
		inline static void rasterize_old(
			Callable&& output,
			const Vertex* vertex_begin,
			const Vertex* vertex_end,
			const viewport& viewport, const tile& tile,
			Args&&... args
		) {
			for (const Vertex* triangle = vertex_begin; triangle != vertex_end; triangle += 3) {
				rasterize_one(output, triangle, viewport, tile, args...);
			}
		}
	};
}
