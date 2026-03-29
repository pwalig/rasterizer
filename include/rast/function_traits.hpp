#pragma once
#include <tuple>
#include <type_traits>

namespace rast {
	template <typename Signature>
	struct function_traits;

	template <typename R, typename ...Args>
	struct function_traits<R(Args...)> {
		using return_type = R;
		using argument_types = std::tuple<Args...>;
	};

	template <typename R, typename ...Args>
	struct function_traits<R(*)(Args...)> {
		using return_type = R;
		using argument_types = std::tuple<Args...>;
	};

	template <typename Signature>
	using function_argument_types = typename function_traits<Signature>::argument_types;

	template <typename Signature>
	using function_return_type = typename function_traits<Signature>::return_type;

	template <typename Signature, size_t N = 0>
	using function_argument = std::tuple_element_t<N, function_argument_types<Signature>>;


	// skip first
	template <typename T>
	struct tuple_skip_first;

	template <typename First, typename... Rest>
	struct tuple_skip_first<std::tuple<First, Rest...>>
	{
	  using type = std::tuple<Rest...>;
	};

	template <typename T>
	using tuple_skip_first_t = typename tuple_skip_first<T>::type;

	// remove cv
	template <typename T>
	struct tuple_remove_cv;

	template <typename ...Args>
	struct tuple_remove_cv<std::tuple<Args...>> {
		using type = std::tuple<std::remove_cv_t<Args>...>;
	};

	template <typename T>
	using tuple_remove_cv_t = typename tuple_remove_cv<T>::type;

	// remove reference
	template <typename T>
	struct tuple_remove_reference;

	template <typename ...Args>
	struct tuple_remove_reference<std::tuple<Args...>> {
		using type = std::tuple<std::remove_reference_t<Args>...>;
	};

	template <typename T>
	using tuple_remove_reference_t = typename tuple_remove_reference<T>::type;

	// remove cvref
	template <typename T>
	using tuple_remove_cvref_t = tuple_remove_cv_t<tuple_remove_reference_t<T>>;
}
