#define STB_IMAGE_IMPLEMENTATION
#include "Texture.h"

texture::texture(char const *path)
{
	glGenTextures(1, &this->texture_ID);

	int width, height, components;
	unsigned char *data = stbi_load(path, &width, &height, &components, 0);
	if (data)
	{
		GLenum format;
		if (components == 1)
			format = GL_RED;
		else if (components == 3)
			format = GL_RGB;
		else if (components == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, this->texture_ID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "No se ha podido cargar la textura en: " << path << std::endl;
		stbi_image_free(data);
	}
}

texture::~texture()
{
}

unsigned int texture::get_texture_ID()
{
	return this->texture_ID;
}
