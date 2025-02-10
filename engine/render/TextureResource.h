#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
//#include "stb_image.h"
//#include <iostream>

class TextureResource
{
public:
	unsigned int texture;
	int width, height, nrChannels;
	unsigned char* data;

	TextureResource();

	TextureResource(const char* filename);

	~TextureResource();

	void LoadFromFile(const char* filename);
	void BindTexture(int bind);
	std::shared_ptr<TextureResource> MoveToSharedPointer();
};