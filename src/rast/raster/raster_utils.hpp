#pragma once
#include <glm/glm.hpp>
#include "../math/vec.hpp"

#include "../viewport.hpp"
#include "../tile.hpp"

namespace rast::raster {
	inline constexpr glm::ivec2 to_screen_space(const glm::vec4& vertex, const viewport& viewport) {
		return glm::ivec2(
			(( vertex.x + 1.0f ) * (float)viewport.extent.x * 0.5f + (float)viewport.offset.x),
			(( -vertex.y + 1.0f ) * (float)viewport.extent.y * 0.5f + (float)viewport.offset.y)
		);
	}

	template <size_t Count>
	inline constexpr math::i32vec2x<Count> to_screen_space(
		const math::f32x<Count>& x,
		const math::f32x<Count>& y,
		const viewport_x<Count>& Viewport
	) {
		return math::i32vec2x<Count>(
			(x + math::f32x<Count>(1.0f)) * math::cast<float>(Viewport.extent[0]) * math::f32x<Count>(0.5f) + math::cast<float>(Viewport.offset[0]),
			(-y + math::f32x<Count>(1.0f)) * math::cast<float>(Viewport.extent[1]) * math::f32x<Count>(0.5f) + math::cast<float>(Viewport.offset[1])
		);
	}
	template <size_t Count>
	inline constexpr math::i32vec2x<Count> to_screen_space(
		const math::f32x<Count>& x,
		const math::f32x<Count>& y,
		const viewport& Viewport
	) {
		return to_screen_space(x, y, viewport_x<Count>(Viewport.offset.x, Viewport.offset.y, Viewport.extent.x, Viewport.extent.y));
	}

	template <typename T, typename EquationResults>
	inline constexpr T partial_coefs(EquationResults E, typename EquationResults::value_type area) {
		static_assert(std::is_integral_v<typename EquationResults::value_type>);
		using Float = typename T::value_type;
		static_assert(std::is_floating_point_v<Float>);
		return T(
			static_cast<Float>(E.y) / area,
			static_cast<Float>(E.z) / area,
			static_cast<Float>(E.x) / area
		);
	}

	template <typename Shader, typename Callable>
	using function = void(*)(
		Callable&& output,
		const typename Shader::vertex::output* vertex_begin,
		const typename Shader::vertex::output* vertex_end,
		const viewport& viewport,
		const tile& tile
	);


	template <typename T>
	inline constexpr T fill_convention(T Dx, T Dy) {
		static_assert(std::is_integral_v<typename T::value_type>);
		return T(
			(Dy.x > 0 || (Dy.x == 0 && Dx.x < 0)) ? 1 : 0,
			(Dy.y > 0 || (Dy.y == 0 && Dx.y < 0)) ? 1 : 0,
			(Dy.z > 0 || (Dy.z == 0 && Dx.z < 0)) ? 1 : 0
		);
	}

	template <size_t Count>
	inline constexpr math::i32vec3x<Count> fill_convention(math::i32vec3x<Count> Dx, math::i32vec3x<Count> Dy) {
		auto zero = math::i32x<Count>(0);
		return math::i32vec3x<Count>(
			math::cast<int>(Dy[0] > zero || (Dy[0] == zero && (Dx[0] < zero))),
			math::cast<int>(Dy[1] > zero || (Dy[1] == zero && (Dx[1] < zero))),
			math::cast<int>(Dy[2] > zero || (Dy[2] == zero && (Dx[2] < zero)))
		);
	}

	template <typename Rasterizer, typename Shader, typename Callable>
	inline void execute(
		Callable&& output,
		const typename Shader::vertex::output* vertex_begin,
		const typename Shader::vertex::output* vertex_end,
		const viewport& viewport,
		const tile& tile
	) {
		using vertex = typename Shader::vertex::output;
		for (const vertex* triangle = vertex_begin; triangle != vertex_end; triangle += 3) {
			Rasterizer::template rasterize_one<Shader>(output, triangle, viewport, tile);
		}
	}
}
