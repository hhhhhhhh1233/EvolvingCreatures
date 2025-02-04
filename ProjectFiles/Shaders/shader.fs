#version 430

out vec4 Color;
layout(location=0) in vec2 TexCoord;
layout(binding=0) uniform sampler2D ourTexture;

void main()
{
	Color = texture(ourTexture, TexCoord);
}