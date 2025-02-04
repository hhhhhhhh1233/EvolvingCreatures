#pragma once

#include "render/ShaderResource.h"
#include "render/TextureResource.h"
#include <memory>

class Material
{
public:
	virtual void Apply() = 0;
};

class BlinnPhongMaterial : Material
{
public:
	std::shared_ptr<ShaderResource> shader;
	std::shared_ptr<TextureResource> texture;
	std::shared_ptr<TextureResource> normal;
	float shininess;

	BlinnPhongMaterial()
	{
		shader = NULL;
		texture = NULL;
		shininess = 32;
	}

	BlinnPhongMaterial(const char* vertexShaderPath, const char* fragmentShaderPath, const char* texturePath, float Shininess)
	{
		shader->LoadShaders(vertexShaderPath, fragmentShaderPath);
		texture->LoadFromFile(texturePath);
		shininess = Shininess;
	}

	BlinnPhongMaterial(std::shared_ptr<ShaderResource> Shader, std::shared_ptr<TextureResource> Texture, float Shininess)
	{
		shader = Shader;
		texture = Texture;
		shininess = Shininess;
	}

	void Apply()
	{
		shader->UseProgram();
		texture->BindTexture(0);
		if (normal != nullptr)
		{
			if (normal->texture != 0)
				shader->SetInt("hasNormal", 1);
			else
				shader->SetInt("hasNormal", 0);
			normal->BindTexture(2);
		}
		shader->SetFloat("material.shininess", shininess);
	}
};