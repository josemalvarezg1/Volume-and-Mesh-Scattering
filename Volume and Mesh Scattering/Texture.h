#pragma once
#include <iostream>
#include <GL/glew.h>
#include <stb_image.h>

class texture
{
private:
	unsigned int texture_ID;

public:
	texture(char const * path);
	~texture();
	unsigned int get_texture_ID();
};