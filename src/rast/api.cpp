#include "api.hpp"

#include <algorithm>
#include <DirectXMath.h>

namespace rast::api {
	template <typename color>
	void draw_triangles(image::view<color>& image_view, const DirectX::XMFLOAT3* vertex_data, data_len_t size, const color& col) {
		using img_siz = typename image::view<color>::size_type;


		for (data_len_t i = 0; i + 2 < size; i += 3) {
			img_siz miny = std::min<img_siz>(
				std::min<img_siz>((img_siz)vertex_data[i].y, (img_siz)vertex_data[i + 1].y),
				std::min<img_siz>((img_siz)vertex_data[i + 2].y, 0)
			);
			img_siz minx = std::min<img_siz>(
				std::min<img_siz>((img_siz)vertex_data[i].x, (img_siz)vertex_data[i + 1].x),
				std::min<img_siz>((img_siz)vertex_data[i + 2].x, 0)
			);
			img_siz maxy = std::max<img_siz>(
				std::max<img_siz>((img_siz)vertex_data[i].y, (img_siz)vertex_data[i + 1].y),
				std::max<img_siz>((img_siz)vertex_data[i + 2].y, image_view.height)
			);
			img_siz maxx = std::max<img_siz>(
				std::max<img_siz>((img_siz)vertex_data[i].x, (img_siz)vertex_data[i + 1].x),
				std::max<img_siz>((img_siz)vertex_data[i + 2].x, image_view.width)
			);

			DirectX::XMVECTOR x123 = DirectX::XMVectorSet(vertex_data[i].x, vertex_data[i+1].x, vertex_data[i+2].x, 0.0f);
			DirectX::XMVECTOR x231 = DirectX::XMVectorSet(vertex_data[i+1].x, vertex_data[i+2].x, vertex_data[i].x, 0.0f);
			DirectX::XMVECTOR y123 = DirectX::XMVectorSet(vertex_data[i].y, vertex_data[i+1].y, vertex_data[i+2].y, 0.0f);
			DirectX::XMVECTOR y231 = DirectX::XMVectorSet(vertex_data[i+1].y, vertex_data[i+2].y, vertex_data[i].y, 0.0f);

			DirectX::XMVECTOR Dx = DirectX::XMVectorSubtract(x123, x231);
			DirectX::XMVECTOR Dy = DirectX::XMVectorSubtract(y123, y231);

			for (img_siz y = miny; y < maxy; ++y) {
				for (img_siz x = minx; x < maxx; ++x) {
					DirectX::XMVECTOR Y = DirectX::XMVectorReplicate(y);
					DirectX::XMVECTOR X = DirectX::XMVectorReplicate(x);

					Y = DirectX::XMVectorSubtract(Y, y123);
					X = DirectX::XMVectorSubtract(X, x123);

					if (DirectX::XMVector3Greater(
						DirectX::XMVectorSubtract(
							DirectX::XMVectorMultiply(Dx, Y),
							DirectX::XMVectorMultiply(Dy, X)
						),
						DirectX::XMVectorReplicate(0.0f)
					)) {
						image_view.at(x, y) = col;
					}
				}
			}
		}
	}
}

template void rast::api::draw_triangles<rast::color::rgba8>(
	image::view<rast::color::rgba8>& image_view,
	const DirectX::XMFLOAT3* vertex_data,
	data_len_t size, const rast::color::rgba8& col
);
