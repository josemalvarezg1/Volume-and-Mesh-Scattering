#include "G-Buffer.h"

light_buffer::light_buffer(int g_width, int g_height)
{
	glGenFramebuffers(1, &this->g_buffer);
	glGenTextures(1, &this->g_position);
	glGenTextures(1, &this->g_normal);
	glGenTextures(1, &this->depth_map);
	glGenRenderbuffers(1, &this->g_depth);
	this->attachments[0] = GL_COLOR_ATTACHMENT0;
	this->attachments[1] = GL_COLOR_ATTACHMENT1;
	this->update_g_buffer(g_width, g_height);
}

light_buffer::~light_buffer()
{
	glDeleteFramebuffers(1, &this->g_buffer);
	glDeleteTextures(1, &this->g_position);
	glDeleteTextures(1, &this->g_normal);
	glDeleteRenderbuffers(1, &this->g_depth);
}

void light_buffer::update_g_buffer(int g_width, int g_height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, this->g_buffer);

	glBindTexture(GL_TEXTURE_2D, this->g_position);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, g_width, g_height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->g_position, 0);

	glBindTexture(GL_TEXTURE_2D, this->g_normal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, g_width, g_height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, this->g_normal, 0);

	glBindTexture(GL_TEXTURE_2D, depth_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, g_width, g_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_map);

	//glBindRenderbuffer(GL_RENDERBUFFER, this->g_depth);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, g_width, g_height);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->g_depth);

	glDrawBuffers(2, attachments);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


light_buffers_set::light_buffers_set()
{
}

light_buffers_set::~light_buffers_set()
{
	this->array_of_buffers.empty();
}