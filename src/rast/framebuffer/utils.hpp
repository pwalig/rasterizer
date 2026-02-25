#pragma once
#include "../math/vec.hpp"
#include "../interpolation.hpp"
#include "../discard_fragment.hpp"
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
	inline static const bool matching_format_v = matching_format<ColorFormat, FragmentOutput>();

	template <typename DepthFormat>
	inline DepthFormat float_depth_to_depth_format(float depth) {
		if constexpr (std::is_same_v<DepthFormat, float>) return depth;
		else if constexpr (std::is_floating_point_v<DepthFormat>) return static_cast<DepthFormat>(depth);
		else return static_cast<DepthFormat>(
			(depth * 0.5f + 0.5f) * std::numeric_limits<DepthFormat>::max()
		);
	}
	template <typename DepthFormat, size_t Count>
	inline constexpr math::_scalar<DepthFormat, Count> float_depth_to_depth_format(math::f32x<Count> depth) {
		if constexpr (std::is_same_v<DepthFormat, float>) return depth;
		else if constexpr (std::is_floating_point_v<DepthFormat>) return math::cast<DepthFormat>(depth);
		else return math::cast<DepthFormat>(
			(depth * math::f32x<Count>(0.5f) + math::f32x<Count>(0.5f)) *
			math::f32x<Count>(static_cast<float>(std::numeric_limits<DepthFormat>::max()))
		);
		
	}

	template <typename Vec3>
	inline constexpr float get_float_depth(
		float z0, float z1, float z2,
		Vec3 partial_coefs
	) {
		return interpol::interpolate(
			z0, z1, z2,
			interpol::coefs::linear(partial_coefs)
		);
	}
	template <size_t Count>
	inline constexpr math::f32x<Count> get_float_depth(
		float z0, float z1, float z2,
		math::f32vec3x<Count> partial_coefs
	) {
		return interpol::interpolate(
			math::f32x<Count>(z0),
			math::f32x<Count>(z1),
			math::f32x<Count>(z2),
			interpol::coefs::linear(partial_coefs)
		);
	}

	template <typename vertex_output>
	inline constexpr float get_float_depth(const vertex_output* triangle, glm::vec3 partial_coefs) {
		return interpol::interpolate(
			triangle[0].rastPos.z,
			triangle[1].rastPos.z,
			triangle[2].rastPos.z,
			interpol::coefs::linear(partial_coefs)
		);
	}
	template <typename depth_format, typename vertex_output>
	inline depth_format get_depth(const vertex_output* triangle, glm::vec3 partial_coefs) {
		return float_depth_to_depth_format<depth_format>(get_float_depth(triangle, partial_coefs));
	}
	template <typename depth_format>
	inline depth_format get_depth(float z0, float z1, float z2, glm::vec3 partial_coefs) {
		return float_depth_to_depth_format<depth_format>(get_float_depth(z0, z1, z2, partial_coefs));
	}
	template <typename depth_format, size_t Count>
	inline constexpr math::_scalar<depth_format, Count> get_depth(
		float z0, float z1, float z2,
		math::f32vec3x<Count> partial_coefs
	) {
		return float_depth_to_depth_format<depth_format>(
			get_float_depth(z0, z1, z2, partial_coefs)
		);
	}

	using default_alpha_blend = rast::alpha_blend::func<
		rast::alpha_blend::factor::src_alpha,
		rast::alpha_blend::factor::one_minus_src_alpha,
		rast::alpha_blend::equation::add
	>;

	namespace test {
		constexpr auto dpth = get_float_depth(
			0.0f, 1.0f, 2.0f, math::f32vec3x4(
				math::make_f32x4(1.0f, 0.0f, 0.0f, 0.33f),
				math::make_f32x4(0.0f, 1.0f, 0.0f, 0.33f),
				math::make_f32x4(0.0f, 0.0f, 1.0f, 0.33f)
			)
		);
		static_assert(dpth[0] == 0.0f);
		static_assert(dpth[1] == 1.0f);
		static_assert(dpth[2] == 2.0f);
		static_assert(dpth[3] == 1.0f);
		constexpr auto dpth2 = get_depth<uint8_t>(
			0.0f, -0.5f, 0.5f, math::f32vec3x4(
				math::make_f32x4(1.0f, 0.0f, 0.0f, 0.33f),
				math::make_f32x4(0.0f, 1.0f, 0.0f, 0.33f),
				math::make_f32x4(0.0f, 0.0f, 1.0f, 0.33f)
			)
		);
		static_assert(dpth2[0] == 127);
		static_assert(dpth2[1] == 63);
		static_assert(dpth2[2] == 191);
		static_assert(dpth2[3] == 127);
	}
}
