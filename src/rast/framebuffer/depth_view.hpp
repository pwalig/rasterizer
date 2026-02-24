#pragma once
#include "color_depth.hpp"

namespace rast::framebuffer {
	template<typename ColorFormat, typename DepthFormat, depth_test::function::type<DepthFormat> DepthTest = depth_test::less>
	class depth_view : public color_depth<ColorFormat, DepthFormat, alpha_blend::replace<ColorFormat>, DepthTest> {
		using parent = color_depth<ColorFormat, DepthFormat, alpha_blend::replace<ColorFormat>, DepthTest>;
		using color_format = ColorFormat;
		using depth_format = DepthFormat;
	public:
		float near = 0.1f;
		float far = 100.0f;

		float near_clip = 0.0f;
		float far_clip = 1.0f;

		inline depth_view(
			color_format* ColorData, depth_format* DepthData, uint32_t Width, uint32_t Height, float Near, float Far, float NearClip = 0.0f, float FarClip = 1.0f
		) : parent(ColorData, DepthData, Width, Height), near(Near), far(Far), near_clip(NearClip), far_clip(FarClip) { }
		
		inline depth_view(
			image<color_format>& ColorImage, image<depth_format>& DepthImage, float Near, float Far, float NearClip = 0.0f, float FarClip = 1.0f
		) : parent(ColorImage, DepthImage), near(Near), far(Far), near_clip(NearClip), far_clip(FarClip) {}

		inline depth_view(
			color_format* ColorData, image<depth_format>& DepthImage, float Near, float Far, float NearClip = 0.0f, float FarClip = 1.0f
		) : parent(ColorData, DepthImage), near(Near), far(Far), near_clip(NearClip), far_clip(FarClip) {}

		inline depth_view(
			image<color_format>& ColorImage, depth_format* DepthData, float Near, float Far, float NearClip = 0.0f, float FarClip = 1.0f
		) : parent(ColorImage, DepthData), near(Near), far(Far), near_clip(NearClip), far_clip(FarClip) {}

		template <typename Shader>
		void draw(
			uint32_t x, uint32_t y,
			const typename Shader::vertex::output* triangle,
			glm::vec3 partial_coefs,
			const typename Shader::fragment::uniform_buffer&
		) {
			// depth test
			depth_format& oldDepth = this->depth(x, y);
			float depth = get_float_depth(triangle, partial_coefs);
			depth_format newDepth = float_depth_to_depth_format<depth_format>(depth);
			if (DepthTest(newDepth, oldDepth)) {
				oldDepth = newDepth;

				// output color
				depth = near / (far + depth * (near - far));
				this->color(x, y) = convert::f01_to_uint<float, uint8_t>(glm::vec4(glm::vec3((depth - near_clip) / far_clip), 1.0f));
			}
		}

		template <typename Shader>
		auto raster_adapter(const typename Shader::fragment::uniform_buffer& uniform_buffer) {
			return [this, &uniform_buffer](
				uint32_t x, uint32_t y, const typename Shader::vertex::output* triangle, glm::vec3 partial_coefs
			) {
				this->draw<Shader>(x, y, triangle, partial_coefs, uniform_buffer);
			};
		}
	};
}

namespace rast::raster {
	template<typename ColorFormat, typename DepthFormat, depth_test::function::type<DepthFormat> DepthTest>
	inline constexpr output_interface output_interface_v<framebuffer::depth_view<ColorFormat, DepthFormat, DepthTest>> = output_interface::framebuffer;
}
