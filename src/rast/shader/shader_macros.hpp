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
		friend inline vertex_shader_output operator* (const vertex_shader_output& rhs, float lhs) {
			return { rhs.rastPos * lhs, rhs.data * lhs };
		}
		friend inline vertex_shader_output operator+ (const vertex_shader_output& rhs, const vertex_shader_output& lhs) {
			return { rhs.rastPos + lhs.rastPos, rhs.data + lhs.data };
		}
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
			friend inline position_normal operator* (const position_normal& rhs, float lhs) {
				return { rhs.position * lhs, rhs.normal * lhs };
			}
			friend inline position_normal operator+ (const position_normal& rhs, const position_normal& lhs) {
				return { rhs.position + lhs.position, rhs.normal + lhs.normal };
			}
		};
		class position_uv {
		public:
			position position;
			uv uv;
			friend inline position_uv operator* (const position_uv& rhs, float lhs) {
				return { rhs.position * lhs, rhs.uv * lhs };
			}
			friend inline position_uv operator+ (const position_uv& rhs, const position_uv& lhs) {
				return { rhs.position + lhs.position, rhs.uv + lhs.uv };
			}
		};
		class position_normal_uv {
		public:
			position position;
			normal normal;
			uv uv;
			friend inline position_normal_uv operator* (const position_normal_uv& rhs, float lhs) {
				return { rhs.position * lhs, rhs.normal * lhs };
			}
			friend inline position_normal_uv operator+ (const position_normal_uv& rhs, const position_normal_uv& lhs) {
				return { rhs.position + lhs.position, rhs.normal + lhs.normal, rhs.uv + lhs.uv };
			}
		};
		class normal_uv {
		public:
			normal normal;
			uv uv;
			friend inline normal_uv operator* (const normal_uv& rhs, float lhs) {
				return { rhs.normal * lhs, rhs.uv * lhs };
			}
			friend inline normal_uv operator+ (const normal_uv& rhs, const normal_uv& lhs) {
				return { rhs.normal + lhs.normal, rhs.uv + lhs.uv };
			}
		};
	}
}
