#pragma once
#include "color.hpp"

namespace rast::alpha_blend {
	struct func_params {
		float src_factor;
		float dest_factor;
	};

	enum class factor : uint8_t {
		zero, one,
		src_color, one_minus_src_color,
		dst_color, one_minus_dst_color,
		src_alpha, one_minus_src_alpha,
		dst_alpha, one_minus_dst_alpha,
	};

	template <factor f, typename Color>
	Color mul_by_factor(const Color& color, const Color& src, const Color& dst);
	template <>
	inline rast::color::rgba8 mul_by_factor<factor::zero, rast::color::rgba8>(
		const rast::color::rgba8& color, const rast::color::rgba8& src, const rast::color::rgba8& dst
	) {
		return rast::color::rgba8(0);
	}
	template <>
	inline rast::color::rgba8 mul_by_factor<factor::one, rast::color::rgba8>(
		const rast::color::rgba8& color, const rast::color::rgba8& src, const rast::color::rgba8& dst
	) {
		return src;
	}
	template <>
	inline rast::color::rgba8 mul_by_factor<factor::src_alpha, rast::color::rgba8>(
		const rast::color::rgba8& color, const rast::color::rgba8& src, const rast::color::rgba8& dst
	) {
		float factor = static_cast<float>(src.a) / 255.0f;
		return rast::color::rgba8(
			static_cast<uint8_t>(color.r * factor),
			static_cast<uint8_t>(color.g * factor),
			static_cast<uint8_t>(color.b * factor),
			static_cast<uint8_t>(color.a * factor)
		);
	}
	template <>
	inline rast::color::rgba8 mul_by_factor<factor::one_minus_src_alpha, rast::color::rgba8>(
		const rast::color::rgba8& color, const rast::color::rgba8& src, const rast::color::rgba8& dst
	) {
		float factor = static_cast<float>(255 - src.a) / 255.0f;
		return rast::color::rgba8(
			static_cast<uint8_t>(color.r * factor),
			static_cast<uint8_t>(color.g * factor),
			static_cast<uint8_t>(color.b * factor),
			static_cast<uint8_t>(color.a * factor)
		);
	}

	enum class equation {
		add, subtract, reverse_subtract,
		min, max
	};

	template <factor, factor, equation>
	struct func {
		template<typename Color>
		static Color blend(const Color& src, const Color& dst);
	};

	template <factor sfactor, factor dfactor>
	struct func<sfactor, dfactor, equation::add> {
		template<typename Color>
		inline static Color blend(const Color& src, const Color& dst) {
			return mul_by_factor<sfactor, Color>(src, src, dst) + mul_by_factor<dfactor, Color>(dst, src, dst);
		}
	};
	template <factor sfactor, factor dfactor>
	struct func<sfactor, dfactor, equation::subtract> {
		template<typename Color>
		inline static Color blend(const Color& src, const Color& dst) {
			return mul_by_factor<sfactor, Color>(src, src, dst) - mul_by_factor<dfactor, Color>(dst, src, dst);
		}
	};
	template <factor sfactor, factor dfactor>
	struct func<sfactor, dfactor, equation::reverse_subtract> {
		template<typename Color>
		inline static Color blend(const Color& src, const Color& dst) {
			return mul_by_factor<sfactor, Color>(dst, src, dst) - mul_by_factor<dfactor, Color>(src, src, dst);
		}
	};
}
