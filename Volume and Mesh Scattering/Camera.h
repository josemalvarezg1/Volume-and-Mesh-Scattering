#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum camera_movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DONW
};

const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 1.5f;
const GLfloat SENSITIVTY = 0.25f;
const GLfloat ZOOM = 45.0f;

class camera
{
public:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 world_up;
	GLfloat yaw;
	GLfloat pitch;
	GLfloat movement_speed;
	GLfloat mouse_sensitivity;
	GLfloat zoom;

	camera(glm::vec3 position);
	void process_keyboard(camera_movement direction, GLfloat delta_time);
	void process_mouse_movement(GLfloat x_offset, GLfloat y_offset);
	void process_mouse_scroll(GLfloat y_offset);
	glm::mat4 get_view_matrix();

private:
	void update_camera_vectors();
};