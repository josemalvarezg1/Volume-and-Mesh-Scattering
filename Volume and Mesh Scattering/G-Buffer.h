#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>
#include <vector>

class light_buffer
{
public:
	GLuint g_buffer, g_position, g_normal, g_depth, depth_map, attachments[2];

	light_buffer(int g_width, int g_height);
	~light_buffer();

	void update_g_buffer(int g_width, int g_height);
};


class light_buffers_set
{
public:
	std::vector<light_buffer*> array_of_buffers;

	light_buffers_set();
	~light_buffers_set();
};