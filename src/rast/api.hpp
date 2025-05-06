#pragma once
#include <DirectXMath.h>
#include <glm/glm.hpp>

#include "image.hpp"

namespace rast::api {
	using data_len_t = u32;
	template <typename color>
	void draw_triangles_directX(
		image::view<color>& image_view,
		const DirectX::XMFLOAT3* vertex_data,
		data_len_t size, const color& col
	);

	template <typename color>
	void draw_triangles_glm(
		image::view<color>& image_view,
		const glm::vec3* vertex_data,
		data_len_t size, const color& col
	);
}
