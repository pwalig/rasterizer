#pragma once
#include <glm/glm.hpp>

namespace rast::shader {
	template <typename Shader>
	class vertex_shader_output {
	public:
		glm::vec4 rastPos;
		typename Shader::fragment::input data;
	};
}
