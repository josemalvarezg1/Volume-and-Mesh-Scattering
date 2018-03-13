#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class ScatteredMap
{
public:
	glm::vec3 position;
	unsigned int texture, texture2, buffer;
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	ScatteredMap(glm::vec3 position);
	void ScatteredMap::create_gbuffer();
	~ScatteredMap();
};