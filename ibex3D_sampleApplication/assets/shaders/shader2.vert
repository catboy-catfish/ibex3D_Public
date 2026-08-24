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

layout (location = 0) in vec3 msVertexPosition;
layout (location = 1) in vec3 msNormalDirection;
layout (location = 2) in vec2 vertexUvCoordinates;

layout (location = 0) out vec3 wsFragPosition;
layout (location = 1) out vec3 wsNormalDirection;
layout (location = 2) out vec2 uvCoordinates;
layout (location = 3) out vec3 wsCameraPosition;

layout (location = 4) out pointLightInfo light0;
layout (location = 6) out pointLightInfo light1;

// ----------------------------------------------------------------------------------------------------

void main()
{	
	vec4 wsFragPos = ubo.modelMatrix * vec4(msVertexPosition, 1.0);
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * wsFragPos;

	wsFragPosition = wsFragPos.xyz;
	wsNormalDirection = mat3(ubo.modelMatrix) * msNormalDirection;
	uvCoordinates = vertexUvCoordinates;
	wsCameraPosition = ubo.cameraPosition;

	light0.position = (ubo.modelMatrix * vec4(-3.0, 0.0, 3.0, 1.0)).xyz;
	light0.color = vec4(0.95, 1.0, 0.95, 1.0);

	light1.position = (ubo.modelMatrix * vec4(3.0, 0.0, 3.0, 1.0)).xyz;
	light1.color = vec4(1.0, 0.95, 1.0, 0.1);
}