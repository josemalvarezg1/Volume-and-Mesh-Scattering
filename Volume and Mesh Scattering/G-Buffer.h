#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>
#include <vector>

class light_buffer
{
public:
	GLuint g_buffer[2], g_position, g_normal, g_depth;
	light_buffer(int g_width, int g_height, int layers);
	~light_buffer();
	void update_g_buffer(int g_width, int g_height, int layers);
};