#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class ScatteredMap
{
public:
	glm::vec3 position;
	unsigned int texture, buffer;
	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
	ScatteredMap(glm::vec3 position);
	~ScatteredMap();
};