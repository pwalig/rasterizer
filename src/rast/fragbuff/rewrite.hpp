#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace rast::fragbuff {
	template <typename Shader>
	struct rewrite {
		using vertex = typename Shader::vertex::output;

		struct entry {
			uint32_t x;
			uint32_t y;
			vertex* triangle;
			float partial_coefs[3];
		};

		std::vector<entry> storage;

		void push(
			uint32_t x, uint32_t y, vertex* triangle,
			float partial_coefs[3]
		) {
			storage.push_back({ x, y, triangle, partial_coefs });
		};
	};
}
