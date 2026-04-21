#version 450

#include "include/basicLighting.glsl"

// ----------------------------------------------------------------------------------------------------

layout (binding = 1) uniform sampler2D texSampler;

// ----------------------------------------------------------------------------------------------------

layout (location = 0) in vec3 fragPosition;
layout (location = 1) in vec3 fragNormalDir;
layout (location = 2) in vec2 fragTexCoord;
layout (location = 3) in vec3 fragCameraPosition;

layout (location = 0) out vec4 outColor;

// ----------------------------------------------------------------------------------------------------

void main()
{
	vec3 lightDir0 = vec3(3.0, 0.0, 3.0);										// Raw light position
	lightDir0 = normalize(lightDir0 - fragPosition);							// Converted to light direction

	vec3 lightDir1 = vec3(-3.0, 0.0, 3.0);
	lightDir1 = normalize(lightDir1 - fragPosition);

	vec3 normalDir = normalize(fragNormalDir);
	vec3 viewDir = normalize(fragCameraPosition - fragPosition);
	
	vec3 diffuseLighting = vec3(diffuse_lambertian(normalDir, lightDir0));		// Diffuse light 0
	diffuseLighting += vec3(diffuse_lambertian(normalDir, lightDir1));			// Diffuse light 1

	vec3 specularLighting = vec3(specular_blinnPhong(normalDir, lightDir0, viewDir, 1000.0));
	specularLighting += vec3(specular_blinnPhong(normalDir, lightDir1, viewDir, 1000.0));

	vec3 diffuseColor = texture(texSampler, fragTexCoord).rgb * diffuseLighting;
	outColor = vec4(diffuseColor + specularLighting, 1.0);
}