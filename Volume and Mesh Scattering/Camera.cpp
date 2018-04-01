#include "Camera.h"

camera::camera(glm::vec3 position)
{
	this->position = position;
	this->world_up = glm::vec3(0.0f, 1.0f, 0.0f);
	this->yaw = YAW;
	this->pitch = PITCH;
	this->movement_speed = SPEED;
	this->mouse_sensitivity = SENSITIVTY;
	this->zoom = ZOOM;
	this->update_camera_vectors();
}

void camera::process_keyboard(camera_movement direction, GLfloat delta_time)
{
	GLfloat velocity = this->movement_speed * delta_time;
	if (direction == FORWARD)
		this->position += this->front * velocity;
	if (direction == BACKWARD)
		this->position -= this->front * velocity;
	if (direction == LEFT)
		this->position -= this->right * velocity;
	if (direction == RIGHT)
		this->position += this->right * velocity;
	if (direction == DONW)
		this->position -= this->up * velocity;
	if (direction == UP)
		this->position += this->up * velocity;
}

void camera::process_mouse_movement(GLfloat x_offset, GLfloat y_offset)
{
	x_offset *= this->mouse_sensitivity;
	y_offset *= this->mouse_sensitivity;
	this->yaw += x_offset;
	this->pitch += y_offset;
	if (this->pitch > 89.0f)
		this->pitch = 89.0f;
	if (this->pitch < -89.0f)
		this->pitch = -89.0f;
	this->update_camera_vectors();
}

void camera::process_mouse_scroll(GLfloat y_offset)
{
	if (this->zoom >= 1.0f && this->zoom <= 45.0f)
		this->zoom -= y_offset;
	if (this->zoom <= 1.0f)
		this->zoom = 1.0f;
	if (this->zoom >= 45.0f)
		this->zoom = 45.0f;
}

glm::mat4 camera::get_view_matrix()
{
	return glm::lookAt(this->position, this->position + this->front, this->up);
}

void camera::update_camera_vectors()
{
	glm::vec3 front;
	front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
	front.y = sin(glm::radians(this->pitch));
	front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
	this->front = glm::normalize(front);
	this->right = glm::normalize(glm::cross(this->front, this->world_up));
	this->up = glm::normalize(glm::cross(this->right, this->front));
}