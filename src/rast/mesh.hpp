#pragma once
#include <vector>
#include <fstream>

#include <glm/glm.hpp>

#include "../sa_vector.hpp"
#include "types.hpp"

namespace rast::mesh {
	namespace cube {
		extern const f32 vertex_array [108];
		extern const u32 indices [36];
		extern const f32 vertices [72];
		extern const f32 normals [72];
		extern const f32 uv[48];
	};
	namespace screen_quad {
		extern const f32 vertex_array[30];
	}
	std::vector<glm::vec3> grid(u32 x, u32 y, f siz);

	template <typename VertexT>
	class indexed {
	public:
		using vertex = VertexT;

		sa_vector<u32> index_buffer;
		sa_vector<vertex> vertex_buffer;

		inline indexed() = default;
		// .format
		//
		// fieldb i.x i.z i.y
		// entryb v n uv.0.x uv.0.y
		inline indexed(const char* filename) {
			const u32 max_alloc = 10485760; // 10 MB

			std::ifstream file(filename, std::ios::binary);
			if (!file.is_open()) throw std::runtime_error("failed to open file!");

			// index buffer
			u32 count;
			file.read((char*)&count, sizeof(u32));
			if (count > max_alloc) throw std::runtime_error("allocation limit exceeded");
			index_buffer.resize(count);
			file.read((char*)(index_buffer.data()), count * sizeof(u32));

			// vertex buffer
			file.read((char*)&count, sizeof(u32));
			if (count > max_alloc) throw std::runtime_error("allocation limit exceeded");
			vertex_buffer.resize(count);
			file.read((char*)(vertex_buffer.data()), count * sizeof(vertex));
		}
	};

}
