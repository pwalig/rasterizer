#pragma once
namespace rast {
	enum struct cull {
		none, clockwise, counter_clockwise, both
	};
	inline constexpr cull cull_default = cull::counter_clockwise;
}
