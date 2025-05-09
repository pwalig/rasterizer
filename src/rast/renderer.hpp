#pragma once
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "types.hpp"
#include "image.hpp"

namespace rast {
	class renderer {
	public:
		using data_len_t = u32;
	private:
		glm::ivec2 viewportDims = glm::ivec2(100, 100);
		glm::ivec2 viewportOffset = glm::ivec2(0, 0);

		glm::mat4 M = glm::mat4(1.0f);
		glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, 30.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 P = glm::perspective(glm::radians(70.0f), 16.0f / 9.0f, 0.1f, 100.0f);

		glm::ivec3 toWindowSpace(const glm::vec3& vertex);

	public:
		void setViewport(int xoffset, int yoffset, int width, int height);

		void setM(const glm::mat4& M);
		void setV(const glm::mat4& V);
		void setP(const glm::mat4& P);

		template <typename colorT>
		void draw_triangles_glm(
			image::view<colorT>& image_view,
			const glm::vec3* vertex_data,
			data_len_t size, const colorT& col
		);

	};
}
