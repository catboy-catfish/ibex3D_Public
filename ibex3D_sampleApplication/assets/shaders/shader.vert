#version 450

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

// ----------------------------------------------------------------------------------------------------

void main()
{	
	vec4 worldSpacePosition = ubo.modelMatrix * vec4(inPosition, 1.0);
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * worldSpacePosition;

	fragPosition = worldSpacePosition.xyz;

	fragNormalDir = inNormalDir;
	fragTexCoord = inTexCoord;

	fragCameraPosition = ubo.cameraPosition;
}