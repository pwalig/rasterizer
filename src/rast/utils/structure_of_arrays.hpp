#pragma once
#include <tuple>
#include <array>

namespace rast::utils {
	template <template <typename> typename Container, typename ...Types>
	struct structure_of_arrays {
	private:
		std::tuple<Container<Types>...> fields;
	public:
		inline constexpr structure_of_arrays(Container<Types>&&... args) : fields(std::forward<Container<Types>>(args)...) {}

		template <typename T>
		inline constexpr const T& at(size_t i) const {
			return std::get<Container<T>>(fields).at(i);
		}
		template <typename T>
		inline constexpr T& at(size_t i) {
			return std::get<Container<T>>(fields).at(i);
		}
		template <typename T>
		inline constexpr const Container<T>& get() const {
			return std::get<Container<T>>(fields);
		}
		template <typename T>
		inline constexpr Container<T>& get() {
			return std::get<Container<T>>(fields);
		}
		template <typename T>
		inline constexpr const T* data() const {
			return std::get<Container<T>>(fields).data();
		}
		template <typename T>
		inline constexpr T* data() {
			return std::get<Container<T>>(fields).data();
		}
		template <size_t Index>
		inline constexpr const auto* data() const {
			return std::get<Index>(fields).data();
		}
		template <size_t Index>
		inline constexpr auto* data() {
			return std::get<Index>(fields).data();
		}
	};


	template <template <typename> typename Container>
	struct color_soa {
		structure_of_arrays<Container, uint8_t, uint8_t, uint8_t, uint8_t> data;
		inline constexpr const uint8_t* r() const {
			return data.data<0>();
		}
	};

	namespace structure_of_arrays_test {
		template <typename T>
		using container = std::array<T, 4>;

		inline constexpr bool structure_of_arrays_test() {
			using test_soa = structure_of_arrays<container, int, float>;

			auto tsoa = test_soa(container<int>(), container<float>());
			tsoa.at<int>(0) = 0;
			return tsoa.at<int>(0) == 0;
		}
		static_assert(structure_of_arrays_test());
	}
}
