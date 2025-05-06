#pragma once
#include <DirectXMath.h>

#include "image.hpp"

namespace rast::api {
	using data_len_t = u32;
	template <typename color>
	void draw_triangles(image::view<color>& image_view, const DirectX::XMFLOAT3* vertex_data, data_len_t size, const color& col);
}
