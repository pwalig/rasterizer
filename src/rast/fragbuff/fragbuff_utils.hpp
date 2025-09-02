#pragma once

namespace rast::fragbuff {
	template <typename Shader>
	using output_function = void(*)(int, int, typename Shader::vertex::output*, int[3], int);
}
