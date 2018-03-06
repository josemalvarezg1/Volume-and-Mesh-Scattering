#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum cameraMovement
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
const GLfloat SPEED = 0.8f;
const GLfloat SENSITIVTY = 0.25f;
const GLfloat ZOOM = 45.0f;

class camera
{
public:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;
	GLfloat yaw;
	GLfloat pitch;
	GLfloat movementSpeed;
	GLfloat mouseSensitivity;
	GLfloat zoom;

	camera(glm::vec3 position);
	void processKeyboard(cameraMovement direction, GLfloat deltaTime);
	void processMouseMovement(GLfloat xoffset, GLfloat yoffset);
	void processMouseScroll(GLfloat yoffset);
	glm::mat4 getViewMatrix();

private:
	void updateCameraVectors();
};