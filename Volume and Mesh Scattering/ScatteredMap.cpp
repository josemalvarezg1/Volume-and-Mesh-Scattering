#include "ScatteredMap.h"

scattered_map::scattered_map(int g_width, int g_height, int layers)
{
	glGenFramebuffers(1, &this->buffer);
	glGenTextures(1, &this->array_texture);
	glGenTextures(1, &this->depth_texture);
	this->update_scattered_map(g_width, g_height, layers);
}

scattered_map::~scattered_map() 
{
	glDeleteFramebuffers(1, &this->buffer);
	glDeleteTextures(1, &this->array_texture);
	glDeleteTextures(1, &this->depth_texture);
}

void scattered_map::update_scattered_map(int g_width, int g_height, int layers)
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, this->array_texture);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA32F, g_width, g_height, layers, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D_ARRAY, this->depth_texture);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32, g_width, g_height, layers, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->buffer);
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, this->array_texture, 0);
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, this->depth_texture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}