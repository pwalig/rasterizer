#pragma once
#include <type_traits>
#include "../sized2d_base.hpp"

namespace rast::framebuffer {
	template <typename DepthFormat>
	struct basic_depth : sized2d_base {
		using depth_format = DepthFormat;
		using size_type = typename sized2d_base::size_type;
		using value_type = depth_format;
		using reference = std::add_lvalue_reference_t<value_type>;
		using pointer = std::add_pointer_t<value_type>;
		using const_reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;
		using const_pointer = std::add_pointer_t<std::add_const_t<value_type>>;

	protected:
		pointer _depth_data;

	public:
		constexpr basic_depth(
			pointer DepthData, size_type Width, size_type Height
		) noexcept : sized2d_base(Width, Height), _depth_data(DepthData) {}

		template <typename ImageLike>
		constexpr basic_depth(ImageLike& DepthImage) noexcept : 
			basic_depth(DepthImage.data(), DepthImage.width(), DepthImage.height()) {}

		constexpr reference depth(size_type x, size_type y) {
			return _depth_data[data_offset(x, y)];
		}

		constexpr const_reference depth(size_type x, size_type y) const {
			return _depth_data[data_offset(x, y)];
		}
		void clear_depth_buffer(depth_format clear_value = std::numeric_limits<depth_format>::max()) {
			std::fill_n(_depth_data, area(), clear_value);
		}
	};
}
