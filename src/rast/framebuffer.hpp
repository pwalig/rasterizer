#pragma once
#include <algorithm>

#include <glm/glm.hpp>

#include "image.hpp"
#include "interpolation.hpp"
#include "convert.hpp"
#include "alpha_blend.hpp"
#include "discard_fragment.hpp"
#include "depth_test.hpp"

#define rast_framebuffer_shader_output_assert \
using fragment_output = typename Shader::fragment::output; \
if constexpr (is_discardable_v<fragment_output>) { \
	static_assert(std::is_same_v<color_format, typename fragment_output::value_type>); \
} else { \
	static_assert(std::is_same_v<color_format, fragment_output>); \
}

namespace rast::framebuffer {
	template <typename DepthFormat>
	inline DepthFormat float_depth_to_depth_format(float depth) {
		if constexpr (std::is_floating_point_v<DepthFormat>) {
			return static_cast<DepthFormat>(depth);
		}
		else {
			return static_cast<DepthFormat>(
				(depth * 0.5f + 0.5f) * std::numeric_limits<DepthFormat>::max()
			);
		}
	}
	template <typename vertex_output>
	inline float get_float_depth(const vertex_output* triangle, const glm::ivec3& results, int area) {
		return interpol::interpolate(
			glm::vec3(
				triangle[0].rastPos.z,
				triangle[1].rastPos.z,
				triangle[2].rastPos.z
			),
			interpol::linear_coefs(results, area)
		);
	}
	template <typename depth_format, typename vertex_output>
	inline depth_format get_depth(const vertex_output* triangle, const glm::ivec3& results, int area) {
		return float_depth_to_depth_format<depth_format>(get_float_depth(triangle, results, area));
	}

	namespace raster_adapter {
		template <typename Framebuffer, typename Shader>
		struct with_ubo {
			using vertex = typename Shader::vertex::output;
			using uniform_buffer = typename Shader::fragment::uniform_buffer;

			Framebuffer& framebuffer;
			const uniform_buffer& ubo;

			inline with_ubo(Framebuffer& FrameBuffer, const uniform_buffer& UniformBuffer)
				: framebuffer(FrameBuffer), ubo(UniformBuffer) {}
			inline void operator()(uint32_t x, uint32_t y, const vertex* triangle, glm::ivec3 equation_results, int area) {
				framebuffer.template draw<Shader>(x, y, triangle, ubo, equation_results, area);
			}
		};
		template <typename Framebuffer, typename Shader>
		struct framebuffer_only {
			using vertex = typename Shader::vertex::output;
			using uniform_buffer = typename Shader::fragment::uniform_buffer;

			Framebuffer& framebuffer;

			inline framebuffer_only(Framebuffer& FrameBuffer)
				: framebuffer(FrameBuffer) {}
			inline void operator()(uint32_t x, uint32_t y, const vertex* triangle, glm::ivec3 equation_results, int area) {
				framebuffer.template draw<Shader>(x, y, triangle, uniform_buffer(), equation_results, area);
			}
		};
	}

	using default_alpha_blend = rast::alpha_blend::func<rast::alpha_blend::factor::src_alpha, rast::alpha_blend::factor::one_minus_src_alpha, rast::alpha_blend::equation::add>;

	template<typename ColorFormat, typename DepthFormat, alpha_blend::function::type<ColorFormat> AlphaBlend = default_alpha_blend::blend, depth_test::function::type<DepthFormat> DepthTest = depth_test::less>
	class color_depth {
	public:
		using depth_format = DepthFormat;
		using color_format = ColorFormat;

	private:
		color_format* colorImage;
		depth_format* depthImage;
		uint32_t _width;
		uint32_t _height;

	protected:
		inline color_format& color(uint32_t x, uint32_t y) {
			return colorImage[y * _width + x];
		}
		inline depth_format& depth(uint32_t x, uint32_t y) {
			return depthImage[y * _width + x];
		}

	public:
		inline color_depth(color_format* ColorData, depth_format* DepthData, uint32_t Width, uint32_t Height) :
			colorImage(ColorData), depthImage(DepthData), _width(Width), _height(Height) { }
		inline color_depth(image<color_format>& ColorImage, image<depth_format>& DepthImage) :
			colorImage(ColorImage.data()), depthImage(DepthImage.data()), _width(ColorImage.width()), _height(ColorImage.height()) {}
		inline color_depth(color_format* ColorData, image<depth_format>& DepthImage) :
			colorImage(ColorData), depthImage(DepthImage.data()), _width(DepthImage.width()), _height(DepthImage.height()) {}
		inline color_depth(image<color_format>& ColorImage, depth_format* DepthData) :
			colorImage(ColorImage.data()), depthImage(DepthData), _width(ColorImage.width()), _height(ColorImage.height()) {}

		inline uint32_t width() const { return _width; }
		inline uint32_t height() const { return _height; }
		inline const uint32_t area() const { return _width * _height; }

		void clear_color(color_format clear_value) {
			std::fill_n(colorImage, area(), clear_value);
		}

		void clear_depth_buffer(depth_format clear_value = std::numeric_limits<depth_format>::max()) {
			std::fill_n(depthImage, area(), clear_value);
		}

		template <typename Shader>
		using raster_adapter = raster_adapter::with_ubo<color_depth, Shader>;

		template <typename Shader>
		raster_adapter<Shader> get_raster_adapter(const typename Shader::fragment::uniform_buffer& ubo) {
			return raster_adapter<Shader>(*this, ubo);
		}

		template <typename Shader>
		void draw(
			uint32_t x, uint32_t y,
			const typename Shader::vertex::output* triangle,
			const typename Shader::fragment::uniform_buffer& uniform_buffer,
			const glm::ivec3& results, int area
		) {
			rast_framebuffer_shader_output_assert

			// depth test
			depth_format& oldDepth = depth(x, y);
			depth_format newDepth = get_depth<depth_format>(triangle, results, area);
			if (DepthTest(newDepth, oldDepth)) {

				if constexpr (is_discardable_v<fragment_output>) {
					fragment_output frag = Shader::fragment::shade(
						interpol::interpolate(triangle[0].data, triangle[1].data, triangle[2].data, interpol::persp_coefs(results, area, triangle)),
						uniform_buffer
					);
					if (should_discard(frag)) return;

					color(x, y) = AlphaBlend(
						get_frag_from_discardable(frag),
						color(x, y)
					);
					oldDepth = newDepth;
				}
				else {
					color(x, y) = AlphaBlend(
						Shader::fragment::shade(
							interpol::interpolate(triangle[0].data, triangle[1].data, triangle[2].data, interpol::persp_coefs(results, area, triangle)),
							uniform_buffer
						),
						color(x, y)
					);
					oldDepth = newDepth;
				}
			}
		}
	};

	using rgba8_u8 = color_depth<rast::color::rgba8, uint8_t>;
	using rgba8_u16 = color_depth<rast::color::rgba8, uint16_t>;
	using rgba8_u32 = color_depth<rast::color::rgba8, uint32_t>;
	using rgba8_u64 = color_depth<rast::color::rgba8, uint64_t>;

	using rgb8_u8 = color_depth<rast::color::rgb8, uint8_t>;
	using rgb8_u16 = color_depth<rast::color::rgb8, uint16_t>;
	using rgb8_u32 = color_depth<rast::color::rgb8, uint32_t>;
	using rgb8_u64 = color_depth<rast::color::rgb8, uint64_t>;

	using rgba8_f = color_depth<rast::color::rgb8, float>;
	using rgba8_d = color_depth<rast::color::rgb8, double>;

	using rgb8_f = color_depth<rast::color::rgb8, float>;
	using rgb8_d = color_depth<rast::color::rgb8, double>;

	template <typename ColorFormat, alpha_blend::function::type<ColorFormat> AlphaBlend = default_alpha_blend::blend>
	class color {
	public:
		using color_format = ColorFormat;

	private:
		color_format* colorImage;
		uint32_t _width;
		uint32_t _height;

		inline color_format& at(uint32_t x, uint32_t y) { return colorImage[y * _width + x]; }

	public:
		inline color(color_format* ColorData, uint32_t Width, uint32_t Height) :
			colorImage(ColorData), _width(Width), _height(Height) { }
		inline color(image<color_format>& ColorImage) :
			colorImage(ColorImage), _width(ColorImage.width()), _height(ColorImage.height()) { }

		inline uint32_t width() const { return _width; }
		inline uint32_t height() const { return _height; }
		inline const uint32_t area() const { return _width * _height; }

		void clear(color clear_value) {
			std::fill_n(colorImage, area(), clear_value);
		}

		template <typename Shader>
		void draw(
			uint32_t x, uint32_t y,
			const typename Shader::vertex::output* triangle,
			const typename Shader::fragment::uniform_buffer& uniform_buffer,
			const glm::ivec3& results, int area
		) {
			rast_framebuffer_shader_output_assert

			if constexpr (is_discardable_v<fragment_output>) {
				fragment_output frag = Shader::fragment::shade(
					interpol::interpolate(triangle[0].data, triangle[1].data, triangle[2].data, interpol::persp_coefs(results, area, triangle)),
					uniform_buffer
				);
				if (should_discard(frag)) return;

				at(x, y) = AlphaBlend(
					get_frag_from_discardable(frag),
					at(x, y)
				);
			}
			else {
				at(x, y) = AlphaBlend(
					Shader::fragment::shade(
						interpol::interpolate(triangle[0].data, triangle[1].data, triangle[2].data, interpol::persp_coefs(results, area, triangle)),
						uniform_buffer
					),
					at(x, y)
				);
			}
		}
	};

	using rgb8 = color<rast::color::rgb8>;
	using rgba8 = color<rast::color::rgba8>;

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
		using raster_adapter = raster_adapter::framebuffer_only<depth_view, Shader>;

		template <typename Shader>
		raster_adapter<Shader> get_raster_adapter(const typename Shader::fragment::uniform_buffer&) {
			return raster_adapter<Shader>(*this);
		}

		template <typename Shader>
		void draw(
			uint32_t x, uint32_t y,
			const typename Shader::vertex::output* triangle,
			const typename Shader::fragment::uniform_buffer&,
			const glm::ivec3& results, int area
		) {
			// depth test
			depth_format& oldDepth = this->depth(x, y);
			float depth = get_float_depth(triangle, results, area);
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
