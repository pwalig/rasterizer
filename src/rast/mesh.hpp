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

		inline static indexed load(std::ifstream& file) {
			const uint32_t max_alloc = 10485760; // 10 MB
			indexed res;

			// index buffer
			uint32_t count;
			file.read((char*)&count, sizeof(uint32_t));
			if (count > max_alloc) throw std::runtime_error("allocation limit exceeded");
			res.index_buffer.resize(count);
			file.read((char*)(res.index_buffer.data()), count * sizeof(uint32_t));

			// vertex buffer
			file.read((char*)&count, sizeof(uint32_t));
			if (count > max_alloc) throw std::runtime_error("allocation limit exceeded");
			res.vertex_buffer.resize(count);
			file.read((char*)(res.vertex_buffer.data()), count * sizeof(vertex));

			return res;
		}

		// .format
		// begin file {mesh}.mesh
		//
		// fieldb i.x i.z i.y
		// entryb v n uv.0.x uv.0.y
		// end
		inline static indexed load(const char* filename) {
			std::ifstream file(filename, std::ios::binary);
			if (!file.is_open()) throw std::runtime_error("failed to open file!");

			return load(file);
		}

		// .format
		// begin mesh
		// 
		// fieldb i.x i.z i.y
		// entryb v n uv.0.x uv.0.y
		// end
		//
		// begin file {file}.meshes 
		// 
		// entryb ; mesh
		// end
		static std::vector<indexed> load_multiple(const char* filename) {
			const uint32_t max_meshes = 1000;
			std::vector<indexed> res;

			std::ifstream file(filename, std::ios::binary);
			if (!file.is_open()) throw std::runtime_error("failed to open file!");

			// count of meshes
			uint32_t count;
			file.read((char*)&count, sizeof(uint32_t));
			if (count > max_meshes) throw std::runtime_error("mesh limit exceeded");
			res.reserve(count);
			for (uint32_t i = 0; i < count; ++i) {
				res.push_back(load(file));
			}
			return res;
		}

		template <void (*Func)(vertex&)>
		inline void process() {
			for (vertex& v : vertex_buffer) Func(v); 
		}

		template <typename Func>
		inline void process(Func f) {
			for (vertex& v : vertex_buffer) f(v); 
		}
	};

}
