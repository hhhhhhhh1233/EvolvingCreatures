#version 430
layout(location=0) in vec2 TexCoord;
layout(location=1) in vec3 Normal;
layout(location=2) in vec3 FragPos;

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

uniform sampler2D ourTexture;
uniform PointLight pointLight;
uniform DirectionalLight directionalLight;
uniform vec3 viewPos;

vec3 applyPointLight()
{
	// NORMAL
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(pointLight.position - FragPos);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * pointLight.color;

	// SPECULAR
	float specularStrength = 0.5;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * pointLight.color;

	return diffuse + specular;
}

vec3 applyDirectionalLight()
{
	// NORMAL
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(-directionalLight.direction);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * directionalLight.color;

	// SPECULAR
	float specularStrength = 0.5;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * directionalLight.color;

	return diffuse + specular;
}

void main()
{
	// AMBIENT
	float ambientStrength = 0.01;
	vec3 ambient = ambientStrength * directionalLight.color * pointLight.color;
	
	vec3 dsd = applyDirectionalLight();
	vec3 dsp = applyPointLight();

	// APPLY THE FINAL TOUCHES
	vec3 result = ambient + (dsd * directionalLight.intensity) + (dsp * pointLight.intensity);
	Color = vec4(result, 1.0) * texture(ourTexture, TexCoord);
}