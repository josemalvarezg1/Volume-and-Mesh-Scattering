#include "Mipmap.h"

mipmap::mipmap(int g_width, int g_height, int layers, int mipmaps)
{
	glGenFramebuffers(1, &this->buffer);
	glGenTextures(1, &this->array_texture);
	glGenTextures(1, &this->depth_texture);
	this->update_mipmap(g_width, g_height, layers, mipmaps);
}

mipmap::~mipmap()
{
	glDeleteFramebuffers(1, &this->buffer);
	glDeleteTextures(1, &this->array_texture);
	glDeleteTextures(1, &this->depth_texture);
}

void mipmap::update_mipmap(int g_width, int g_height, int layers, int mipmaps)
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, this->array_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	for (size_t i = 0; i < mipmaps; i++)
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, i, GL_RGBA32F, g_width, g_height, layers);

	glBindTexture(GL_TEXTURE_2D_ARRAY, this->depth_texture);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

	for (size_t i = 0; i < mipmaps; i++)
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, i, GL_DEPTH_COMPONENT32F, g_width, g_height, layers);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->buffer);
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, this->array_texture, 0);
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, this->depth_texture, 0);


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}