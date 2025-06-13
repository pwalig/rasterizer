#pragma once
#include <cstdint>
#include <cassert>

#include <glm/glm.hpp>

#include "clip_vert.hpp"

namespace rast {
	struct near_clip_far_discard {
		inline static constexpr uint32_t maxClipTriangles = 2;
		inline static constexpr uint32_t maxClipVerts = maxClipTriangles * 3;

		// Shader::vertex::output* end = clip<typename Shader::vertex::output>(verts);
		template <typename vertex>
		inline static vertex* clip(
			typename vertex* verts
		) {
			if (
				verts[0].rastPos.z > verts[0].rastPos.w ||
				verts[1].rastPos.z > verts[1].rastPos.w ||
				verts[2].rastPos.z > verts[2].rastPos.w
			) return verts;

			glm::vec4 equation = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
			float values[3] = {
				glm::dot(verts[0].rastPos, equation),
				glm::dot(verts[1].rastPos, equation),
				glm::dot(verts[2].rastPos, equation)
			};

			uint8_t mask =
				(values[0] < 0.0f ? 1 : 0) |
				(values[1] < 0.0f ? 2 : 0) |
				(values[2] < 0.0f ? 4 : 0);

			auto end = verts + 3;
			switch (mask) {
			case 0b110:
				verts[1] = clip_vert(verts[1], verts[0], values[1], values[0]);
				verts[2] = clip_vert(verts[2], verts[0], values[2], values[0]);
				break;
			case 0b101:
				verts[0] = clip_vert(verts[0], verts[1], values[0], values[1]);
				verts[2] = clip_vert(verts[2], verts[1], values[2], values[1]);
				break;
			case 0b011:
				verts[0] = clip_vert(verts[0], verts[2], values[0], values[2]);
				verts[1] = clip_vert(verts[1], verts[2], values[1], values[2]);
				break;
			case 0b001:
				{
					auto v01 = clip_vert(verts[0], verts[1], values[0], values[1]);
					auto v02 = clip_vert(verts[0], verts[2], values[0], values[2]);
					verts[0] = v01;
					verts[3] = v01;
					verts[4] = verts[2];
					verts[5] = v02;
					end += 3;
				}
				break;
			case 0b010:
				{
					auto v10 = clip_vert(verts[1], verts[0], values[1], values[0]);
					auto v12 = clip_vert(verts[1], verts[2], values[1], values[2]);
					verts[1] = v10;
					verts[3] = v10;
					verts[4] = v12;
					verts[5] = verts[2];
					end += 3;
				}
				break;
			case 0b100:
				{
					auto v20 = clip_vert(verts[2], verts[0], values[2], values[0]);
					auto v21 = clip_vert(verts[2], verts[1], values[2], values[1]);
					verts[2] = v21;
					verts[3] = v21;
					verts[4] = v20;
					verts[5] = verts[0];
					end += 3;
				}
				break;
			case 0b111:
				return verts;
			}

			return end;
		}
	};
}
