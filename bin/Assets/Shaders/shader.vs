#version 430
layout(location=0) in vec3 pos;
layout(location=1) in vec4 color;
layout(location=2) in vec2 aTexCoord;

uniform mat4 transform;
uniform mat4 viewProjection;

layout(location=0) out vec2 TexCoord;

void main()
{
	gl_Position = viewProjection * transform * vec4(pos, 1);
	TexCoord = aTexCoord;
}