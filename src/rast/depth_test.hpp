#pragma once

namespace rast::depth_test {
	enum class option {
		always, never, less, equal, less_or_equal, greater, not_equal, greater_or_equal
	};


	template <typename depth_format>
	inline bool always(depth_format newDepth, depth_format oldDepth) {
		return true;
	}
	template <typename depth_format>
	inline bool never(depth_format newDepth, depth_format oldDepth) {
		return false;
	}
	template <typename depth_format>
	inline bool less(depth_format newDepth, depth_format oldDepth) {
		return newDepth < oldDepth;
	}
	template <typename depth_format>
	inline bool equal(depth_format newDepth, depth_format oldDepth) {
		return newDepth == oldDepth;
	}
	template <typename depth_format>
	inline bool less_or_equal(depth_format newDepth, depth_format oldDepth) {
		return newDepth <= oldDepth;
	}
	template <typename depth_format>
	inline bool greater(depth_format newDepth, depth_format oldDepth) {
		return newDepth > oldDepth;
	}
	template <typename depth_format>
	inline bool not_equal(depth_format newDepth, depth_format oldDepth) {
		return newDepth != oldDepth;
	}
	template <typename depth_format>
	inline bool greater_or_equal(depth_format newDepth, depth_format oldDepth) {
		return newDepth >= oldDepth;
	}

	namespace function {
		template <typename depth_format>
		using type = bool (*)(depth_format, depth_format);

		template <option>
		struct wrapper {
			template <typename depth_format>
			inline constexpr static function::type<depth_format> function = nullptr;
		};

		template<>
		struct wrapper<option::always> {
			template <typename depth_format>
			inline constexpr static function::type<depth_format> function = always<depth_format>;
		};
		template<>
		struct wrapper<option::never> {
			template <typename depth_format>
			inline constexpr static function::type<depth_format> function = never<depth_format>;
		};
		template<>
		struct wrapper<option::equal> {
			template <typename depth_format>
			inline constexpr static function::type<depth_format> function = equal<depth_format>;
		};
		template<>
		struct wrapper<option::less> {
			template <typename depth_format>
			inline constexpr static function::type<depth_format> function = less<depth_format>;
		};
		template<>
		struct wrapper<option::less_or_equal> {
			template <typename depth_format>
			inline constexpr static function::type<depth_format> function = less_or_equal<depth_format>;
		};
		template<>
		struct wrapper<option::greater> {
			template <typename depth_format>
			inline constexpr static function::type<depth_format> function = greater<depth_format>;
		};
		template<>
		struct wrapper<option::not_equal> {
			template <typename depth_format>
			inline constexpr static function::type<depth_format> function = not_equal<depth_format>;
		};
		template<>
		struct wrapper<option::greater_or_equal> {
			template <typename depth_format>
			inline constexpr static function::type<depth_format> function = greater_or_equal<depth_format>;
		};
	}

	namespace type {
		template <option Option>
		struct wrapper {
			inline constexpr static option value = Option;
			using function_wrapper = function::wrapper<Option>;

			// following code does not compile with Visual Studio (i get C3878), but does compile with CMake (with MVSC)
			//template <typename depth_format>
			//inline constexpr static function::type<depth_format> function = function_wrapper::function<depth_format>;
		};
		using always = wrapper<option::always>;
		using never = wrapper<option::never>;
		using less = wrapper<option::less>;
		using equal = wrapper<option::equal>;
		using less_or_equal = wrapper<option::less_or_equal>;
		using greater = wrapper<option::greater>;
		using not_equal = wrapper<option::not_equal>;
		using greater_or_equal = wrapper<option::greater_or_equal>;
	}
}
