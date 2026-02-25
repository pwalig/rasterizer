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

	private:
		value_type _data[Count];

	public:
		inline constexpr _scalar() : _data() {}
		inline constexpr explicit _scalar(const_pointer Data) : _data() {
			for_count _data[i] = Data[i];
		}
		inline constexpr explicit _scalar(const_reference Value) : _data() {
			for_count _data[i] = Value;
		}
		template <typename U>
		inline constexpr explicit _scalar(_scalar<U, Count> other) : _data() {
			for_count _data[i] = U(other[i]);
		}
		inline constexpr const_reference operator[](size_type i) const { return _data[i]; }
		inline constexpr reference operator[](size_type i) { return _data[i]; }
		inline constexpr size_type size() const { return Count; }
		inline constexpr const_pointer data() const { return _data; }
		inline constexpr pointer data() { return _data; }

		template <typename U> inline constexpr _scalar& operator+=(const _scalar<U, Count>& rhs) { for_count _data[i] += rhs[i]; return *this; }
		template <typename U> inline constexpr _scalar& operator-=(const _scalar<U, Count>& rhs) { for_count _data[i] -= rhs[i]; return *this; }
		template <typename U> inline constexpr _scalar& operator*=(const _scalar<U, Count>& rhs) { for_count _data[i] *= rhs[i]; return *this; }
		template <typename U> inline constexpr _scalar& operator/=(const _scalar<U, Count>& rhs) { for_count _data[i] /= rhs[i]; return *this; }
		template <typename U> inline constexpr _scalar& operator%=(const _scalar<U, Count>& rhs) { for_count _data[i] %= rhs[i]; return *this; }
		template <typename U> inline constexpr _scalar& operator>>=(const _scalar<U, Count>& rhs) { for_count _data[i] >>= rhs[i]; return *this; }
		template <typename U> inline constexpr _scalar& operator<<=(const _scalar<U, Count>& rhs) { for_count _data[i] <<= rhs[i]; return *this; }
		template <typename U> inline constexpr _scalar& operator&=(const _scalar<U, Count>& rhs) { for_count _data[i] &= rhs[i]; return *this; }
		template <typename U> inline constexpr _scalar& operator|=(const _scalar<U, Count>& rhs) { for_count _data[i] |= rhs[i]; return *this; }
		inline constexpr _scalar operator-() { _scalar res; for_count res[i] = -(data[i]); return res; }
		template <typename U> friend inline constexpr _scalar operator+(const _scalar& lhs, const _scalar<U, Count>& rhs) { _scalar tmp = lhs; tmp += rhs; return tmp; }
		template <typename U> friend inline constexpr _scalar operator-(const _scalar& lhs, const _scalar<U, Count>& rhs) { _scalar tmp = lhs; tmp -= rhs; return tmp; }
		template <typename U> friend inline constexpr _scalar operator*(const _scalar& lhs, const _scalar<U, Count>& rhs) { _scalar tmp = lhs; tmp *= rhs; return tmp; }
		template <typename U> friend inline constexpr _scalar operator/(const _scalar& lhs, const _scalar<U, Count>& rhs) { _scalar tmp = lhs; tmp /= rhs; return tmp; }
		template <typename U> friend inline constexpr _scalar operator%(const _scalar& lhs, const _scalar<U, Count>& rhs) { _scalar tmp = lhs; tmp %= rhs; return tmp; }
		template <typename U> friend inline constexpr _scalar operator>>(const _scalar& lhs, const _scalar<U, Count>& rhs) { _scalar tmp = lhs; tmp >>= rhs; return tmp; }
		template <typename U> friend inline constexpr _scalar operator<<(const _scalar& lhs, const _scalar<U, Count>& rhs) { _scalar tmp = lhs; tmp <<= rhs; return tmp; }
		template <typename U> friend inline constexpr _scalar operator&(const _scalar& lhs, const _scalar<U, Count>& rhs) { _scalar tmp = lhs; tmp &= rhs; return tmp; }
		template <typename U> friend inline constexpr _scalar operator|(const _scalar& lhs, const _scalar<U, Count>& rhs) { _scalar tmp = lhs; tmp |= rhs; return tmp; }
		template <typename U> friend inline constexpr _scalar<bool, Count> operator==(const _scalar& lhs, const _scalar<U, Count>& rhs) { _scalar<bool, Count> res; for_count res[i] = (lhs[i] == rhs[i]); return res; }
		template <typename U> friend inline constexpr _scalar<bool, Count> operator!=(const _scalar& lhs, const _scalar<U, Count>& rhs) { _scalar<bool, Count> res; for_count res[i] = (lhs[i] != rhs[i]); return res; }
		template <typename U> friend inline constexpr _scalar<bool, Count> operator<=(const _scalar& lhs, const _scalar<U, Count>& rhs) { _scalar<bool, Count> res; for_count res[i] = (lhs[i] <= rhs[i]); return res; }
		template <typename U> friend inline constexpr _scalar<bool, Count> operator>=(const _scalar& lhs, const _scalar<U, Count>& rhs) { _scalar<bool, Count> res; for_count res[i] = (lhs[i] >= rhs[i]); return res; }
		template <typename U> friend inline constexpr _scalar<bool, Count> operator<(const _scalar& lhs, const _scalar<U, Count>& rhs) { _scalar<bool, Count> res; for_count res[i] = (lhs[i] < rhs[i]); return res; }
		template <typename U> friend inline constexpr _scalar<bool, Count> operator>(const _scalar& lhs, const _scalar<U, Count>& rhs) { _scalar<bool, Count> res; for_count res[i] = (lhs[i] > rhs[i]); return res; }

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
	};

	template <size_t Count, typename T>
	inline constexpr _scalar<T, Count> vectorize(T Value) {
		return _scalar<T, Count>(std::move(Value));
	}

	template <typename T, typename U, size_t Count>
	inline constexpr _scalar<T, Count> load(T* Address, _scalar<U, Count> Offsets) {
		auto res = _scalar<T, Count>();
		for_count res[i] = Address[Offsets[i]];
		return res;
	}
	template <typename T, size_t Count>
	inline constexpr _scalar<T, Count> load(_scalar<T*, Count> Address) {
		auto res = _scalar<T, Count>();
		for_count res[i] = *(Address[i]);
		return res;
	}
	template <typename T, size_t Count>
	inline constexpr void store(_scalar<T*, Count> Address, const _scalar<T, Count>& Value) {
		for_count *(Address[i]) = Value[i];
	}
	template <typename T, typename U, size_t Count>
	inline constexpr void store(T* Address, const _scalar<U, Count> Offsets, const _scalar<T, Count>& Value) {
		for_count Address[Offsets[i]] = Value[i];
	}

	template <typename T>
	inline constexpr _scalar<T, 1> make_x1(T a0) {
		auto res = _scalar<T, 1>();
		res[0] = std::move(a0);
		return res;
	}
	template <typename T>
	inline constexpr _scalar<T, 4> make_x4(
		T a0, T a1, T a2, T a3
	) {
		auto res = _scalar<T, 4>();
		res[0] = std::move(a0);
		res[1] = std::move(a1);
		res[2] = std::move(a2);
		res[3] = std::move(a3);
		return res;
	}

	template <typename U, typename T, size_t Count>
	inline constexpr _scalar<U, Count> floor(
		const _scalar<T, Count>& Value
	) {
		auto res = _scalar<U, Count>();
		for_count res[i] = math::floor<U>(Value[i]);
		return res;
	}

	template <typename U, typename T, size_t Count>
	inline constexpr _scalar<U, Count> ceil(
		const _scalar<T, Count>& Value
	) {
		auto res = _scalar<U, Count>();
		for_count res[i] = math::ceil<U>(Value[i]);
		return res;
	}

	template <typename U, typename T, size_t Count>
	inline constexpr _scalar<U, Count> round(
		const _scalar<T, Count>& Value
	) {
		auto res = _scalar<U, Count>();
		for_count res[i] = math::round<U>(Value[i]);
		return res;
	}

	template <typename T, size_t Count>
	inline constexpr _scalar<T, Count> sqrt(
		const _scalar<T, Count>& Value
	) {
		auto res = _scalar<T, Count>();
		for_count res[i] = math::sqrt(Value[i]);
		return res;
	}

	template <typename T, size_t Count>
	inline constexpr _scalar<T, Count> min(
		const _scalar<T, Count>& a,
		const _scalar<T, Count>& b
	) {
		auto res = _scalar<T, Count>();
		for_count res[i] = std::min(a[i], b[i]);
		return res;
	}

	template <typename T, size_t Count>
	inline constexpr _scalar<T, Count> max(
		const _scalar<T, Count>& a,
		const _scalar<T, Count>& b
	) {
		auto res = _scalar<T, Count>();
		for_count res[i] = std::max(a[i], b[i]);
		return res;
	}

	template <typename T, size_t Count>
	inline constexpr _scalar<T, Count> clamp(
		const _scalar<T, Count>& Value,
		const _scalar<T, Count>& Min,
		const _scalar<T, Count>& Max
	) {
		auto res = _scalar<T, Count>();
		for_count res[i] = std::clamp(Value[i], Min[i], Max[i]);
		return res;
	}

	template <typename U, typename T, size_t Count>
	inline constexpr _scalar<U, Count> cast(const _scalar<T, Count>& Value) {
		auto res = _scalar<U, Count>();
		for_count res[i] = static_cast<U>(Value[i]);
		return res;
	}

	template <typename T> using x1 = _scalar<T, 1>;
	template <typename T> using x4 = _scalar<T, 4>;
	template <typename T> using x8 = _scalar<T, 8>;
	template <typename T> using x16 = _scalar<T, 16>;

	template <size_t Count> using boolx = _scalar<bool, Count>;
	template <size_t Count> using i8x = _scalar<int8_t, Count>;
	template <size_t Count> using u8x = _scalar<uint8_t, Count>;
	template <size_t Count> using i16x = _scalar<int16_t, Count>;
	template <size_t Count> using u16x = _scalar<uint16_t, Count>;
	template <size_t Count> using i32x = _scalar<int32_t, Count>;
	template <size_t Count> using u32x = _scalar<uint32_t, Count>;
	template <size_t Count> using i64x = _scalar<int64_t, Count>;
	template <size_t Count> using u64x = _scalar<uint64_t, Count>;
	template <size_t Count> using f32x = _scalar<float, Count>;
	template <size_t Count> using f64x = _scalar<double, Count>;

	template <typename T, typename U, size_t Count>
	inline constexpr void store(
		T* Address, const _scalar<U, Count> Offsets,
		const _scalar<T, Count>& Value, const boolx<Count>& Mask
	) {
		for_count if (Mask[i]) Address[Offsets[i]] = Value[i];
	}

	using i8x1 = _scalar<int8_t, 1>;
	using u8x1 = _scalar<uint8_t, 1>;
	using i16x1 = _scalar<int16_t, 1>;
	using u16x1 = _scalar<uint16_t, 1>;
	using i32x1 = _scalar<int32_t, 1>;
	using u32x1 = _scalar<uint32_t, 1>;
	using i64x1 = _scalar<int64_t, 1>;
	using u64x1 = _scalar<uint64_t, 1>;
	using f32x1 = _scalar<float, 1>;
	using f64x1 = _scalar<double, 1>;

	using i8x4 = _scalar<int8_t, 4>;
	using u8x4 = _scalar<uint8_t, 4>;
	using i16x4 = _scalar<int16_t, 4>;
	using u16x4 = _scalar<uint16_t, 4>;
	using i32x4 = _scalar<int32_t, 4>;
	using u32x4 = _scalar<uint32_t, 4>;
	using i64x4 = _scalar<int64_t, 4>;
	using u64x4 = _scalar<uint64_t, 4>;
	using f32x4 = _scalar<float, 4>;
	using f64x4 = _scalar<double, 4>;

	inline constexpr u32x4 make_u32x4(
		uint32_t a, uint32_t b, uint32_t c, uint32_t d
	) {
		return make_x4<uint32_t>(a, b, c, d);
	}
	inline constexpr f32x4 make_f32x4(
		float a, float b, float c, float d
	) {
		return make_x4<float>(a, b, c, d);
	}

	template <size_t Count>
	inline constexpr bool and_accross(boolx<Count> Val) noexcept {
		bool res = true;
		for_count res &= Val[i];
		return res;
	}
	template <size_t Count>
	inline constexpr bool or_accross(boolx<Count> Val) noexcept {
		bool res = false;
		for_count res |= Val[i];
		return res;
	}

	namespace _scalar_test {
		constexpr float data[4] = { 0.0f, 1.0f, 2.0f, 3.0f };
		constexpr auto scal = f32x4(data);
		static_assert(scal[0] == 0.0f);
		static_assert(scal[1] == 1.0f);
		static_assert(scal[2] == 2.0f);
		static_assert(scal[3] == 3.0f);
		constexpr auto scal2 = f32x4(4.0f);
		static_assert(scal2[0] == 4.0f);
		static_assert(scal2[1] == 4.0f);
		static_assert(scal2[2] == 4.0f);
		static_assert(scal2[3] == 4.0f);
		constexpr auto scal3 = scal + scal2;
		static_assert(scal3[0] == 4.0f);
		static_assert(scal3[1] == 5.0f);
		static_assert(scal3[2] == 6.0f);
		static_assert(scal3[3] == 7.0f);
		constexpr auto scal4 = scal * scal2;
		static_assert(scal4[0] == 0.0f);
		static_assert(scal4[1] == 4.0f);
		static_assert(scal4[2] == 8.0f);
		static_assert(scal4[3] == 12.0f);

		constexpr float data2[4] = { 4.0f, 12.25f, 16.0f, 25.0f };
		constexpr auto sqrtres = sqrt(make_x4<float>(4.0f, 12.25f, 16.0f, 25.0f));
		static_assert(sqrtres[0] == 2.0f);
		static_assert(sqrtres[1] == 3.5f);
		static_assert(sqrtres[2] == 4.0f);
		static_assert(sqrtres[3] == 5.0f);
	}

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

		inline constexpr typename value_type::const_reference x(size_type i) const { return x()[i]; }
		inline constexpr typename value_type::reference x(size_type i) { return x()[i]; }
		inline constexpr typename value_type::const_reference y(size_type i) const { return y()[i]; }
		inline constexpr typename value_type::reference y(size_type i) { return y()[i]; }
		inline constexpr typename value_type::const_reference z(size_type i) const { return z()[i]; }
		inline constexpr typename value_type::reference z(size_type i) { return z()[i]; }
		inline constexpr typename value_type::const_reference w(size_type i) const { return w()[i]; }
		inline constexpr typename value_type::reference w(size_type i) { return w()[i]; }

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
		inline constexpr _vec operator-() { _vec res; for_dim res[j] = -(data[j]); return res; }
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
			return sqrt(dot(*this, *this));
		}
		inline constexpr _vec& normalize() {
			return *this /= length();
		}
		inline constexpr _vec normalized() const {
			return *this / length();
		}
	};

	template <size_t Count, typename T, size_t Dim>
	inline constexpr _vec<T, Dim, Count> vectorize(_vec<T, Dim, 1> Value) {
		auto res = _vec<T, Dim, Count>();
		for_dim res[j] = vectorize<Count>(Value[j][0]);
		return res;
	}

	template <typename T, size_t Dim, size_t Count>
	inline constexpr _scalar<T, Count> dot(const _vec<T, Dim, Count>& a, const _vec<T, Dim, Count>& b) {
		auto res = _scalar<T, Count>(static_cast<T>(0));
		for_count for_dim res[i] += a[j][i] * b[j][i];
		return res;
	}

	template <typename U, typename T, size_t Dim, size_t Count>
	inline constexpr _vec<U, Dim, Count> cast(const _vec<T, Dim, Count>& Value) {
		auto res = _vec<U, Dim, Count>();
		for_dim res[j] = cast<U>(Value[j]);
		return res;
	}

	template <typename T, typename U, size_t Dim, size_t Count>
	inline constexpr _vec<T, Dim, Count> load(_vec<T, Dim, 1>* Address, _scalar<U, Count> Offsets) {
		auto res = _vec<T, Dim, Count>();
		for_count for_dim res[j][i] = Address[Offsets[i]][j][0];
		return res;
	}
	template <typename T, size_t Dim, size_t Count>
	inline constexpr void store(_scalar<_vec<T, Dim, 1>*, Count> Address, const _vec<T, Dim, Count>& Value) {
		for_count for_dim (*(Address[i]))[j][0] = Value[j][i];
	}
	template <typename T, typename U, size_t Dim, size_t Count>
	inline constexpr void store(_vec<T, Dim, 1>* Address, const _scalar<U, Count>& offsets, const _vec<T, Dim, Count>& Value) {
		for_count for_dim Address[offsets[i]][j][0] = Value[j][i];
	}
	template <typename T, typename U, size_t Dim, size_t Count>
	inline constexpr void store(
		_vec<T, Dim, 1>* Address, const _scalar<U, Count>& Offsets,
		const _vec<T, Dim, Count>& Value, const boolx<Count>& Mask
	) {
		for_count if(Mask[i]) for_dim Address[Offsets[i]][j][0] = Value[j][i];
	}

	template <typename T, size_t Count> using vec1x = _vec<T, 1, Count>;
	template <typename T, size_t Count> using vec2x = _vec<T, 2, Count>;
	template <typename T, size_t Count> using vec3x = _vec<T, 3, Count>;
	template <typename T, size_t Count> using vec4x = _vec<T, 4, Count>;

	template <typename T> using vec1x1 = vec1x<T, 1>;
	template <typename T> using vec2x1 = vec2x<T, 1>;
	template <typename T> using vec3x1 = vec3x<T, 1>;
	template <typename T> using vec4x1 = vec4x<T, 1>;

	template <typename T> using vec1x4 = vec1x<T, 4>;
	template <typename T> using vec2x4 = vec2x<T, 4>;
	template <typename T> using vec3x4 = vec3x<T, 4>;
	template <typename T> using vec4x4 = vec4x<T, 4>;

	template <typename T> using vec1x8 = vec1x<T, 8>;
	template <typename T> using vec2x8 = vec2x<T, 8>;
	template <typename T> using vec3x8 = vec3x<T, 8>;
	template <typename T> using vec4x8 = vec4x<T, 8>;

	template <size_t Count> using u8vec1x = vec1x<uint8_t, Count>;
	template <size_t Count> using u8vec2x = vec2x<uint8_t, Count>;
	template <size_t Count> using u8vec3x = vec3x<uint8_t, Count>;
	template <size_t Count> using u8vec4x = vec4x<uint8_t, Count>;

	template <size_t Count> using i32vec1x = vec1x<int32_t, Count>;
	template <size_t Count> using i32vec2x = vec2x<int32_t, Count>;
	template <size_t Count> using i32vec3x = vec3x<int32_t, Count>;
	template <size_t Count> using i32vec4x = vec4x<int32_t, Count>;

	template <size_t Count> using u32vec1x = vec1x<uint32_t, Count>;
	template <size_t Count> using u32vec2x = vec2x<uint32_t, Count>;
	template <size_t Count> using u32vec3x = vec3x<uint32_t, Count>;
	template <size_t Count> using u32vec4x = vec4x<uint32_t, Count>;

	template <size_t Count> using f32vec1x = vec1x<float, Count>;
	template <size_t Count> using f32vec2x = vec2x<float, Count>;
	template <size_t Count> using f32vec3x = vec3x<float, Count>;
	template <size_t Count> using f32vec4x = vec4x<float, Count>;

	using u8vec1x1 = vec1x1<uint8_t>;
	using u8vec2x1 = vec2x1<uint8_t>;
	using u8vec3x1 = vec3x1<uint8_t>;
	using u8vec4x1 = vec4x1<uint8_t>;

	using i32vec1x4 = vec1x4<int32_t>;
	using i32vec2x4 = vec2x4<int32_t>;
	using i32vec3x4 = vec3x4<int32_t>;
	using i32vec4x4 = vec4x4<int32_t>;

	using f32vec1x1 = vec1x1<float>;
	using f32vec2x1 = vec2x1<float>;
	using f32vec3x1 = vec3x1<float>;
	using f32vec4x1 = vec4x1<float>;

	using f32vec1x4 = vec1x4<float>;
	using f32vec2x4 = vec2x4<float>;
	using f32vec3x4 = vec3x4<float>;
	using f32vec4x4 = vec4x4<float>;

	template <typename T, size_t Dim, size_t Count>
	inline constexpr _vec<T, Count, Dim> transpose(_vec<T, Dim, Count> val) {
		auto res = _vec<T, Count, Dim>();
		for_count for_dim res[i][j] = val[j][i];
		return res;
	}

	template <size_t Dim, typename T, size_t Count>
	inline constexpr _vec<typename T::value_type, Dim, Count> transpose(_scalar<T, Count> val) {
		auto res = _vec<typename T::value_type, Dim, Count>();
#if _MSC_VER && !__INTEL_COMPILER
#pragma warning( push )
#pragma warning( disable : 4267 )
#endif
		for_count for_dim res[j][i] = val[i][j];
#if _MSC_VER && !__INTEL_COMPILER
#pragma warning( pop )
#endif
		return res;
	}

	template <typename U, typename T, size_t Dim, size_t Count>
	inline constexpr _scalar<U, Count> transpose(_vec<T, Dim, Count> val) {
		auto res = _scalar<U, Count>();
		for_count for_dim res[i][j] = val[j][i];
		return res;
	}

	namespace vec_test {
		static_assert(alignof(f32vec2x4) == 16);
		constexpr float data[8] = {
			0.0f, 1.0f, 2.0f, 3.0f,
			4.0f, 5.0f, 6.0f, 7.0f
		};
		constexpr auto vect = f32vec2x4(data);
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

		constexpr float data3[4] = { 3.0f, 6.0f, 4.0f, 8.0f };
		constexpr auto len3 = f32vec2x<2>(data3).length();
		static_assert(len3[0] == 5.0f);
		static_assert(len3[1] == 10.0f);
		static_assert(vec2x<float, 2>(data3).normalized().length()[0] == 1.0f);
	}

#undef for_count
#undef for_dim
}
