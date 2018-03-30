#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

class ScatteredMap
{
public:
	unsigned int texture, buffer, g_depth, depth_map, array_texture, depth_texture;
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	std::vector<glm::vec3> cameraPositions;
	ScatteredMap::ScatteredMap(int g_width, int g_height, int layers);
	~ScatteredMap();
};