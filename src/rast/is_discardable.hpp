#pragma once

namespace rast {
	template <typename fragment_output>
	struct is_discardable {
		inline static constexpr bool value = false;
	};

	template <typename fragment_output>
	inline constexpr bool is_discardable_v = is_discardable<fragment_output>::value;
}

