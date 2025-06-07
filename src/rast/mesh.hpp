#pragma once
#include <vector>
#include <fstream>

#include <glm/glm.hpp>

#include "../sa_vector.hpp"

namespace rast::mesh {
	namespace cube {
		extern const float vertex_array [108];
		extern const uint32_t indices [36];
		extern const float vertices [72];
		extern const float normals [72];
		extern const float uv[48];
	};
	namespace screen_quad {
		extern const float vertex_array[30];
	}
	std::vector<glm::vec3> grid(uint32_t x, uint32_t y, float siz);

	template <typename VertexT>
	class indexed {
	public:
		using vertex = VertexT;

		sa_vector<uint32_t> index_buffer;
		sa_vector<vertex> vertex_buffer;

		inline indexed() = default;
		// .format
		//
		// fieldb i.x i.z i.y
		// entryb v n uv.0.x uv.0.y
		inline indexed(const char* filename) {
			const uint32_t max_alloc = 10485760; // 10 MB

			std::ifstream file(filename, std::ios::binary);
			if (!file.is_open()) throw std::runtime_error("failed to open file!");

			// index buffer
			uint32_t count;
			file.read((char*)&count, sizeof(uint32_t));
			if (count > max_alloc) throw std::runtime_error("allocation limit exceeded");
			index_buffer.resize(count);
			file.read((char*)(index_buffer.data()), count * sizeof(uint32_t));

			// vertex buffer
			file.read((char*)&count, sizeof(uint32_t));
			if (count > max_alloc) throw std::runtime_error("allocation limit exceeded");
			vertex_buffer.resize(count);
			file.read((char*)(vertex_buffer.data()), count * sizeof(vertex));
		}
	};

}
