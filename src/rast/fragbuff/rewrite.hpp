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
			int equation_results[3];
			int area;
		};

		std::vector<entry> storage;

		void push(
			uint32_t x, uint32_t y, vertex* triangle,
			int equation_results[3], int area
		) {
			storage.push_back({ x, y, triangle, equation_results, area });
		};
	};
}
