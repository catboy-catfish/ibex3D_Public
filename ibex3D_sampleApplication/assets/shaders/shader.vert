#version 450

#include "include/commonStructs.glsl"

// ----------------------------------------------------------------------------------------------------

layout (binding = 0) uniform UniformBufferObject
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	vec3 cameraPosition;
	float padding;
} ubo;

// ----------------------------------------------------------------------------------------------------

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormalDir;
layout (location = 2) in vec2 inTexCoord;

layout (location = 0) out vec3 fragPosition;
layout (location = 1) out vec3 fragNormalDir;
layout (location = 2) out vec2 fragTexCoord;
layout (location = 3) out vec3 fragCameraPosition;
layout (location = 4) out pointLightInfo light0;
layout (location = 6) out pointLightInfo light1;

// ----------------------------------------------------------------------------------------------------

void main()
{	
	vec4 worldSpacePosition = ubo.modelMatrix * vec4(inPosition, 1.0);
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * worldSpacePosition;

	fragPosition = worldSpacePosition.xyz;
	fragNormalDir = mat3(ubo.modelMatrix) * inNormalDir;
	fragTexCoord = inTexCoord;
	fragCameraPosition = ubo.cameraPosition;

	light0.position = (ubo.modelMatrix * vec4(-3.0, 0.0, 3.0, 1.0)).xyz;
	light0.color = vec4(0.95, 1.0, 0.95, 1.0);

	light1.position = (ubo.modelMatrix * vec4(3.0, 0.0, 3.0, 1.0)).xyz;
	light1.color = vec4(1.0, 0.95, 1.0, 0.1);
}