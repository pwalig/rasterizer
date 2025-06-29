#pragma once
#include <glm/glm.hpp>

#include "../viewport.hpp"
#include "../tile.hpp"

namespace rast::raster {
	inline glm::ivec2 toScreenSpace(const glm::vec4& vertex, const viewport& viewport) {
		return glm::ivec2(
			(( vertex.x + 1.0f ) * (float)viewport.extent.x * 0.5f + (float)viewport.offset.x),
			(( -vertex.y + 1.0f ) * (float)viewport.extent.y * 0.5f + (float)viewport.offset.y)
		);
	}

	template <typename Shader, typename Framebuffer>
	using function = void(*)(
		Framebuffer& framebuffer,
		const typename Shader::vertex::output* vertex_begin,
		const typename Shader::vertex::output* vertex_end,
		const typename Shader::fragment::uniform_buffer& uniform_buffer,
		const viewport& viewport,
		const tile& tile
	);


	glm::ivec3 fill_convention(const glm::ivec3& Dx, const glm::ivec3& Dy) {
		return glm::ivec3(
			(Dy.x > 0 || (Dy.x == 0 && Dx.x < 0)) ? 1 : 0,
			(Dy.y > 0 || (Dy.y == 0 && Dx.y < 0)) ? 1 : 0,
			(Dy.z > 0 || (Dy.z == 0 && Dx.z < 0)) ? 1 : 0
		);
	}

	template <typename Rasterizer, typename Shader, typename Framebuffer>
	inline void execute(
		Framebuffer& framebuffer,
		const typename Shader::vertex::output* vertex_begin,
		const typename Shader::vertex::output* vertex_end,
		const typename Shader::fragment::uniform_buffer& uniform_buffer,
		const viewport& viewport,
		const tile& tile
	) {
		using vertex = typename Shader::vertex::output;
		for (const vertex* triangle = vertex_begin; triangle != vertex_end; triangle += 3) {
			Rasterizer::template rasterize_one<Shader>(framebuffer, triangle, uniform_buffer, viewport, tile);
		}
	}
}
