#include "renderer.hpp"
#include <algorithm>
#include <iostream>

glm::ivec3 rast::renderer::toWindowSpace(const glm::vec3& vertex) {
	//return glm::ivec3(
	//	vertex.x * 16.0f,
	//	vertex.y * 16.0f,
	//	vertex.z * 16.0f
	//);
    glm::vec4 res = P * V * M * glm::vec4(vertex, 1.0f);
    //glm::vec4 res = V * M * glm::vec4(vertex, 1.0f);


	//std::cout << res.x << res.y << res.z << res.w;

    res /= res.w;

    return glm::ivec3(
        (( res.x + 1.0f ) * (float)viewportDims.x * 0.5f + (float)viewportOffset.x) * 16.0f,
        (( -res.y + 1.0f ) * (float)viewportDims.y * 0.5f + (float)viewportOffset.y) * 16.0f,
        ( res.z + 1.0f ) * 500.0f // (-1 : 1) ==> (0 : 1000)
    );
}

void rast::renderer::setViewport(int xoffset, int yoffset, int width, int height)
{
    viewportDims.x = width;
    viewportDims.y = height;
    viewportOffset.x = xoffset;
    viewportOffset.y = yoffset;
}

void rast::renderer::setM(const glm::mat4& M_) {M = M_;}
void rast::renderer::setV(const glm::mat4& V_) {V = V_;}
void rast::renderer::setP(const glm::mat4& P_) {P = P_;}

template<typename colorT>
void rast::renderer::draw_triangles_glm(
	image::view<colorT>& image_view,
	const glm::vec3* vertex_data,
	data_len_t size, const colorT& col
) {
	using img_siz = image::size_type;

	for (data_len_t i = 0; i + 2 < size; i += 3) {

		glm::ivec3 a = toWindowSpace(vertex_data[i]);
		glm::ivec3 b = toWindowSpace(vertex_data[i + 1]);
		glm::ivec3 c = toWindowSpace(vertex_data[i + 2]);

		glm::ivec2 min = glm::ivec2(
			std::max((int)std::min({ a.x, b.x, c.x }), 0),
			std::max((int)std::min({ a.y, b.y, c.y }), 0)
		);
		glm::ivec2 max = glm::ivec2(
			std::min<int>(std::max({ a.x, b.x, c.x }), image_view.width << 4),
			std::min<int>(std::max({ a.y, b.y, c.y }), image_view.height << 4)
		);

		glm::ivec3 x123 = glm::ivec3(a.x, b.x, c.x);
		glm::ivec3 x231 = glm::ivec3(x123.y, x123.z, x123.x);
		glm::ivec3 y123 = glm::ivec3(a.y, b.y, c.y);
		glm::ivec3 y231 = glm::ivec3(y123.y, y123.z, y123.x);

		glm::ivec3 Dx = x123 - x231;
		glm::ivec3 Dy = y123 - y231;

		glm::ivec3 C = (Dy * x123) - (Dx * y123) +glm::ivec3(
			(Dy.x < 0 || (Dy.x == 0 && Dx.x > 0)) ? 1 : 0,
			(Dy.y < 0 || (Dy.y == 0 && Dx.y > 0)) ? 1 : 0,
			(Dy.z < 0 || (Dy.z == 0 && Dx.z > 0)) ? 1 : 0
		);

		glm::ivec3 Cy = C + (Dx * min.y) - (Dy * min.x);

		max /= 16;
		min /= 16;
		for (img_siz y = min.y; y < max.y; ++y) {
			glm::ivec3 Cx = Cy;

			for (img_siz x = min.x; x < max.x; ++x) {

				if (Cx.x > 0 && Cx.y > 0 && Cx.z > 0) {
					image_view.at(x, y) += col;
				}

				Cx -= Dy * 16;
			}
			Cy += Dx * 16;
		}
	}
}

template void rast::renderer::draw_triangles_glm<rast::color::rgba8>(
	image::view<rast::color::rgba8>& image_view,
	const glm::vec3* vertex_data,
	data_len_t size, const rast::color::rgba8& col
);

