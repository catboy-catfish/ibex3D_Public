#version 450

#include "include/basicLighting.glsl"

// ----------------------------------------------------------------------------------------------------

layout (binding = 1) uniform sampler2D texSampler;

// ----------------------------------------------------------------------------------------------------

layout (location = 0) in vec3 fragPosition;
layout (location = 1) in vec3 fragNormalDir;
layout (location = 2) in vec2 fragTexCoord;
layout (location = 3) in vec3 fragViewDir;

layout (location = 0) out vec4 outColor;

// ----------------------------------------------------------------------------------------------------

void main()
{
	vec3 diffuseColor = texture(texSampler, fragTexCoord).rgb;
	
	vec3 lightPos = vec3(0.0, 0.0, 1.0);
	vec3 lightDir = normalize(lightPos - fragPosition);

	vec3 diffuseLighting = vec3(1.0, 0.0, 0.0) * diffuse_lambertian(fragNormalDir, lightDir);
	vec3 specularLighting = vec3(1.0, 1.0, 1.0) * specular_blinnPhong(fragNormalDir, lightDir, fragViewDir, 1.0);

	outColor = vec4(diffuseLighting + specularLighting, 1.0);
}