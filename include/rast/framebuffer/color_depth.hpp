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
#include "../simd.hpp"

namespace rast::framebuffer {
	template<
		typename ColorFormat,
		auto FragmentShader,
		auto AlphaBlend = default_alpha_blend::blend<ColorFormat>,
		depth_test::function_t<> DepthTest = depth_test::less
	> class color_depth : public sized2d_base {
	public:
		using color_format = ColorFormat;
		using depth_format = float;
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

	public:
		constexpr color_depth(
			color_format* ColorData, depth_format* DepthData,
			size_type Width, size_type Height
		) noexcept : sized2d_base(Width, Height), _color_data(ColorData), _depth_data(DepthData) { }

		template <typename ImageLike1, typename ImageLike2>
		constexpr color_depth(ImageLike1& ColorImage, ImageLike2& DepthImage) :
			color_depth(ColorImage.data(), DepthImage.data(), ColorImage.width(), ColorImage.height())
		{
			static_assert(std::is_same_v<typename ImageLike1::value_type, color_format>);
			static_assert(std::is_same_v<typename ImageLike2::value_type, depth_format>);
			assert(ColorImage.width() == DepthImage.width());
			assert(ColorImage.height() == DepthImage.height());
		}

		template <typename ImageLike>
		constexpr color_depth(color_format* ColorData, ImageLike& DepthImage) :
			color_depth(ColorData, DepthImage.data(), DepthImage.width(), DepthImage.height())
		{
			static_assert(std::is_same_v<typename ImageLike::value_type, depth_format>);
		}
		
		template <typename ImageLike>
		constexpr color_depth(ImageLike& ColorImage, depth_format* DepthData) :
			color_depth(ColorImage.data(), DepthData, ColorImage.width(), ColorImage.height())
		{
			static_assert(std::is_same_v<typename ImageLike::value_type, color_format>);
		}

		constexpr void clear_color(color_format clear_value) {
			std::fill_n(_color_data, area(), clear_value);
		}

		constexpr void clear_depth_buffer(depth_format clear_value = std::numeric_limits<depth_format>::max()) {
			std::fill_n(_depth_data, area(), clear_value);
		}

		template <typename VertexT, typename ...Args>
		inline constexpr void operator()(
			size_type x, size_type y,
			const VertexT* triangle,
			glm::vec3 partial_coefs,
			Args&&... args
		) {
			depth_format& oldDepth = depth(x, y);
			depth_format newDepth = interpol::depth(triangle, partial_coefs);
			if (DepthTest(newDepth, oldDepth)) {
				auto frag = FragmentShader(
					interpol::perspective(triangle, partial_coefs),
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
		template <size_t Count, typename VertexT, typename ...Args>
		inline void draw(
			simd::i32x_<Count> x, simd::i32x_<Count> y,
			const VertexT* triangle,
			glm::vec<3, simd::f32x_<Count>> partial_coefs,
			simd::i32x_<Count> mask,
			Args&&... args
		) {
			simd::u32x_<Count> off = data_offset<Count>(simd::cvt<size_type>(x), simd::cvt<size_type>(y));

			alignas(Count * sizeof(size_type)) size_type offsets[Count];
			simd::store(offsets, off);

			alignas(Count * sizeof(int)) float depths[Count];
			for (size_t i = 0; i < Count; ++i) if (mask[static_cast<int>(i)]) depths[i] = _depth_data[offsets[i]];
			simd::f32x_<Count> old_depth = simd::load<float, Count>(depths);

			simd::f32x_<Count> new_depth = interpol::depth(triangle, partial_coefs);
			simd::f32x_<Count> depth_mask = new_depth < old_depth;
			mask &= simd::reinterpret<int, 4>(depth_mask);
			if (simd::movemask(mask)) {
				simd::store(depths, new_depth);
				auto colors = std::array<color_format, Count>();
				for (size_t i = 0; i < Count; ++i)
					if (mask[static_cast<int>(i)])
						colors[i] = _color_data[offsets[i]];
				auto frag = FragmentShader(
					interpol::perspective(triangle, partial_coefs),
					std::forward<Args>(args)...
				);
				if constexpr (is_discardable_v<decltype(frag)>) {
					mask &= simd::i32x_<Count>(frag);
					colors = AlphaBlend(*frag, colors);
				}
				else colors = AlphaBlend(frag, colors);
				for (size_t i = 0; i < Count; ++i) {
					if (!mask[static_cast<int>(i)]) continue;
					_color_data[offsets[i]] = colors[i];
					_depth_data[offsets[i]] = depths[i];
				}
			}
		}
		template <typename VertexT, typename ...Args>
		inline void operator()(
			simd::i32x4 x, simd::i32x4 y,
			const VertexT* triangle,
			glm::vec<3, simd::f32x4> partial_coefs,
			simd::i32x4 mask,
			Args&&... args
		) {
			draw<4>(x, y, triangle, partial_coefs, mask, std::forward<Args>(args)...);
		}
		template <typename VertexT, typename ...Args>
		inline void operator()(
			simd::i32x8 x, simd::i32x8 y,
			const VertexT* triangle,
			glm::vec<3, simd::f32x8> partial_coefs,
			simd::i32x8 mask,
			Args&&... args
		) {
			draw<8>(x, y, triangle, partial_coefs, mask, std::forward<Args>(args)...);
		}
	};
}
