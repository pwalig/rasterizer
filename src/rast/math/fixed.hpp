#pragma once
#include <cstdint>
#include <cmath>
#include <type_traits>
#include "rounding.hpp"

namespace rast::math {
	template <typename T, uint8_t Precision>
	struct fixed_point_arithmetic {
		static_assert(sizeof(T) * 8 > Precision);

		inline static constexpr T one = static_cast<T>(1) << Precision;
		inline static constexpr T half = one >> 1;

		template <typename U>
		inline static constexpr T to_fixed(U Value) {
			if constexpr (std::is_floating_point_v<U>) {
				return math::round<T>(Value * one);
			} else {
				static_assert(std::is_integral_v<U>);
				return static_cast<T>(Value) * one;
			}
		}

		template <typename U>
		inline static constexpr U from_fixed(T Value) {
			static_assert(std::is_integral_v<T>);
			if constexpr (std::is_floating_point_v<U>) {
				return static_cast<U>(Value) / one;
			} else {
				static_assert(std::is_integral_v<U>);
				return static_cast<U>(Value / one);
			}
		}

		inline static constexpr T multiply(T lhs, T rhs) {
			static_assert(std::is_integral_v<T>);
			constexpr T help = static_cast<T>(1) << (Precision / 2);
			return (lhs / help) * (rhs / help);
		}
		inline static constexpr T divide(T lhs, T rhs) {
			static_assert(std::is_integral_v<T>);
			return (lhs / rhs) * one;
		}
		inline static constexpr T trunc(T Value) {
			static_assert(std::is_integral_v<T>);
			return (Value / one) * one;
		}
		inline static constexpr T floor(T Value) {
			return trunc(Value >= 0 ? Value : Value - one);
		}
		inline static constexpr T round(T Value) {
			return trunc(Value >= 0 ? Value + half : Value - half);
		}

		template <typename U>
		inline static constexpr bool back_and_forth_test(U Value) {
			return from_fixed<U>(to_fixed(Value)) == Value;
		}
		template <typename U>
		inline static constexpr bool multiply_test(U a, U b) {
			return multiply(to_fixed(a), to_fixed(b)) == to_fixed(a * b);
		}
		template <typename U>
		inline static constexpr bool floor_test(U Value) {
			return from_fixed<U>(floor(to_fixed(Value))) == math::floor<U>(Value);
		}
	};

	static_assert(fixed_point_arithmetic<uint32_t, 16>::back_and_forth_test(278));
	static_assert(fixed_point_arithmetic<uint32_t, 16>::back_and_forth_test(12.25f));
	static_assert(fixed_point_arithmetic<uint32_t, 2>::to_fixed(1) == 4);
	static_assert(fixed_point_arithmetic<int32_t, 1>::to_fixed(-3) == -6);
	static_assert(fixed_point_arithmetic<uint32_t, 1>::to_fixed(2.5f) == 5);
	static_assert(fixed_point_arithmetic<int32_t, 1>::to_fixed(-0.5f) == -1);
	static_assert(fixed_point_arithmetic<uint32_t, 16>::to_fixed(2.5f) == 163840);
	static_assert(fixed_point_arithmetic<uint32_t, 16>::multiply_test(2.5f, 2.5f));
	static_assert(fixed_point_arithmetic<uint32_t, 2>::floor(5) == 4);
	static_assert(fixed_point_arithmetic<uint32_t, 16>::floor_test(2.5f));
	static_assert(fixed_point_arithmetic<int32_t, 16>::floor_test(-2.5f));

	template <typename T, uint8_t Precision> struct fixed;

	template <typename T, uint8_t Precision> constexpr fixed<T, Precision> round(fixed<T, Precision>);
	template <typename T, uint8_t Precision> constexpr fixed<T, Precision> floor(fixed<T, Precision>);
	template <typename T, uint8_t Precision> constexpr fixed<T, Precision> ceil(fixed<T, Precision>);
	template <typename T, uint8_t Precision> constexpr fixed<T, Precision> trunc(fixed<T, Precision>);

	template <typename T, uint8_t Precision>
	struct fixed {
		static_assert(std::is_integral_v<T>);
		static_assert(sizeof(T) * 8 > Precision);
		static constexpr uint8_t bits_of_precision = Precision;

		using value_type = T;
		using reference = std::add_lvalue_reference_t<value_type>;
		using pointer = std::add_pointer_t<value_type>;
		using const_reference = std::add_const_t<reference>;
		using const_pointer = std::add_const_t<pointer>;

	private:
		inline constexpr value_type one() { return static_cast<value_type>(1) << bits_of_precision; }
		template <typename T>
		inline static constexpr value_type to_fixed(T Value) {
			if constexpr (std::is_floating_point_v<T>) return static_cast<value_type>(std::round(Value * one()));
			else {
				static_assert(std::is_integral_v<T>);
				return static_cast<value_type>(Value << bits_of_precision);
			}
		}
		value_type _data;

	public:
		inline constexpr explicit fixed() : _data(0) {}
		template <typename T> inline constexpr explicit fixed(T Value) : _data(to_fixed(Value)) {}

		friend constexpr fixed floor<T, Precision>(fixed Value);
		friend constexpr fixed round<T, Precision>(fixed Value);
		friend constexpr fixed ceil<T, Precision>(fixed Value);
		friend constexpr fixed trunc<T, Precision>(fixed Value);

		inline constexpr explicit operator value_type() { return static_cast<value_type>(_data >> bits_of_precision); }

		inline constexpr fixed& operator++() { _data += one(); return *this; }
		inline constexpr fixed& operator++(int) { fixed old = *this; operator++(); return old; }
		inline constexpr fixed& operator--() { _data -= one(); return *this; }
		inline constexpr fixed& operator--(int) { fixed old = *this; operator--(); return old; }

		inline constexpr fixed& operator+=(fixed rhs) { _data += rhs._data; _data >>= bits_of_precision; return *this; }
		inline constexpr fixed& operator-=(fixed rhs) { _data -= rhs._data; return *this; }
		inline constexpr fixed& operator*=(fixed rhs) { _data *= rhs._data; return *this; }
		inline constexpr fixed& operator/=(fixed rhs) { _data /= rhs._data; return *this; }
		inline constexpr fixed& operator%=(fixed rhs) { _data %= rhs._data; return *this; }
		inline constexpr fixed& operator>>=(uint8_t rhs) { _data >>= rhs; return *this; }
		inline constexpr fixed& operator<<=(uint8_t rhs) { _data <<= rhs; return *this; }

		friend inline constexpr fixed operator+(fixed lhs, fixed rhs) { return lhs += rhs; }
		friend inline constexpr fixed operator-(fixed lhs, fixed rhs) { return lhs -= rhs; }
		friend inline constexpr fixed operator*(fixed lhs, fixed rhs) { return lhs *= rhs; }
		friend inline constexpr fixed operator/(fixed lhs, fixed rhs) { return lhs /= rhs; }
		friend inline constexpr fixed operator%(fixed lhs, fixed rhs) { return lhs %= rhs; }
		friend inline constexpr fixed operator>>(fixed lhs, uint8_t rhs) { return lhs >>= rhs; }
		friend inline constexpr fixed operator<<(fixed lhs, uint8_t rhs) { return lhs <<= rhs; }

		friend inline constexpr bool operator<(fixed lhs, fixed rhs) { return lhs._data < rhs._data; }
		friend inline constexpr bool operator>(fixed lhs, fixed rhs) { return lhs._data > rhs._data; }
		friend inline constexpr bool operator<=(fixed lhs, fixed rhs) { return lhs._data <= rhs._data; }
		friend inline constexpr bool operator>=(fixed lhs, fixed rhs) { return lhs._data >= rhs._data; }
		friend inline constexpr bool operator==(fixed lhs, fixed rhs) { return lhs._data == rhs._data; }
		friend inline constexpr bool operator!=(fixed lhs, fixed rhs) { return lhs._data != rhs._data; }
	};
	template <typename uint8_t Precision> using fixed8 = fixed<int8_t, Precision>;
	template <typename uint8_t Precision> using fixed16 = fixed<int16_t, Precision>;
	template <typename uint8_t Precision> using fixed32 = fixed<int32_t, Precision>;
	template <typename uint8_t Precision> using fixed64 = fixed<int64_t, Precision>;
	template <typename uint8_t Precision> using ufixed8 = fixed<uint8_t, Precision>;
	template <typename uint8_t Precision> using ufixed16 = fixed<uint16_t, Precision>;
	template <typename uint8_t Precision> using ufixed32 = fixed<uint32_t, Precision>;
	template <typename uint8_t Precision> using ufixed64 = fixed<uint64_t, Precision>;
	template <typename uint8_t Precision> using fixed_size = fixed<size_t, Precision>;

	template <typename T, uint8_t Precision> constexpr fixed<T, Precision> floor(fixed<T, Precision> Value) {
		return fixed<T, Precision>();
	}
	template <typename T, uint8_t Precision> constexpr fixed<T, Precision> ceil(fixed<T, Precision> Value) {
		return fixed<T, Precision>();
	}
	template <typename T, uint8_t Precision> constexpr fixed<T, Precision> round(fixed<T, Precision> Value) {
		T fraction = Value._data % (1 << Precision);
		return fixed<T, Precision>();
	}
	template <typename T, uint8_t Precision> constexpr fixed<T, Precision> trunc(fixed<T, Precision> Value) {
		return Value >> Precision << Precision;
	}
}
