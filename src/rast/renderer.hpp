#pragma once
#include <algorithm>
#include <iostream>

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

		inline glm::ivec3 toScreenSpace(const glm::vec4& vertex) {
			glm::vec4 res = vertex / vertex.w;

			return glm::ivec3(
				(( res.x + 1.0f ) * (float)viewportDims.x * 0.5f + (float)viewportOffset.x) * 16.0f,
				(( -res.y + 1.0f ) * (float)viewportDims.y * 0.5f + (float)viewportOffset.y) * 16.0f,
				( res.z * 0.5f + 0.5f) * std::numeric_limits<int>::max()
			);
		}
		
		template <typename T>
		inline T interpolate(const T& a, const T& b, const T& c, const glm::vec3& coefs) {
			return (T)(a * coefs.x) + (T)(b * coefs.y) + (T)(c * coefs.z);
		}

		template <typename T>
		inline T interpolate(const glm::vec<3, T>& values, const glm::vec3& coefs) {
			return (T)(values.x * coefs.x) + (T)(values.y * coefs.y) + (T)(values.z * coefs.z);
		}

		template <typename T>
		inline T interpolate2(T a, T b, T c, const glm::vec3& coefs) {
			return (T)(a * coefs.x) + (T)(b * coefs.y) + (T)(c * coefs.z);
		}

		template <typename Shader, typename Image, typename DepthImage>
		void rasterize(
			Image& image,
			DepthImage& depthImage,
			const typename Shader::vertex::output* vertex_begin,
			const typename Shader::vertex::output* vertex_end
		) {
			using vertex = typename Shader::vertex::output;

			for (const vertex* vert = vertex_begin; vert != vertex_end; vert += 3) {

				glm::ivec3 a = toScreenSpace(vert[0].rastPos);
				glm::ivec3 b = toScreenSpace(vert[1].rastPos);
				glm::ivec3 c = toScreenSpace(vert[2].rastPos);

				glm::ivec2 min = glm::ivec2(
					std::max((int)std::min({ a.x, b.x, c.x }), 0),
					std::max((int)std::min({ a.y, b.y, c.y }), 0)
				) / 16;
				glm::ivec2 max = glm::ivec2(
					std::min<int>(std::max({ a.x, b.x, c.x }) + 16, image.width << 4),
					std::min<int>(std::max({ a.y, b.y, c.y }) + 16, image.height << 4)
				) / 16;

				glm::ivec3 x012 = glm::ivec3(a.x, b.x, c.x);
				glm::ivec3 x120 = glm::ivec3(b.x, c.x, a.x);

				glm::ivec3 y012 = glm::ivec3(a.y, b.y, c.y);
				glm::ivec3 y120 = glm::ivec3(b.y, c.y, a.y);

				glm::ivec3 Dx = x120 - x012;
				glm::ivec3 Dy = y120 - y012;

				glm::ivec3 fill_convention = glm::ivec3(
					(Dy.x > 0 || (Dy.x == 0 && Dx.x < 0)) ? 1 : 0,
					(Dy.y > 0 || (Dy.y == 0 && Dx.y < 0)) ? 1 : 0,
					(Dy.z > 0 || (Dy.z == 0 && Dx.z < 0)) ? 1 : 0
				);

				int area = (Dy.x * Dx.z) - (Dx.x * Dy.z);

				for (int y = min.y; y < max.y; ++y) {
					// Dx * Y - fill_convention
					glm::ivec3 Cy = Dx * (glm::ivec3(y << 4) - y012) - fill_convention;

					for (int x = min.x; x < max.x; ++x) {
						glm::ivec3 X = glm::ivec3(x << 4) - x012;

						glm::ivec3 res = Cy - (Dy * X);
						if (res.x >= 0 && res.y >= 0 && res.z >= 0) {

							// interpolation coefitients
							glm::vec3 coefs(
								(float)res.y / area,
								(float)res.z / area,
								(float)res.x / area
							);

							// depth test
							int& oldDepth = depthImage.at(x, y);
							int newDepth = interpolate2(a.z, b.z, c.z, coefs);
							coefs /= glm::vec3(
								vert[0].rastPos.w,
								vert[1].rastPos.w,
								vert[2].rastPos.w
							);
							float sum = coefs.x + coefs.y + coefs.z;
							coefs /= sum;
							if (newDepth < oldDepth) {
								oldDepth = newDepth;

								// output color
								image.at(x, y) = Shader::fragment::shade(
									Shader::fragment::interpolate(
										vertex_begin[0].data,
										vertex_begin[1].data,
										vertex_begin[2].data,
										coefs
									)
								);
							}
						}
					}
				}
			}
		}

	public:
		void setViewport(int xoffset, int yoffset, int width, int height);

		template <typename Shader, typename Image, typename DepthImage>
		void draw_array(
			Image& image,
			DepthImage& depthImage,
			const typename Shader::vertex::input* vertex_begin,
			const typename Shader::vertex::input* vertex_end
		) {
			using input_vertex = typename Shader::vertex::input;
			using output_vertex = typename Shader::vertex::output;

			for (const input_vertex* vert = vertex_begin; vert < vertex_end; vert += 3) {
				output_vertex clipped_verts[3];

				clipped_verts[0] = Shader::vertex::shade(vert[0]);
				clipped_verts[1] = Shader::vertex::shade(vert[1]);
				clipped_verts[2] = Shader::vertex::shade(vert[2]);

				if (
					clipped_verts[0].rastPos.z > clipped_verts[0].rastPos.w ||
					clipped_verts[0].rastPos.z < -clipped_verts[0].rastPos.w ||
					clipped_verts[1].rastPos.z > clipped_verts[1].rastPos.w ||
					clipped_verts[1].rastPos.z < -clipped_verts[1].rastPos.w ||
					clipped_verts[2].rastPos.z > clipped_verts[2].rastPos.w ||
					clipped_verts[2].rastPos.z < -clipped_verts[2].rastPos.w
					) continue;

				rasterize<Shader, Image, DepthImage>(
					image,
					depthImage,
					clipped_verts,
					clipped_verts + 3
				);
			}
		}

		template <typename Shader, typename Image, typename DepthImage>
		void draw_indexed(
			Image& image,
			DepthImage& depthImage,
			const u32* index_begin,
			const u32* index_end,
			const typename Shader::vertex::input* vertex_buffer
		) {
			using input_vertex = typename Shader::vertex::input;
			using output_vertex = typename Shader::vertex::output;

			for (const u32* i = index_begin; i < index_end; i += 3) {
				output_vertex clipped_verts[3];

				clipped_verts[0] = Shader::vertex::shade(vertex_buffer[i[0]]);
				clipped_verts[1] = Shader::vertex::shade(vertex_buffer[i[1]]);
				clipped_verts[2] = Shader::vertex::shade(vertex_buffer[i[2]]);

				if (
					clipped_verts[0].rastPos.z > clipped_verts[0].rastPos.w ||
					clipped_verts[0].rastPos.z < -clipped_verts[0].rastPos.w ||
					clipped_verts[1].rastPos.z > clipped_verts[1].rastPos.w ||
					clipped_verts[1].rastPos.z < -clipped_verts[1].rastPos.w ||
					clipped_verts[2].rastPos.z > clipped_verts[2].rastPos.w ||
					clipped_verts[2].rastPos.z < -clipped_verts[2].rastPos.w
					) continue;

				rasterize<Shader, Image, DepthImage>(
					image,
					depthImage,
					clipped_verts,
					clipped_verts + 3
				);
			}
		}
	};
}
