#version 430
out vec4 Color;

in vec2 TexCoords;

layout (binding = 0) uniform sampler2D gPosition;
layout (binding = 1) uniform sampler2D gNormal;
layout (binding = 2) uniform sampler2D gAlbedoSpec;

struct PointLight
{
	vec3 position;
	vec3 color;
	float intensity;

    float Linear;
    float Quadratic;
    float Radius;
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
};

const int NR_LIGHTS = 200;
uniform PointLight lights[NR_LIGHTS];

uniform PointLight pointLight;
uniform DirectionalLight directionalLight = {vec3(0,1,0), vec3(1,1,0), 0.3};
uniform Material material;
uniform vec3 viewPos;

vec3 calcPointLights(vec3 viewDir)
{
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;

    vec3 lighting = Diffuse * 0.1; 
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        float dist = length(lights[i].position - FragPos);
        if (dist < lights[i].Radius)
        {
            vec3 lightDir = normalize(lights[i].position - FragPos);
            vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].color;
            // specular
            vec3 halfwayDir = normalize(lightDir + viewDir);  
            float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
            vec3 specular = lights[i].color * spec * Specular;
            // attenuation
            float attenuation = 1.0 / (1.0 + lights[i].Linear * dist + lights[i].Quadratic * dist * dist);
            diffuse *= attenuation;
            specular *= attenuation;
            lighting += (diffuse + specular) * lights[i].intensity;
        }
    }
    return lighting;
}

vec3 calcDirectionalLight(vec3 viewDir)
{
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    
    vec3 diffuse = max(dot(Normal, normalize(directionalLight.direction)), 0.0) * Diffuse * directionalLight.color;
    // specular
    vec3 halfwayDir = normalize(directionalLight.direction + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
    vec3 specular = directionalLight.color * spec * Specular;
    return (diffuse + specular) * directionalLight.intensity;
}

void main()
{             
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 viewDir  = normalize(viewPos - FragPos);
    
    vec3 result = calcPointLights(viewDir) + calcDirectionalLight(viewDir);  
    Color = vec4(result, 1.0);
}