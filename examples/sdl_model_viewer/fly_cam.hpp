#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace game {
	struct fly_cam {
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::quat rotation = glm::quat::wxyz(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec2 rot = glm::vec2(0.0f);
		float speed = 2.5f;
		float rot_speed = glm::pi<float>() * 0.05f;
		float max_rot = glm::pi<float>() * 0.49f;
		void update(glm::vec3 movementInput, glm::vec2 rotationInput, float deltaTime);
	};
}
