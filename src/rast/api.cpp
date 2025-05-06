#include "api.hpp"

#include <DirectXMath.h>

namespace rast::api {
	template <typename color>
	void draw_triangles(image::view<color>& image_view, const f* vertex_data, data_len_t size) {
		for (data_len_t i = 0; i + 8 < size; i += 9) {
			XMVECTOR v0 = XMVectorSet( vertex_data[i], vertex_data[i+1], vertex_data[i+2], 0.f );

		}
	}
}
