#pragma once
#include <algorithm>
#include <type_traits>

#include <glm/glm.hpp>

#include "../interpolation.hpp"
#include "../depth_test.hpp"
#include "utils.hpp"
#include "basic_depth.hpp"
#include "../simd.hpp"

namespace rast::framebuffer {
	template <depth_test::function::type<float> DepthTest = depth_test::less>
	class visibility : public basic_depth<float> {
	public:
		struct visibility_format {
			size_t primitive_id;
			uint32_t draw_call_id;
			glm::vec3 interpolation_coefs;
		};
		using depth_format = float;
		using size_type = typename sized2d_base::size_type;

	private:
		visibility_format* _visibility_data;

	protected:
		inline constexpr visibility_format& visibility_(size_type x, size_type y) {
			return _visibility_data[data_offset(x, y)];
		}

	public:
		inline constexpr visibility(
			visibility_format* ColorData, depth_format* DepthData,
			size_type Width, size_type Height
		) noexcept : basic_depth<float>(DepthData, Width, Height), _visibility_data(ColorData) { }

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

		inline void clear_visibility(visibility_format clear_value = std::numeric_limits<visibility_format>::max()) {
			std::fill_n(_visibility_data, area(), clear_value);
		}

		template <typename VertexT, typename ...Args>
		inline void operator()(
			size_type x, size_type y,
			const VertexT* triangle,
			glm::vec3 partial_coefs,
			uint32_t draw_call_id,
			size_t primitive_id
		) {
			depth_format& oldDepth = depth(x, y);
			depth_format newDepth = get_depth<depth_format>(triangle, partial_coefs);
			if (DepthTest(newDepth, oldDepth)) {
				visibility_(x, y) = visibility_format{ primitive_id, draw_call_id, partial_coefs };
				oldDepth = newDepth;
			}
		}
		template <size_t Count, typename VertexT, typename ...Args>
		inline void draw(
			simd::i32x_<Count> x, simd::i32x_<Count> y,
			const VertexT* triangle,
			glm::vec<3, simd::f32x_<Count>> partial_coefs,
			simd::i32x_<Count> mask,
			uint32_t draw_call_id,
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
						_visibility_data[offsets[i]] = visibility_format{
							primitive_id, draw_call_id,
							glm::vec3(partial_coefs.x[i], partial_coefs.y[i], partial_coefs.z[i])
						};
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
			uint32_t draw_call_id,
			size_t primitive_id
		) {
			draw<4>(x, y, triangle, partial_coefs, mask, draw_call_id, primitive_id);
		}
		template <typename VertexT, typename ...Args>
		inline void operator()(
			simd::i32x8 x, simd::i32x8 y,
			const VertexT* triangle,
			glm::vec<3, simd::f32x8> partial_coefs,
			simd::i32x8 mask,
			uint32_t draw_call_id,
			size_t primitive_id
		) {
			draw<8>(x, y, triangle, partial_coefs, mask, draw_call_id, primitive_id);
		}
	};
}
