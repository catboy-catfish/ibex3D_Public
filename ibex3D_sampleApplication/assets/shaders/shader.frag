#version 450

#include "include/commonStructs.glsl"
#include "include/basicLighting.glsl"

// ----------------------------------------------------------------------------------------------------

layout (binding = 1) uniform sampler2D texSampler;

// ----------------------------------------------------------------------------------------------------

layout (location = 0) in vec3 fragPosition;
layout (location = 1) in vec3 fragNormalDir;
layout (location = 2) in vec2 fragTexCoord;
layout (location = 3) in vec3 fragCameraPosition;
layout (location = 4) in pointLightInfo light0;
layout (location = 6) in pointLightInfo light1;

layout (location = 0) out vec4 outColor;

// ----------------------------------------------------------------------------------------------------

void main()
{
	vec3 normalDir = normalize(fragNormalDir);
	vec3 viewDir = normalize(fragCameraPosition - fragPosition);

	vec3 lightDir = normalize(light0.position - fragPosition);
	vec3 lightColor = light0.color.rgb * light0.color.a;
	vec3 diffuseLighting = lightColor * diffuse_lambertian(normalDir, lightDir);
	vec3 specularLighting = lightColor * specular_blinnPhong(normalDir, lightDir, viewDir, 100.0);

	lightDir = normalize(light1.position - fragPosition);
	lightColor = light1.color.rgb * light1.color.a;
	diffuseLighting += lightColor * diffuse_lambertian(normalDir, lightDir);
	specularLighting += lightColor * specular_blinnPhong(normalDir, lightDir, viewDir, 100.0);

	diffuseLighting += vec3(0.01);

	vec3 diffuseColor = texture(texSampler, fragTexCoord).rgb * diffuseLighting;
	outColor = vec4(diffuseColor + specularLighting, 1.0);
}