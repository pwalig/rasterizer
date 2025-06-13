#pragma once

namespace rast {
	template <typename VertT>
	inline static VertT clip_vert(
		const VertT& v0, const VertT& v1,
		float coef0, float coef1
	) {
		float t = coef0 / (coef0 - coef1);
		return (v0 * (1.0f - t)) + (v1 * t);
	}

}
