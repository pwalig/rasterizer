#pragma once
#include "color.hpp"

namespace rast::alpha_blend {
	enum class factor : uint8_t {
		zero, one,
		src_color, one_minus_src_color,
		dst_color, one_minus_dst_color,
		src_alpha, one_minus_src_alpha,
		dst_alpha, one_minus_dst_alpha,
	};

	template <factor Factor, typename Color>
	inline constexpr Color mul_by_factor(const Color& color, const Color& src, const Color& dst) {
		if constexpr (Factor == factor::zero) return Color(static_cast<typename Color::value_type>(0));
		else if constexpr (Factor == factor::one) return color;
		else if constexpr (Factor == factor::src_color) return color * src;
		else if constexpr (Factor == factor::one_minus_src_color) return color * (Color(static_cast<typename Color::value_type>(1)) - src);
		else if constexpr (Factor == factor::dst_color) return color * dst;
		else if constexpr (Factor == factor::one_minus_dst_color) return color * (Color(static_cast<typename Color::value_type>(1)) - dst);
		else if constexpr (Factor == factor::src_alpha) return color * Color(src[4]);
		else if constexpr (Factor == factor::one_minus_src_color) return color * Color(static_cast<typename Color::value_type>(1) - src[4]);
		else if constexpr (Factor == factor::dst_color) return color * Color(dst[4]);
		else if constexpr (Factor == factor::one_minus_dst_color) return color * Color(static_cast<typename Color::value_type>(1) - dst[4]);
		else return color; // otherwise compiler complains about missing return statement
	}

	inline rast::color::rgba8 mul_by_factor_helper(const rast::color::rgba8& color, float factor) {
		return rast::color::rgba8(
			static_cast<uint8_t>(color.r * factor),
			static_cast<uint8_t>(color.g * factor),
			static_cast<uint8_t>(color.b * factor),
			static_cast<uint8_t>(color.a * factor)
		);
	}

	template <>
	inline rast::color::rgba8 mul_by_factor<factor::zero, rast::color::rgba8>(
		const rast::color::rgba8&, const rast::color::rgba8&, const rast::color::rgba8&
	) { return rast::color::rgba8(0); }
	template <>
	inline rast::color::rgba8 mul_by_factor<factor::one, rast::color::rgba8>(
		const rast::color::rgba8&, const rast::color::rgba8& src, const rast::color::rgba8&
	) { return src; }
	template <>
	inline rast::color::rgba8 mul_by_factor<factor::src_color, rast::color::rgba8>(
		const rast::color::rgba8& color, const rast::color::rgba8& src, const rast::color::rgba8&
	) { return color * src / rast::color::rgba8(255); }
	template <>
	inline rast::color::rgba8 mul_by_factor<factor::one_minus_src_color, rast::color::rgba8>(
		const rast::color::rgba8& color, const rast::color::rgba8& src, const rast::color::rgba8&
	) {
		return color * rast::color::rgba8(255 - src.r, 255 - src.g, 255 - src.b, 255 - src.a) / rast::color::rgba8(255);
	}
	template <>
	inline rast::color::rgba8 mul_by_factor<factor::dst_color, rast::color::rgba8>(
		const rast::color::rgba8& color, const rast::color::rgba8&, const rast::color::rgba8& dst
	) { return color * dst / rast::color::rgba8(255); }
	template <>
	inline rast::color::rgba8 mul_by_factor<factor::one_minus_dst_color, rast::color::rgba8>(
		const rast::color::rgba8& color, const rast::color::rgba8&, const rast::color::rgba8& dst
	) {
		return color * rast::color::rgba8(255 - dst.r, 255 - dst.g, 255 - dst.b, 255 - dst.a) / rast::color::rgba8(255);
	}
	template <>
	inline rast::color::rgba8 mul_by_factor<factor::src_alpha, rast::color::rgba8>(
		const rast::color::rgba8& color, const rast::color::rgba8& src, const rast::color::rgba8&
	) {
		float factor = static_cast<float>(src.a) / 255.0f;
		return mul_by_factor_helper(color, factor);
	}
	template <>
	inline rast::color::rgba8 mul_by_factor<factor::one_minus_src_alpha, rast::color::rgba8>(
		const rast::color::rgba8& color, const rast::color::rgba8& src, const rast::color::rgba8&
	) {
		float factor = static_cast<float>(255 - src.a) / 255.0f;
		return mul_by_factor_helper(color, factor);
	}
	template <>
	inline rast::color::rgba8 mul_by_factor<factor::dst_alpha, rast::color::rgba8>(
		const rast::color::rgba8& color, const rast::color::rgba8&, const rast::color::rgba8& dst
	) {
		float factor = static_cast<float>(dst.a) / 255.0f;
		return mul_by_factor_helper(color, factor);
	}
	template <>
	inline rast::color::rgba8 mul_by_factor<factor::one_minus_dst_alpha, rast::color::rgba8>(
		const rast::color::rgba8& color, const rast::color::rgba8&, const rast::color::rgba8& dst
	) {
		float factor = static_cast<float>(255 - dst.a) / 255.0f;
		return mul_by_factor_helper(color, factor);
	}

	enum class equation {
		add, subtract, reverse_subtract,
		min, max
	};

	namespace function {
		template <typename Color>
		using type = Color(*)(const Color&, const Color&);
	}

	template <typename Color>
	using function_t = Color(*)(const Color&, const Color&);

	template <factor SrcFactor, factor DstFactor, equation Equation>
	struct func {
		template<typename Color>
		static Color blend(const Color& src, const Color& dst) {
			auto s = mul_by_factor<SrcFactor, Color>(src, src, dst);
			auto d = mul_by_factor<DstFactor, Color>(dst, src, dst);
			if constexpr (Equation == equation::add) return s + d;
			else if constexpr (Equation == equation::subtract) return s - d;
			else if constexpr (Equation == equation::reverse_subtract) return d - s;
		}
	};

	template <typename Color, factor SrcFactor, factor DstFactor, equation Equation>
	inline constexpr function_t<Color> function_v = func<SrcFactor, DstFactor, Equation>::blend;


	template <>
	struct func<factor::one, factor::zero, equation::add> {
		template<typename Color>
		inline static Color blend(const Color& src, const Color&) {
			return src;
		}
	};

	template<typename Color>
	inline constexpr function::type<Color> replace = func<factor::one, factor::zero, equation::add>::blend;
}
