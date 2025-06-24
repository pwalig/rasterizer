#pragma once

namespace rast {
	template <typename fragment_output>
	inline constexpr bool is_discardable_v = false;

	template <typename discardable>
	constexpr bool should_discard(const discardable&);

	template <typename discardable>
	typename discardable::value_type get_frag_from_discardable(const discardable&);
}

