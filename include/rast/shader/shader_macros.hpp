#pragma once
#include <glm/glm.hpp>
#include "../math/vec.hpp"
#include "../is_discardable.hpp"

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

	template <typename Shader>
	struct shader_uniform_buffer {
		typename Shader::fragment::uniform_buffer fragment;
		typename Shader::vertex::uniform_buffer vertex;
	};

	namespace uniforms {
		struct PVM_struct {
			glm::mat4 PVM;
		};
		using PVM_matrix = glm::vec4;

		struct MVP {
			glm::mat4 M = glm::mat4(1.0f);
			glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 P = glm::perspective(glm::radians(70.0f), 16.0f / 9.0f, 0.1f, 100.0f);
		};
	}
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

			inline static void format(
				inputs::position* posBegin,
				inputs::uv* uvBegin,
				size_t count,
				position_uv* outBegin
			) {
				for (size_t i = 0; i < count; ++i) {
					outBegin->position = *posBegin;
					outBegin->uv = *uvBegin;
					++posBegin;
					++uvBegin;
					++outBegin;
				}
			}
		};
		class position_normal_uv {
		public:
			position position;
			normal normal;
			uv uv;
			friend inline position_normal_uv operator* (const position_normal_uv& rhs, float lhs) {
				return { rhs.position * lhs, rhs.normal * lhs, rhs.uv * lhs };
			}
			friend inline position_normal_uv operator+ (const position_normal_uv& rhs, const position_normal_uv& lhs) {
				return { rhs.position + lhs.position, rhs.normal + lhs.normal, rhs.uv + lhs.uv };
			}
			inline static void flip_uv(position_normal_uv& v) { v.uv.y = -v.uv.y; }
			inline static void format(
				inputs::position* posBegin,
				inputs::normal* normBegin,
				inputs::uv* uvBegin,
				size_t count,
				position_normal_uv* outBegin
			) {
				for (size_t i = 0; i < count; ++i) {
					outBegin->position = *posBegin;
					outBegin->normal = *normBegin;
					outBegin->uv = *uvBegin;
					++posBegin;
					++normBegin;
					++uvBegin;
					++outBegin;
				}
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
			template <size_t Count>
			struct vec {
				math::f32vec3x<Count> normal;
				math::f32vec2x<Count> uv;
			};
			template <size_t Count>
			inline constexpr vec<Count> vectorize() const {
				return {
					math::vectorize<3, Count>(normal),
					math::vectorize<2, Count>(uv)
				};
			}
		};
	}

	namespace outputs {
		template <typename output>
		class discardable {
			output value;
			bool discarded;
			inline discardable() : value(), discarded(true) {}
		public:
			using value_type = output;
			using reference = std::add_lvalue_reference_t<value_type>;
			using pointer = std::add_pointer_t<value_type>;
			using const_reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;
			using const_pointer = std::add_pointer_t<std::add_const_t<value_type>>;

			inline discardable(output&& v) : value(std::forward<output>(v)), discarded(false) {}
			static discardable discard() { return discardable(); }

			inline constexpr const_reference operator*() const { return value; }
			inline constexpr reference operator*() { return value; }

			inline constexpr explicit operator bool() const { return !discarded; }
		};

		template <typename T>
		struct alpha_discardable {
		private:
			using U = typename T::value_type;
			T _value;
			inline constexpr alpha_discardable() : _value() { _value[4] = U(0); }
		public:
			using value_type = T;
			using reference = std::add_lvalue_reference_t<value_type>;
			using pointer = std::add_pointer_t<value_type>;
			using const_reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;
			using const_pointer = std::add_pointer_t<std::add_const_t<value_type>>;

			inline constexpr alpha_discardable(value_type&& Value) : _value(std::move(Value)) {}
			inline static constexpr alpha_discardable discard() { return alpha_discardable(); }

			inline constexpr const_reference operator*() const { return _value; }
			inline constexpr reference operator*() { return _value; }

			inline constexpr explicit operator bool() const { return _value[4] != U(0); }
		};
	}
}

namespace rast {
	template <typename T>
	struct is_discardable<shader::outputs::discardable<T>> {
		inline static constexpr bool value = true;
	};
	template <typename T>
	struct is_discardable<shader::outputs::alpha_discardable<T>> {
		inline static constexpr bool value = true;
	};
}
