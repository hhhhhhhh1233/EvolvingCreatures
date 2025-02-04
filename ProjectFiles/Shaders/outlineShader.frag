#version 430
layout(location=0) in vec2 TexCoord;
layout(location=1) in vec3 Normal;
layout(location=2) in vec3 FragPos;
layout(location=3) in vec3 Tangent;

out vec4 Color;

uniform vec3 outlineColor;


void main()
{
	Color = vec4(outlineColor, 1);
}