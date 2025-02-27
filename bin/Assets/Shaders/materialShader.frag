#version 430
layout(location=0) in vec2 TexCoord;
layout(location=1) in vec3 Normal;
layout(location=2) in vec3 FragPos;
layout(location=3) in mat3 TBN;

out vec4 Color;

struct PointLight
{
	vec3 position;
	vec3 color;
	float intensity;
};

struct DirectionalLight
{
	vec3 direction;
	vec3 color;
	float intensity;
};

struct Material
{
	float shininess;
	sampler2D tex;
};

layout(binding = 0) uniform sampler2D ourTexture;
layout(binding = 1) uniform sampler2D gradientSample;
layout(binding = 2) uniform sampler2D normalMap;
uniform int hasNormal = 0;

uniform PointLight pointLight;// = {vec3(0,1,0), vec3(1), 0};
uniform DirectionalLight directionalLight = {vec3(0,1,0), vec3(1), 1};
uniform Material material;
uniform vec3 viewPos;

vec3 calculateLight(vec3 lightDir, vec3 lightColor)
{
	// DIFFUSE
	vec3 norm;
	if (hasNormal == 1)
	{
		norm = texture(normalMap, TexCoord).rgb;
		norm = norm * 2.0 - 1.0;
		norm = normalize(TBN * norm);
	}
	else
	{
		norm = normalize(Normal);
	}
	float diff = max(dot(norm, lightDir), 0);
	//diff = min(diff, 0.99);
	vec4 c = texture(gradientSample, vec2(diff, 0.5));
	vec3 diffuse = diff * lightColor;

	// SPECULAR
	float specularStrength = 1;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
	vec3 specular = specularStrength * spec * lightColor;

	return diffuse + specular;
}

void main()
{
	// AMBIENT
	float ambientStrength = 0.1;
	vec3 ambient = vec3(ambientStrength);
	
	// APPLY THE FINAL TOUCHES
	vec3 result = ambient + (calculateLight(normalize(-directionalLight.direction), directionalLight.color) * directionalLight.intensity) + (calculateLight(normalize(pointLight.position - FragPos), pointLight.color) * pointLight.intensity);
	Color = vec4(result, 1) * texture(ourTexture, TexCoord);
}