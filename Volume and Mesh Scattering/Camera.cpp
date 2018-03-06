#include "Camera.h"

camera::camera(glm::vec3 position)
{
	this->position = position;
	this->worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	this->yaw = YAW;
	this->pitch = PITCH;
	this->movementSpeed = SPEED;
	this->mouseSensitivity = SENSITIVTY;
	this->zoom = ZOOM;
	this->updateCameraVectors();
}

void camera::processKeyboard(cameraMovement direction, GLfloat deltaTime)
{
	GLfloat velocity = this->movementSpeed * deltaTime;
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

void camera::processMouseMovement(GLfloat xoffset, GLfloat yoffset)
{
	xoffset *= this->mouseSensitivity;
	yoffset *= this->mouseSensitivity;
	this->yaw += xoffset;
	this->pitch += yoffset;
	if (this->pitch > 89.0f)
		this->pitch = 89.0f;
	if (this->pitch < -89.0f)
		this->pitch = -89.0f;
	this->updateCameraVectors();
}

void camera::processMouseScroll(GLfloat yoffset)
{
	if (this->zoom >= 1.0f && this->zoom <= 45.0f)
		this->zoom -= yoffset;
	if (this->zoom <= 1.0f)
		this->zoom = 1.0f;
	if (this->zoom >= 45.0f)
		this->zoom = 45.0f;
}

glm::mat4 camera::getViewMatrix()
{
	return glm::lookAt(this->position, this->position + this->front, this->up);
}

void camera::updateCameraVectors()
{
	glm::vec3 front;
	front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
	front.y = sin(glm::radians(this->pitch));
	front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
	this->front = glm::normalize(front);
	this->right = glm::normalize(glm::cross(this->front, this->worldUp));
	this->up = glm::normalize(glm::cross(this->right, this->front));
}