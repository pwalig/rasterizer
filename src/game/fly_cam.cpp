#include "fly_cam.hpp"

void game::fly_cam::update(glm::vec3 move, glm::vec2 rotRead, float deltaTime)
{
	// rotation
	rot += rotRead * rot_speed * deltaTime;
	if (rot.x > max_rot) rot.x = max_rot;
	if (rot.x < -max_rot) rot.x = -max_rot;

	rotation = glm::rotate(glm::quat(glm::vec3(0.0f)), rot.y, glm::vec3(0.0f, 1.0f, 0.0f)); // rotate around y axis only to preserve movement on xz plane

	// movement
	glm::vec3 moveOut = rotation * glm::vec3(move.x, 0.0f, move.z);
    moveOut.y = move.y;

    this->position += moveOut * deltaTime * speed;
    rotation = glm::rotate(rotation, rot.x, glm::vec3(1.0f, 0.0f, 0.0f));
}
