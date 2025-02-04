#version 430

layout(location=0) in vec3 pos;
layout(location=1) in vec2 aTexCoord;
layout(location=2) in vec3 normal;

layout(location=0) out vec2 TexCoord;
layout(location=1) out vec3 Normal;
layout(location=2) out vec3 FragPos;

uniform mat4 transform;
uniform mat4 viewProjection;

void main()
{
	gl_Position = viewProjection * transform * vec4(pos, 1);
	TexCoord = aTexCoord;
	Normal = vec3(transform * vec4(normal, 0.0f));
	FragPos = vec3(transform * vec4(pos, 1.0));
}