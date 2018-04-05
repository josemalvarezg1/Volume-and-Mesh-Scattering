#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

class mipmap
{
public:
	unsigned int buffer, array_texture, depth_texture;
	mipmap(int g_width, int g_height, int layers, int mipmaps);
	~mipmap();
	void update_mipmap(int g_width, int g_height, int layers, int mipmaps);
};