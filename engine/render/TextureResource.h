#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

class TextureResource
{
public:
	unsigned int texture;
	int width, height, nrChannels;
	unsigned char* data;

	TextureResource()
	{
		texture = 0;
		width = 0;
		height = 0;
		nrChannels = 0;
		data = 0;
	}

	TextureResource(const char* filename)
	{
		LoadFromFile(filename);
	}

	~TextureResource()
	{
		glDeleteTextures(1, &texture);
	}


	void LoadFromFile(const char* filename)
	{
		//stbi_set_flip_vertically_on_load(true);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		data = stbi_load(filename, &width, &height, &nrChannels, 0);
		if (data)
		{
			glGenTextures(1, &texture);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glBindTexture(GL_TEXTURE_2D, texture);
			if (nrChannels == 3)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			else if (nrChannels == 4)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load image" << std::endl;
		}
	}

	void BindTexture(int bind)
	{
		glActiveTexture(GL_TEXTURE0 + bind);
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	std::shared_ptr<TextureResource> MoveToSharedPointer()
	{
		return std::make_shared<TextureResource>(std::move(*this));
	}
};