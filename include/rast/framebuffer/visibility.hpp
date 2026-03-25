#pragma once
#include <algorithm>
#include <type_traits>

#include <glm/glm.hpp>

#include "../interpolation.hpp"
#include "../depth_test.hpp"
#include "utils.hpp"
#include "../sized2d_base.hpp"
#include "../simd.hpp"

namespace rast::framebuffer {
	template <depth_test::function::type<float> DepthTest = depth_test::less>
	class visibility : public sized2d_base {
	public:
		using visibility_format = size_t;
		using depth_format = float;
		using size_type = typename sized2d_base::size_type;

	private:
		visibility_format* _visibility_data;
		depth_format* _depth_data;

	protected:
		inline constexpr visibility_format& visibility_(size_type x, size_type y) {
			return _visibility_data[data_offset(x, y)];
		}
		inline constexpr depth_format& depth(size_type x, size_type y) {
			return _depth_data[data_offset(x, y)];
		}

	public:
		inline constexpr visibility(
			visibility_format* ColorData, depth_format* DepthData,
			size_type Width, size_type Height
		) noexcept : sized2d_base(Width, Height), _visibility_data(ColorData), _depth_data(DepthData) { }

		template <typename ImageLike1, typename ImageLike2>
		inline constexpr visibility(ImageLike1& VisibilityImage, ImageLike2& DepthImage) :
			visibility(VisibilityImage.data(), DepthImage.data(), VisibilityImage.width(), VisibilityImage.height())
		{
			static_assert(std::is_same_v<typename ImageLike1::value_type, visibility_format>);
			static_assert(std::is_same_v<typename ImageLike2::value_type, depth_format>);
			assert(VisibilityImage.width() == DepthImage.width());
			assert(VisibilityImage.height() == DepthImage.height());
		}

		template <typename ImageLike>
		inline constexpr visibility(visibility_format* VisibilityData, ImageLike& DepthImage) :
			visibility(VisibilityData, DepthImage.data(), DepthImage.width(), DepthImage.height())
		{
			static_assert(std::is_same_v<typename ImageLike::value_type, depth_format>);
		}
		
		template <typename ImageLike>
		inline constexpr visibility(ImageLike& VisibilityImage, depth_format* DepthData) :
			visibility(VisibilityImage.data(), DepthData, VisibilityImage.width(), VisibilityImage.height())
		{
			static_assert(std::is_same_v<typename ImageLike::value_type, visibility_format>);
		}

		void clear_visibility(visibility_format clear_value = std::numeric_limits<visibility_format>::max()) {
			std::fill_n(_visibility_data, area(), clear_value);
		}

		void clear_depth_buffer(depth_format clear_value = std::numeric_limits<depth_format>::max()) {
			std::fill_n(_depth_data, area(), clear_value);
		}

		template <typename VertexT, typename ...Args>
		inline void operator()(
			size_type x, size_type y,
			const VertexT* triangle,
			glm::vec3 partial_coefs,
			size_t primitive_id
		) {
			depth_format& oldDepth = depth(x, y);
			depth_format newDepth = get_depth<depth_format>(triangle, partial_coefs);
			if (DepthTest(newDepth, oldDepth)) {
				visibility_(x, y) = primitive_id;
				depth(x, y) = newDepth;
			}
		}
		template <size_t Count, typename VertexT, typename ...Args>
		inline void draw(
			simd::i32x_<Count> x, simd::i32x_<Count> y,
			const VertexT* triangle,
			glm::vec<3, simd::f32x_<Count>> partial_coefs,
			simd::i32x_<Count> mask,
			size_t primitive_id
		) {
			simd::i32x_<Count> off = data_offset(x, y);

			alignas(Count * sizeof(int)) int offsets[Count];
			simd::store(offsets, off);

			alignas(Count * sizeof(int)) float depths[Count];
			for (size_t i = 0; i < Count; ++i) if (mask[static_cast<int>(i)]) depths[i] = _depth_data[offsets[i]];
			simd::f32x_<Count> old_depth = simd::load<float, Count>(depths);
			simd::f32x_<Count> new_depth = get_float_depth(
				triangle[0].rastPos.z,
				triangle[1].rastPos.z,
				triangle[2].rastPos.z,
				partial_coefs
			);
			simd::f32x_<Count> depth_mask = new_depth < old_depth;
			mask &= simd::reinterpret<int, 4>(depth_mask);
			if (simd::movemask(mask)) {
				simd::store(depths, new_depth);
				for (size_t i = 0; i < Count; ++i) {
					if (mask[static_cast<int>(i)]) {
						_visibility_data[offsets[i]] = primitive_id;
						_depth_data[offsets[i]] = depths[i];
					}
				}
			}
		}
		template <typename VertexT, typename ...Args>
		inline void operator()(
			simd::i32x4 x, simd::i32x4 y,
			const VertexT* triangle,
			glm::vec<3, simd::f32x4> partial_coefs,
			simd::i32x4 mask,
			size_t primitive_id
		) {
			draw<4>(x, y, triangle, partial_coefs, mask, primitive_id);
		}
		template <typename VertexT, typename ...Args>
		inline void operator()(
			simd::i32x8 x, simd::i32x8 y,
			const VertexT* triangle,
			glm::vec<3, simd::f32x8> partial_coefs,
			simd::i32x8 mask,
			size_t primitive_id
		) {
			draw<8>(x, y, triangle, partial_coefs, mask, primitive_id);
		}
	};
}
