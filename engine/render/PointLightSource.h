#pragma once
#include "core/math/mat4.h"
#include "render/ShaderResource.h"

// A light source from a point in space shooting light out in every direction
class PointLightSource
{
public:
	vec3 position;
	vec3 color;
	float intensity;

	PointLightSource()
	{
		position = vec3(0,0,0);
		color = vec3(1,1,1);
		intensity = 1;
	}

	PointLightSource(vec3 Position, vec3 Color, float Intensity)
	{
		position = Position;
		color = Color;
		intensity = Intensity;
	}

	void UpdateShader(ShaderResource *shader) const
	{
		shader->UseProgram();
		shader->SetVec3("pointLight.color", color);
		shader->SetVec3("pointLight.position", position);
		shader->SetFloat("pointLight.intensity", intensity);
	}
};

// Light rays shooting down parallell each other similar to the sun
class DirectionalLightSource
{
public:
	vec3 direction;
	vec3 color;
	float intensity;

	DirectionalLightSource()
	{
		direction = vec3(0, 1, 0);
		color = vec3(1, 1, 1);
		intensity = 1;
	}

	DirectionalLightSource(vec3 Direction, vec3 Color, float Intensity)
	{
		direction = Direction;
		color = Color;
		intensity = Intensity;
	}

	void UpdateShader(ShaderResource *shader)
	{
		shader->SetVec3("directionalLight.color", color);
		shader->SetVec3("directionalLight.direction", direction);
		shader->SetFloat("directionalLight.intensity", intensity);
	}
};