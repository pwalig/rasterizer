#pragma once
#include <algorithm>
#include "../math.hpp"

#define rast_math_for for(size_type i = 0; i < Count; ++i)

#define for_count for(size_t i = 0; i < Count; ++i)
#define for_dim for(size_t j = 0; j < Dim; ++j)

namespace rast::math {
	template <typename T, size_t Count>
	struct alignas(Count * sizeof(T)) _scalar {
		using value_type = T;
		using reference = std::add_lvalue_reference_t<value_type>;
		using pointer = std::add_pointer_t<value_type>;
		using const_reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;
		using const_pointer = std::add_pointer_t<std::add_const_t<value_type>>;
		using size_type = size_t;

		value_type data[Count];

		inline constexpr _scalar() : data() {}
		inline constexpr explicit _scalar(const_pointer Data) : data() {
			for_count data[i] = Data[i];
		}
		inline constexpr explicit _scalar(const_reference Value) : data() {
			for_count data[i] = Value;
		}
		inline constexpr const_reference operator[](size_type i) const { return data[i]; }
		inline constexpr reference operator[](size_type i) { return data[i]; }

		inline constexpr _scalar& operator+=(const _scalar& rhs) { for_count data[i] += rhs[i]; return *this; }
		inline constexpr _scalar& operator-=(const _scalar& rhs) { for_count data[i] -= rhs[i]; return *this; }
		inline constexpr _scalar& operator*=(const _scalar& rhs) { for_count data[i] *= rhs[i]; return *this; }
		inline constexpr _scalar& operator/=(const _scalar& rhs) { for_count data[i] /= rhs[i]; return *this; }
		inline constexpr _scalar& operator%=(const _scalar& rhs) { for_count data[i] %= rhs[i]; return *this; }
		friend inline constexpr _scalar operator+(const _scalar& lhs, const _scalar& rhs) { _scalar tmp = lhs; tmp += rhs; return tmp; }
		friend inline constexpr _scalar operator-(const _scalar& lhs, const _scalar& rhs) { _scalar tmp = lhs; tmp -= rhs; return tmp; }
		friend inline constexpr _scalar operator*(const _scalar& lhs, const _scalar& rhs) { _scalar tmp = lhs; tmp *= rhs; return tmp; }
		friend inline constexpr _scalar operator/(const _scalar& lhs, const _scalar& rhs) { _scalar tmp = lhs; tmp /= rhs; return tmp; }
		friend inline constexpr _scalar operator%(const _scalar& lhs, const _scalar& rhs) { _scalar tmp = lhs; tmp %= rhs; return tmp; }

		friend inline constexpr _scalar<bool, Count> operator==(const _scalar& lhs, const _scalar& rhs) {
			auto res = _scalar<bool, Count>();
			for_count res[i] = (lhs[i] = rhs[i]);
			return res;
		}
		friend inline constexpr _scalar<bool, Count> operator!=(const _scalar& lhs, const _scalar& rhs) {
			auto res = _scalar<bool, Count>();
			for_count res[i] = (lhs[i] != rhs[i]);
			return res;
		}

		inline constexpr _scalar sqrt() const {
			auto res = _scalar();
			for_count res[i] = math::sqrt(data[i]);
			return res;
		}
		inline constexpr _scalar& clamp(const _scalar& min, const _scalar& max) {
			for_count data[i] = std::clamp(data[i], min[i], max[i]);
			return *this;
		}
		inline constexpr _scalar clamped(const _scalar& min, const _scalar& max) const {
			auto res = _scalar();
			for_count res[i] = std::clamp(data[i], min[i], max[i]);
			return res;
		}
	};

	template <typename T, size_t Count>
	inline constexpr _scalar<T, Count> clamp(
		const _scalar<T, Count>& Value,
		const _scalar<T, Count>& Min,
		const _scalar<T, Count>& Max
	) {
		return Value.clamped(Min, Max);
	}

	template <typename U, typename T, size_t Count>
	inline constexpr _scalar<U, Count> cast(const _scalar<T, Count>& Value) {
		auto res = _scalar<U, Count>();
		for_count res[i] = static_cast<U>(Value[i]);
		return res;
	}

	template <typename T> using _4scalar = _scalar<T, 4>;
	template <typename T> using _8scalar = _scalar<T, 8>;

	using _4fscalar = _4scalar<float>;
	using _4iscalar = _4scalar<int>;

	template <typename T, size_t Dim, size_t Count>
	struct alignas(Count * sizeof(T)) _vec {
		using value_type = _scalar<T, Count>;
		using reference = std::add_lvalue_reference_t<value_type>;
		using pointer = std::add_pointer_t<value_type>;
		using const_reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;
		using const_pointer = std::add_pointer_t<std::add_const_t<value_type>>;
		using size_type = size_t;

		value_type data[Dim];

		inline constexpr _vec() : data() {}
		inline constexpr explicit _vec(const T* Data) : data() {
			for_dim data[j] = value_type(Data + (j * Count));
		}
		inline constexpr explicit _vec(const T& Value) : data() {
			for_dim data[j] = value_type(Value);
		}
		inline constexpr explicit _vec(
			value_type X
		) : data() {
			x() = std::move(X);
		}
		inline constexpr explicit _vec(
			value_type X, value_type Y
		) : data() {
			x() = std::move(X);
			y() = std::move(Y);
		}
		inline constexpr explicit _vec(
			value_type X, value_type Y, value_type Z
		) : data() {
			x() = std::move(X);
			y() = std::move(Y);
			z() = std::move(Z);
		}
		inline constexpr explicit _vec(
			value_type X, value_type Y, value_type Z, value_type W
		) : data() {
			x() = std::move(X);
			y() = std::move(Y);
			z() = std::move(Z);
			w() = std::move(W);
		}

		inline constexpr explicit _vec(
			typename value_type::const_reference X,
			typename value_type::const_reference Y
		) : _vec(value_type(X), value_type(Y)) { }
		inline constexpr explicit _vec(
			typename value_type::const_reference X,
			typename value_type::const_reference Y,
			typename value_type::const_reference Z
		) : _vec(value_type(X), value_type(Y), value_type(Z)) { }
		inline constexpr explicit _vec(
			typename value_type::const_reference X,
			typename value_type::const_reference Y,
			typename value_type::const_reference Z,
			typename value_type::const_reference W
		) : _vec(value_type(X), value_type(Y), value_type(Z), value_type(W)) { }

		inline constexpr const_reference operator[](size_type i) const { return data[i]; }
		inline constexpr reference operator[](size_type i) { return data[i]; }
		inline constexpr const_reference x() const { return data[0]; }
		inline constexpr reference x() { return data[0]; }
		inline constexpr const_reference y() const { return data[1]; }
		inline constexpr reference y() { return data[1]; }
		inline constexpr const_reference z() const { return data[2]; }
		inline constexpr reference z() { return data[2]; }
		inline constexpr const_reference w() const { return data[3]; }
		inline constexpr reference w() { return data[3]; }
		inline constexpr const_reference r() const { return data[0]; }
		inline constexpr reference r() { return data[0]; }
		inline constexpr const_reference g() const { return data[1]; }
		inline constexpr reference g() { return data[1]; }
		inline constexpr const_reference b() const { return data[2]; }
		inline constexpr reference b() { return data[2]; }
		inline constexpr const_reference a() const { return data[3]; }
		inline constexpr reference a() { return data[3]; }

		inline constexpr _vec& operator+=(const _vec& rhs) { for_dim data[j] += rhs[j]; return *this; }
		inline constexpr _vec& operator-=(const _vec& rhs) { for_dim data[j] -= rhs[j]; return *this; }
		inline constexpr _vec& operator*=(const _vec& rhs) { for_dim data[j] *= rhs[j]; return *this; }
		inline constexpr _vec& operator/=(const _vec& rhs) { for_dim data[j] /= rhs[j]; return *this; }
		inline constexpr _vec& operator%=(const _vec& rhs) { for_dim data[j] %= rhs[j]; return *this; }
		inline constexpr _vec& operator+=(const value_type& rhs) { for_dim data[j] += rhs; return *this; }
		inline constexpr _vec& operator-=(const value_type& rhs) { for_dim data[j] -= rhs; return *this; }
		inline constexpr _vec& operator*=(const value_type& rhs) { for_dim data[j] *= rhs; return *this; }
		inline constexpr _vec& operator/=(const value_type& rhs) { for_dim data[j] /= rhs; return *this; }
		inline constexpr _vec& operator%=(const value_type& rhs) { for_dim data[j] %= rhs; return *this; }
		friend inline constexpr _vec operator+(const _vec& lhs, const _vec& rhs) { _vec tmp = lhs; tmp += rhs; return tmp; }
		friend inline constexpr _vec operator-(const _vec& lhs, const _vec& rhs) { _vec tmp = lhs; tmp -= rhs; return tmp; }
		friend inline constexpr _vec operator*(const _vec& lhs, const _vec& rhs) { _vec tmp = lhs; tmp *= rhs; return tmp; }
		friend inline constexpr _vec operator/(const _vec& lhs, const _vec& rhs) { _vec tmp = lhs; tmp /= rhs; return tmp; }
		friend inline constexpr _vec operator%(const _vec& lhs, const _vec& rhs) { _vec tmp = lhs; tmp %= rhs; return tmp; }
		friend inline constexpr _vec operator+(const _vec& lhs, const value_type& rhs) { _vec tmp = lhs; tmp += rhs; return tmp; }
		friend inline constexpr _vec operator-(const _vec& lhs, const value_type& rhs) { _vec tmp = lhs; tmp -= rhs; return tmp; }
		friend inline constexpr _vec operator*(const _vec& lhs, const value_type& rhs) { _vec tmp = lhs; tmp *= rhs; return tmp; }
		friend inline constexpr _vec operator/(const _vec& lhs, const value_type& rhs) { _vec tmp = lhs; tmp /= rhs; return tmp; }
		friend inline constexpr _vec operator%(const _vec& lhs, const value_type& rhs) { _vec tmp = lhs; tmp %= rhs; return tmp; }

		friend inline constexpr _scalar<bool, Count> operator==(const _vec& lhs, const _vec& rhs) {
			auto res = _scalar<bool, Count>(true);
			for_dim for_count if (lhs[j][i] != rhs[j][i]) res[i] = false;
			return res;
		}
		friend inline constexpr _vec<bool, 1, Count> operator!=(const _vec& lhs, const _vec& rhs) {
			auto res = _scalar<bool, Count>(false);
			for_dim for_count if (lhs[j][i] != rhs[j][i]) res[i] = true;
			return res;
		}

		inline constexpr _scalar<T, Count> length() const {
			return dot(*this, *this).sqrt();
		}
		inline constexpr _vec& normalize() {
			return *this /= length();
		}
		inline constexpr _vec normalized() const {
			return *this / length();
		}
	};

	template <typename T, size_t Dim, size_t Count>
	inline constexpr _scalar<T, Count> dot(const _vec<T, Dim, Count>& a, const _vec<T, Dim, Count>& b) {
		auto res = _scalar<T, Count>(static_cast<T>(0));
		for_count for_dim res[i] += a[j][i] * b[j][i];
		return res;
	}

	template <typename T, size_t Count> using _vec1 = _vec<T, 1, Count>;
	template <typename T, size_t Count> using _vec2 = _vec<T, 2, Count>;
	template <typename T, size_t Count> using _vec3 = _vec<T, 3, Count>;
	template <typename T, size_t Count> using _vec4 = _vec<T, 4, Count>;

	template <typename T, size_t Count>
	inline constexpr _vec1<T, Count> sqrt(const _vec1<T, Count>& x) {
		auto res = _vec1<T, Count>();
		for_count res[0][i] = sqrt(x[0][i]);
		return res;
	}
	template <typename T, size_t Count>
	inline constexpr _scalar<T, Count> sqrt(const _scalar<T, Count>& x) {
		return x.sqrt();
	}

	template <typename T> using _4vec1 = _vec1<T, 4>;
	template <typename T> using _4vec2 = _vec2<T, 4>;
	template <typename T> using _4vec3 = _vec3<T, 4>;
	template <typename T> using _4vec4 = _vec4<T, 4>;

	using _4ivec1 = _4vec1<int>;
	using _4ivec2 = _4vec2<int>;
	using _4ivec3 = _4vec3<int>;
	using _4ivec4 = _4vec4<int>;

	using _4fvec1 = _4vec1<float>;
	using _4fvec2 = _4vec2<float>;
	using _4fvec3 = _4vec3<float>;
	using _4fvec4 = _4vec4<float>;

	namespace vec_test {
		static_assert(alignof(_4fvec2) == 16);
		constexpr float data[8] = {
			0.0f, 1.0f, 2.0f, 3.0f,
			4.0f, 5.0f, 6.0f, 7.0f
		};
		constexpr auto scal = _4fscalar(data);
		static_assert(scal[0] == 0.0f);
		static_assert(scal[1] == 1.0f);
		static_assert(scal[2] == 2.0f);
		static_assert(scal[3] == 3.0f);
		constexpr auto vect = _4fvec2(data);
		static_assert(vect[0][0] == 0.0f);
		static_assert(vect[0][1] == 1.0f);
		static_assert(vect[0][2] == 2.0f);
		static_assert(vect[0][3] == 3.0f);
		static_assert(vect[1][0] == 4.0f);
		static_assert(vect[1][1] == 5.0f);
		static_assert(vect[1][2] == 6.0f);
		static_assert(vect[1][3] == 7.0f);
		static_assert(vect.x()[0] == 0.0f);
		static_assert(vect.x()[1] == 1.0f);
		static_assert(vect.x()[2] == 2.0f);
		static_assert(vect.x()[3] == 3.0f);
		static_assert(vect.y()[0] == 4.0f);
		static_assert(vect.y()[1] == 5.0f);
		static_assert(vect.y()[2] == 6.0f);
		static_assert(vect.y()[3] == 7.0f);

		constexpr float data2[4] = { 4.0f, 12.25f, 16.0f, 25.0f };
		constexpr auto sqrtres = sqrt(_4fvec1(data2));
		static_assert(sqrtres[0][0] == 2.0f);
		static_assert(sqrtres[0][1] == 3.5f);
		static_assert(sqrtres[0][2] == 4.0f);
		static_assert(sqrtres[0][3] == 5.0f);

		constexpr float data3[4] = { 3.0f, 6.0f, 4.0f, 8.0f };
		constexpr auto len3 = _vec2<float, 2>(data3).length();
		static_assert(len3[0] == 5.0f);
		static_assert(len3[1] == 10.0f);
		static_assert(_vec2<float, 2>(data3).normalized().length()[0] == 1.0f);
	}

#undef for_count
#undef for_dim
}
