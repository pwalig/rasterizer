#pragma once
#include "color.hpp"

namespace rast::image {
	template <typename T>
	class view {
	public:
		using size_type = unsigned short;
		using color = T;
		
		color * const data;
		const size_type width;
		const size_type height;

		inline view(color* Data, size_type Width, size_type Height) :
			data(Data), width(Width), height(Height) { }

		inline color& at(size_type x, size_type y) { return data[y * width + x]; }
	};

	using rgba8 = view<color::rgba8>;
}
