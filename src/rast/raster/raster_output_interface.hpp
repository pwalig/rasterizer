#pragma once
#include <cstdint>

namespace rast::raster {
	enum struct output_interface : uint8_t {
		call, framebuffer, fragbuff
	};

	template <typename T>
	inline constexpr output_interface output_interface_v = output_interface::call;

	template <typename T>
	inline constexpr output_interface output_interface_v<T&> = output_interface_v<T>;
	template <typename T>
	inline constexpr output_interface output_interface_v<T&&> = output_interface_v<T>;
	template <typename T>
	inline constexpr output_interface output_interface_v<const T> = output_interface_v<T>;
	template <typename T>
	inline constexpr output_interface output_interface_v<volatile T> = output_interface_v<T>;

	template <typename Shader, typename Callable, typename ...Args>
	void dispached_output(
		Callable&& output,
		uint32_t x, uint32_t y,
		const typename Shader::vertex::output* triangle,
		const glm::ivec3& equation_results, int area,
		Args&&... args
	) {
		
		if constexpr (output_interface_v<Callable> == output_interface::call) {
			output(x, y, triangle, equation_results, area, std::forward<Args>(args)...);
		}
		else if constexpr (output_interface_v<Callable> == output_interface::framebuffer) {
			output.template draw<Shader>(x, y, triangle, equation_results, area, std::forward<Args>(args)...);
		}
	}
}
