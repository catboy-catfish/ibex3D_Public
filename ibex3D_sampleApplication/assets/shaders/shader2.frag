#version 450

#include "include/commonStructs.glsl"
#include "include/basicLighting.glsl"

// ----------------------------------------------------------------------------------------------------

layout (binding = 1) uniform sampler2D texSampler;

// ----------------------------------------------------------------------------------------------------

layout (location = 0) in vec3 wsFragPosition;
layout (location = 1) in vec3 wsNormalDirection;
layout (location = 2) in vec2 uvCoordinates;
layout (location = 3) in vec3 wsCameraPosition;

layout (location = 4) in pointLightInfo light0;
layout (location = 6) in pointLightInfo light1;

layout (location = 0) out vec4 fragColor;

// ----------------------------------------------------------------------------------------------------

void main()
{
	vec3 normalDirection = normalize(wsNormalDirection);
	vec3 viewDirection = normalize(wsCameraPosition - wsFragPosition);

	vec3 lightDirection = normalize(light0.position - wsFragPosition);
	vec3 lightColor = light0.color.rgb * light0.color.a;
	vec3 diffuseLighting = lightColor * diffuse_lambertian(normalDirection, lightDirection);
	vec3 specularLighting = lightColor * specular_blinnPhong(normalDirection, lightDirection, viewDirection, 100.0);

	lightDirection = normalize(light1.position - wsFragPosition);
	lightColor = light1.color.rgb * light1.color.a;
	diffuseLighting += lightColor * diffuse_lambertian(normalDirection, lightDirection);
	specularLighting += lightColor * specular_blinnPhong(normalDirection, lightDirection, viewDirection, 100.0);

	diffuseLighting += vec3(0.01);

	vec3 diffuseColor = vec3(1.0f, 0.1f, 0.0f) * diffuseLighting;
	fragColor = vec4(diffuseColor + specularLighting, 1.0);
}