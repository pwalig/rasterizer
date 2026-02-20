#pragma once
#include <type_traits>

namespace rast::math {
	template <typename U, typename T>
	inline constexpr U trunc(T x) {
		static_assert(std::is_floating_point_v<T>);
		static_assert(std::is_arithmetic_v<U>);

		if constexpr (std::is_floating_point_v<U>)
			return static_cast<U>(static_cast<int64_t>(x));
		else return static_cast<U>(x);
	}
	template <typename U, typename T>
	inline constexpr U round(T x) {
		return trunc<U>(x >= T(0.0) ? x + T(0.5) : x - T(0.5));
	}
	template <typename U, typename T>
	inline constexpr U floor(T x) {
		return trunc<U>(x >= T(0.0) ? x : x - T(1.0));
	}
	template <typename U, typename T>
	inline constexpr U ceil(T x) {
		return trunc<U>(x >= T(0.0) ? x + T(1.0) : x);
	}
	template <typename T>
	inline constexpr T abs(T x) noexcept {
		return x < 0 ? -x : x;
	}
	static_assert(trunc<int>(3.14f) == 3);
	static_assert(trunc<float>(3.14f) == 3.0f);
	static_assert(trunc<float>(-3.14f) == -3.0f);
	static_assert(floor<float>(3.14f) == 3.0f);
	static_assert(floor<float>(-3.14f) == -4.0f);
}
