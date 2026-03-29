#pragma once
#include "../sized2d_base.hpp"
#include "../depth_test.hpp"
#include "utils.hpp"

namespace rast::framebuffer {
	template<
		typename ColorFormat, typename DepthFormat,
		depth_test::function::type<DepthFormat> DepthTest = depth_test::less
	> struct depth_view : public sized2d_base {
		using color_format = ColorFormat;
		using depth_format = DepthFormat;
		using size_type = typename sized2d_base::size_type;

	private:
		color_format* _color_data;
		depth_format* _depth_data;

	public:
		float near = 0.1f;
		float far = 100.0f;

		float near_clip = 0.0f;
		float far_clip = 1.0f;

		inline constexpr color_format& color(size_type x, size_type y) {
			return _color_data[data_offset(x, y)];
		}
		inline constexpr color_format& depth(size_type x, size_type y) {
			return _depth_data[data_offset(x, y)];
		}

		inline depth_view(
			color_format* ColorData, depth_format* DepthData,
			size_type Width, size_type Height,
			float Near, float Far, float NearClip = 0.0f, float FarClip = 1.0f
		) noexcept : sized2d_base(Width, Height),
			_color_data(ColorData), _depth_data(DepthData),
			near(Near), far(Far), near_clip(NearClip), far_clip(FarClip) {}
		
		template <typename ImageLike1, typename ImageLike2>
		inline depth_view(
			ImageLike1& ColorImage, ImageLike2& DepthImage,
			float Near, float Far, float NearClip = 0.0f, float FarClip = 1.0f
		) : depth_view(
			ColorImage.data(), DepthImage.data(),
			ColorImage.width(), ColorImage.height(),
			Near, Far, NearClip, FarClip
		) {}

		template <typename ImageLike>
		inline depth_view(
			color_format* ColorData, ImageLike& DepthImage,
			float Near, float Far, float NearClip = 0.0f, float FarClip = 1.0f
		) : depth_view(
			ColorData, DepthImage.data(),
			DepthImage.width(), DepthImage.height(),
			Near, Far, NearClip, FarClip
		) {}

		template <typename ImageLike>
		inline depth_view(
			ImageLike& ColorImage, depth_format* DepthData,
			float Near, float Far, float NearClip = 0.0f, float FarClip = 1.0f
		) : depth_view(
			ColorImage.data(), DepthData,
			ColorImage.width(), ColorImage.height(),
			Near, Far, NearClip, FarClip
		) {}

		template <typename VertexT, typename ...Args>
		inline constexpr void operator()(
			uint32_t x, uint32_t y,
			const VertexT* triangle,
			glm::vec3 partial_coefs,
			Args&&... args
		) {
			// depth test
			depth_format& oldDepth = this->depth(x, y);
			float depth = interpol::depth(triangle, partial_coefs);
			depth_format newDepth = float_depth_to_depth_format<depth_format>(depth);
			if (DepthTest(newDepth, oldDepth)) {
				oldDepth = newDepth;

				// output color
				depth = near / (far + depth * (near - far));
				this->color(x, y) = convert::f01_to_uint<float, uint8_t>(glm::vec4(glm::vec3((depth - near_clip) / far_clip), 1.0f));
			}
		}
	};
}
