#pragma once
#include "image.hpp"

namespace rast::api {
	using data_len_t = u32;
	template <typename color>
	void draw_triangles(image::view<color>& image_view, const f* vertex_data, data_len_t size);
}
