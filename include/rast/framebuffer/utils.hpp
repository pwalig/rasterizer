#pragma once
#include "../interpolation.hpp"
#include "../is_discardable.hpp"
#include "../alpha_blend.hpp"

namespace rast::framebuffer {
	template <typename ColorFormat, typename FragmentOutput>
	inline constexpr bool matching_format() {
		if constexpr (is_discardable_v<FragmentOutput>)
			return std::is_same_v<ColorFormat, typename FragmentOutput::value_type>;
		else
			return std::is_same_v<ColorFormat, FragmentOutput>;
	}
	template <typename ColorFormat, typename FragmentOutput>
	inline constexpr bool matching_format_v = matching_format<ColorFormat, FragmentOutput>();

	template <typename DepthFormat>
	inline constexpr DepthFormat float_depth_to_depth_format(float depth) {
		if constexpr (std::is_same_v<DepthFormat, float>) return depth;
		else if constexpr (std::is_floating_point_v<DepthFormat>) return static_cast<DepthFormat>(depth);
		else if constexpr (std::is_integral_v<DepthFormat>) return static_cast<DepthFormat>(
			(depth * 0.5f + 0.5f) * std::numeric_limits<DepthFormat>::max()
			);
		else static_assert(false, "DepthFormat must represent a numeric type");
	}

	using default_alpha_blend = rast::alpha_blend::func<
		rast::alpha_blend::factor::src_alpha,
		rast::alpha_blend::factor::one_minus_src_alpha,
		rast::alpha_blend::equation::add
	>;
}
