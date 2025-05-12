#pragma once
#include <glm/glm.hpp>

#define rast_shader_fragment_shade() \
inline static output shade( \
	const input& frag0, const input& frag1, const input& frag2, \
	const glm::vec3& coefs \
) { return shade(interpolate(frag0, frag1, frag2, coefs)); }

namespace rast::shader {
	template <typename Shader>
	class vertex_shader_output {
	public:
		glm::vec4 rastPos;
		typename Shader::fragment::input data;
	};

	namespace inputs {
		using position = glm::vec3;
		using normal = glm::vec3;
		using uv = glm::vec2;
		using tangent = glm::vec3;
		using bitangent = glm::vec3;
		class position_normal {
		public:
			position position;
			normal normal;
		};
		class position_uv {
		public:
			position position;
			uv uv;
		};
		class position_normal_uv {
		public:
			position position;
			normal normal;
			uv uv;
		};
	}
}
