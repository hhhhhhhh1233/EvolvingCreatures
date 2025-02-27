#version 430
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

layout(location=0) in vec2 TexCoords;
layout(location=1) in vec3 FragPos;
layout(location=2) in vec3 Normal;
layout(location=3) in mat3 TBN;

layout(binding=0) uniform sampler2D ourTexture;
layout(binding=1) uniform sampler2D texture_specular1;
layout(binding=2) uniform sampler2D normal;

uniform int hasNormal;

void main()
{
	gPosition = FragPos;
	if (hasNormal == 0)
		gNormal = normalize(Normal);
	else if (hasNormal == 1)
	{
		gNormal = vec3(texture(normal, TexCoords).rgb);
        gNormal = gNormal * 2.0 - 1.0;
        gNormal = normalize(TBN * gNormal); 
	}
	gAlbedoSpec = texture(ourTexture, TexCoords);
}