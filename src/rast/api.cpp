#include "api.hpp"

#include <algorithm>
#include <DirectXMath.h>

namespace rast::api {
	struct bounds {
		image::size_type minx;
		image::size_type miny;
		image::size_type maxx;
		image::size_type maxy;

		inline bounds(
			image::size_type MinX, image::size_type MinY, image::size_type MaxX, image::size_type MaxY
		) : minx(MinX), miny(MinY), maxx(MaxX), maxy(MaxY) { }
	};

	inline bounds get_triangle_bounds(
		f x1, f y1, f x2, f y2, f x3, f y3,
		image::size_type width, image::size_type height
	) {
		return bounds(
			std::max<image::size_type>((image::size_type)std::min({ x1, x2, x3 }), 0),
			std::max<image::size_type>((image::size_type)std::min({ y1, y2, y3 }), 0),
			std::min<image::size_type>((image::size_type)std::max({ x1, x2, x3 }), width),
			std::min<image::size_type>((image::size_type)std::max({ y1, y2, y3 }), height)
		);
	}

	template <typename color>
	void draw_triangles_directX(image::view<color>& image_view, const DirectX::XMFLOAT3* vertex_data, data_len_t size, const color& col) {
		using img_siz = typename image::view<color>::size_type;


		for (data_len_t i = 0; i + 2 < size; i += 3) {
			bounds br = get_triangle_bounds(
				vertex_data[i].x, vertex_data[i].y,
				vertex_data[i+1].x, vertex_data[i+1].y,
				vertex_data[i+2].x, vertex_data[i+2].y,
				image_view.width, image_view.height
			);

			DirectX::XMVECTOR x123 = DirectX::XMVectorSet(vertex_data[i].x, vertex_data[i+1].x, vertex_data[i+2].x, 0.0f);
			DirectX::XMVECTOR x231 = DirectX::XMVectorSet(vertex_data[i+1].x, vertex_data[i+2].x, vertex_data[i].x, 0.0f);
			DirectX::XMVECTOR y123 = DirectX::XMVectorSet(vertex_data[i].y, vertex_data[i+1].y, vertex_data[i+2].y, 0.0f);
			DirectX::XMVECTOR y231 = DirectX::XMVectorSet(vertex_data[i+1].y, vertex_data[i+2].y, vertex_data[i].y, 0.0f);

			DirectX::XMVECTOR Dx = DirectX::XMVectorSubtract(x123, x231);
			DirectX::XMVECTOR Dy = DirectX::XMVectorSubtract(y123, y231);

			for (img_siz y = br.miny; y < br.maxy; ++y) {
				for (img_siz x = br.minx; x < br.maxx; ++x) {
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
	template<typename color>
	void draw_triangles_glm(
		image::view<color>& image_view,
		const glm::vec3* vertex_data,
		data_len_t size, const color& col
	) {
		using img_siz = image::size_type;

		for (data_len_t i = 0; i + 2 < size; i += 3) {
			bounds br = get_triangle_bounds(
				vertex_data[i].x, vertex_data[i].y,
				vertex_data[i+1].x, vertex_data[i+1].y,
				vertex_data[i+2].x, vertex_data[i+2].y,
				image_view.width, image_view.height
			);

			glm::ivec3 x123 = glm::ivec3(vertex_data[i].x * 16.0f, vertex_data[i+1].x * 16.0f, vertex_data[i+2].x * 16.0f);
			glm::ivec3 x231 = glm::ivec3(x123.y, x123.z, x123.x);
			glm::ivec3 y123 = glm::ivec3(vertex_data[i].y * 16.0f, vertex_data[i+1].y * 16.0f, vertex_data[i+2].y * 16.0f);
			glm::ivec3 y231 = glm::ivec3(y123.y, y123.z, y123.x);

			glm::ivec3 Dx = x123 - x231;
			glm::ivec3 Dy = y123 - y231;

			glm::ivec3 C = (Dy * x123) - (Dx * y123) + glm::ivec3(
				(Dy.x < 0 || (Dy.x == 0 && Dx.x > 0)) ? 1 : 0,
				(Dy.y < 0 || (Dy.y == 0 && Dx.y > 0)) ? 1 : 0,
				(Dy.z < 0 || (Dy.z == 0 && Dx.z > 0)) ? 1 : 0
			);

			glm::ivec3 Cy = C + (Dx * (br.miny << 4)) - (Dy * (br.minx << 4));

			for (img_siz y = br.miny; y < br.maxy; ++y) {
				glm::ivec3 Cx = Cy;

				for (img_siz x = br.minx; x < br.maxx; ++x) {
					glm::ivec3 Y = glm::ivec3(y, y, y) * 16 - y123;
					glm::ivec3 X = glm::ivec3(x, x, x) * 16 - x123;

					glm::ivec3 res = (Dy * X) - (Dx * Y);

					if (Cx.x <= 0 && Cx.y <= 0 && Cx.z <= 0) {
						image_view.at(x, y) += col;
					}

					Cx -= Dy * 16;
				}
				Cy += Dx * 16;
			}
		}
	}
}

template void rast::api::draw_triangles_directX<rast::color::rgba8>(
	image::view<rast::color::rgba8>& image_view,
	const DirectX::XMFLOAT3* vertex_data,
	data_len_t size, const rast::color::rgba8& col
);

template void rast::api::draw_triangles_glm<rast::color::rgba8>(
	image::view<rast::color::rgba8>& image_view,
	const glm::vec3* vertex_data,
	data_len_t size, const rast::color::rgba8& col
);
