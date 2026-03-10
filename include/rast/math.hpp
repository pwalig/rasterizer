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
	inline constexpr U floor(T x) {
		return trunc<U>(x - (x < static_cast<T>(0)));
	}
	template <typename U, typename T>
	inline constexpr U ceil(T x) {
		return trunc<U>(x + (x >= static_cast<T>(0)));
	}
	template <typename U, typename T>
	inline constexpr U round(T x) {
		return floor<U>(x + static_cast<T>(0.5));
	}
	template <typename T>
	inline constexpr T abs(T x) noexcept {
		return x < static_cast<T>(0) ? -x : x;
	}
	template <typename T>
	inline constexpr T sqrt_helper(T x, T curr, T prev) {
		return curr == prev ? curr : sqrt_helper(x, static_cast<T>(0.5) * (curr + x / curr), curr);
	}
	template <typename T, std::enable_if_t<std::is_arithmetic<T>::value, bool> = true>
	inline constexpr T sqrt(T x) {
		return sqrt_helper(x, x, static_cast<T>(0));
	}
	static_assert(trunc<int>(3.14f) == 3);
	static_assert(trunc<int>(-3.14f) == -3);
	static_assert(trunc<float>(3.14f) == 3.0f);
	static_assert(trunc<float>(-3.14f) == -3.0f);

	static_assert(floor<int>(3.14f) == 3);
	static_assert(floor<int>(-3.14f) == -4);
	static_assert(floor<float>(3.14f) == 3.0f);
	static_assert(floor<float>(-3.14f) == -4.0f);

	static_assert(ceil<int>(3.14f) == 4);
	static_assert(ceil<int>(-3.14f) == -3);
	static_assert(ceil<float>(3.14f) == 4.0f);
	static_assert(ceil<float>(-3.14f) == -3.0f);

	static_assert(round<int>(3.14f) == 3);
	static_assert(round<int>(-3.14f) == -3);
	static_assert(round<float>(3.14f) == 3.0f);
	static_assert(round<float>(-3.14f) == -3.0f);
	static_assert(round<int>(3.74f) == 4);
	static_assert(round<int>(-3.74f) == -4);
	static_assert(round<float>(3.74f) == 4.0f);
	static_assert(round<float>(-3.74f) == -4.0f);

	static_assert(abs(3.14f) == 3.14f);
	static_assert(abs(-3.14f) == 3.14f);

	static_assert(sqrt(12.25f) == 3.5f);
}
