#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

class scattered_map
{
public:
	unsigned int buffer, array_texture, depth_texture, attachments[2];
	scattered_map(int g_width, int g_height, int layers);
	~scattered_map();
	void update_scattered_map(int g_width, int g_height, int layers);
};