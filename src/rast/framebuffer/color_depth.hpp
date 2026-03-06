#pragma once
#include <algorithm>
#include <type_traits>

#include <glm/glm.hpp>

#include "../interpolation.hpp"
#include "../convert.hpp"
#include "../alpha_blend.hpp"
#include "../is_discardable.hpp"
#include "../depth_test.hpp"
#include "utils.hpp"
#include "../sized2d_base.hpp"

namespace rast::framebuffer {
	template<
		typename ColorFormat, typename DepthFormat,
		auto FragmentShader,
		auto AlphaBlend = default_alpha_blend::blend<ColorFormat>,
		depth_test::function::type<DepthFormat> DepthTest = depth_test::less
	> class color_depth : public sized2d_base {
	public:
		using color_format = ColorFormat;
		using depth_format = DepthFormat;
		using size_type = typename sized2d_base::size_type;

	private:
		color_format* _color_data;
		depth_format* _depth_data;

	protected:
		inline constexpr color_format& color(size_type x, size_type y) {
			return _color_data[data_offset(x, y)];
		}
		inline constexpr depth_format& depth(size_type x, size_type y) {
			return _depth_data[data_offset(x, y)];
		}
		template <size_t Count>
		inline math::_scalar<depth_format*, Count> depth_data(
			math::u32x<Count> x, math::u32x<Count> y
		) {
			return math::vectorize<Count>(_depth_data) + (y * math::vectorize<Count>(_width) + x);
		}

	public:
		inline constexpr color_depth(
			color_format* ColorData, depth_format* DepthData,
			size_type Width, size_type Height
		) noexcept : sized2d_base(Width, Height), _color_data(ColorData), _depth_data(DepthData) { }

		template <typename ImageLike1, typename ImageLike2>
		inline constexpr color_depth(ImageLike1& ColorImage, ImageLike2& DepthImage) :
			color_depth(ColorImage.data(), DepthImage.data(), ColorImage.width(), ColorImage.height())
		{
			static_assert(std::is_same_v<typename ImageLike1::value_type, color_format>);
			static_assert(std::is_same_v<typename ImageLike2::value_type, DepthFormat>);
			assert(ColorImage.width() == DepthImage.width());
			assert(ColorImage.height() == DepthImage.height());
		}

		template <typename ImageLike>
		inline constexpr color_depth(color_format* ColorData, ImageLike& DepthImage) :
			color_depth(ColorData, DepthImage.data(), DepthImage.width(), DepthImage.height())
		{
			static_assert(std::is_same_v<typename ImageLike::value_type, depth_format>);
		}
		
		template <typename ImageLike>
		inline constexpr color_depth(ImageLike& ColorImage, depth_format* DepthData) :
			color_depth(ColorImage.data(), DepthData, ColorImage.width(), ColorImage.height())
		{
			static_assert(std::is_same_v<typename ImageLike::value_type, color_format>);
		}

		void clear_color(color_format clear_value) {
			std::fill_n(_color_data, area(), clear_value);
		}

		void clear_depth_buffer(depth_format clear_value = std::numeric_limits<depth_format>::max()) {
			std::fill_n(_depth_data, area(), clear_value);
		}

		template <typename VertexT, typename ...Args>
		inline void operator()(
			size_type x, size_type y,
			const VertexT* triangle,
			glm::vec3 partial_coefs,
			Args&&... args
		) {
			depth_format& oldDepth = depth(x, y);
			depth_format newDepth = get_depth<depth_format>(triangle, partial_coefs);
			if (DepthTest(newDepth, oldDepth)) {
				auto frag = FragmentShader(
					interpol::interpolate(triangle[0].data, triangle[1].data, triangle[2].data, interpol::coefs::perspective(partial_coefs, triangle)),
					std::forward<Args>(args)...
				);
				if constexpr (is_discardable_v<decltype(frag)>) {
					if (frag) {
						color(x, y) = AlphaBlend(*frag, color(x, y));
						oldDepth = newDepth;
					}
				}
				else {
					color(x, y) = AlphaBlend(frag, color(x, y));
					oldDepth = newDepth;
				}
			}
		}
		template <typename VertexT, typename ...Args>
		inline void operator()(
			math::simd::i32x4 x, math::simd::i32x4 y,
			const VertexT* triangle,
			glm::vec<3, math::simd::f32x4> partial_coefs,
			math::boolx4 mask,
			Args&&... args
		) {
			math::simd::i32x4 off = data_offset(x, y);

			alignas(16) int offsets[4];
			math::simd::store(offsets, off);

			alignas(16) float depths[4] = {
				mask[0] ? _depth_data[offsets[0]] : std::numeric_limits<float>::min(),
				mask[1] ? _depth_data[offsets[1]] : std::numeric_limits<float>::min(),
				mask[2] ? _depth_data[offsets[2]] : std::numeric_limits<float>::min(),
				mask[3] ? _depth_data[offsets[3]] : std::numeric_limits<float>::min()
			};
			math::simd::f32x4 old_depth = _mm_load_ps(depths);
			math::simd::f32x4 new_depth = get_float_depth(
				triangle[0].rastPos.z,
				triangle[1].rastPos.z,
				triangle[2].rastPos.z,
				partial_coefs
			);
			math::simd::f32x4 depth_mask = _mm_cmplt_ps(new_depth, old_depth);
			mask[0] &= (depth_mask[0] != 0.0f);
			mask[1] &= (depth_mask[1] != 0.0f);
			mask[2] &= (depth_mask[2] != 0.0f);
			mask[3] &= (depth_mask[3] != 0.0f);
			if (math::or_accross(mask)) {
				math::simd::store(depths, new_depth);
				std::array<color_format, 4> colors = {
					mask[0] ? _color_data[offsets[0]] : color_format(),
					mask[1] ? _color_data[offsets[1]] : color_format(),
					mask[2] ? _color_data[offsets[2]] : color_format(),
					mask[3] ? _color_data[offsets[3]] : color_format()
				};
				auto frag = FragmentShader(
					interpol::interpolate(
						triangle[0].data, triangle[1].data, triangle[2].data,
						interpol::coefs::perspective(
							partial_coefs,
							triangle[0].rastPos.w,
							triangle[1].rastPos.w,
							triangle[2].rastPos.w
						)
					), std::forward<Args>(args)...
				);
				if constexpr (is_discardable_v<decltype(frag)>) {
					mask &= math::boolx4(frag);
				}
				colors = AlphaBlend(frag, colors);
				for (size_t i = 0; i < 4; ++i) {
					if (!mask[i]) continue;
					_color_data[offsets[i]] = colors[i];
					_depth_data[offsets[i]] = depths[i];
				}
			}
		}
	};
}
