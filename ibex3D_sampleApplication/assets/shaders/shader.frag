#version 450

#include "include/basicLighting.glsl"

// ----------------------------------------------------------------------------------------------------

layout (binding = 1) uniform sampler2D texSampler;

// ----------------------------------------------------------------------------------------------------

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragNormal;
layout (location = 2) in vec2 fragTexCoord;

layout (location = 0) out vec4 outColor;

// ----------------------------------------------------------------------------------------------------

void main()
{
	vec3 diffuseColor = texture(texSampler, fragTexCoord).rgb;
	
	vec3 lightPos1 = vec3( 1.0,  0.0,  0.0);
	vec3 lightPos2 = vec3(-1.0,  0.0,  0.0);

	vec3 totalLighting = vec3(0.1);															// Ambient light
	totalLighting += diffuse_lambertian(fragNormal, -lightPos1) * vec3(0.5, 1.0, 0.5);		// Diffuse light 1
	totalLighting += diffuse_lambertian(fragNormal, -lightPos2) * vec3(1.0, 0.5, 1.0);		// Diffuse light 2

	outColor = vec4(diffuseColor * totalLighting, 1.0);
}