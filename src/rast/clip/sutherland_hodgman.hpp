#pragma once
#include <cstdint>
#include <cassert>

#include <glm/glm.hpp>

#include "clip_vert.hpp"

namespace rast {
	struct sutherland_hodgman {
		inline static constexpr uint32_t maxSutherlandHodgmanVerts = 9;
		inline static constexpr uint32_t maxClipTriangles = maxSutherlandHodgmanVerts - 2;
		inline static constexpr uint32_t maxClipVerts = maxClipTriangles * 3;

		// Shader::vertex::output* end = clip<typename Shader::vertex::output>(verts);
		template <typename vertex>
		inline static vertex* clip(
			typename vertex* verts
		) {
			vertex list[maxSutherlandHodgmanVerts];
			std::memcpy(list, verts, 3 * sizeof(vertex));

			vertex* outputList = verts;
			vertex* inputList = list;
			uint32_t listIndex = 0; // count of elements in output list
			uint32_t count = 3; // count of elements in input list


			const glm::vec4 equations[6] = {
				glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), // near
				glm::vec4(0.0f, 0.0f, -1.0f, 1.0f), // far
				glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), // X
				glm::vec4(-1.0f, 0.0f, 0.0f, 1.0f), // -X
				glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), // Y
				glm::vec4(0.0f, -1.0f, 0.0f, 1.0f), // -Y
			};

			for (uint32_t eq = 0; eq < 6; ++eq) {
				for (uint32_t i = 0; i < count; ++i) {
					vertex current = inputList[i];
					vertex prev = inputList[(i + count - 1) % count];
					float current_value = glm::dot(current.rastPos, equations[eq]);
					float prev_value = glm::dot(prev.rastPos, equations[eq]);

					if (current_value >= 0.0f) { // if current inside
						if (prev_value < 0.0f) { // if prev outside
							assert(listIndex < maxClipVerts);
							outputList[listIndex++] = clip_vert<vertex>(prev, current, prev_value, current_value);
						}
						assert(listIndex < maxClipVerts);
						outputList[listIndex++] = current;
					}
					else if (prev_value >= 0.0f) { // prev inside
						assert(listIndex < maxClipVerts);
						outputList[listIndex++] = clip_vert<vertex>(current, prev, current_value, prev_value);
					}
				}
				std::swap(inputList, outputList); // "copy" output list to input list
				count = listIndex;
				listIndex = 0; // "clear" output list
			}
			assert(inputList == verts);
			assert(outputList == list);
			if (count < 3) return verts;
			else {
				// triangulate
				for (uint32_t i = 1; i < count - 1; ++i) {
					verts[listIndex++] = list[0];
					verts[listIndex++] = list[i];
					verts[listIndex++] = list[i+1];
				}
				assert(listIndex % 3 == 0);
				return verts + listIndex;
			}
		}

	};
}
