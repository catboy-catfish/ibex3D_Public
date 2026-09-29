#pragma once

#include <vector>

#include <thirdparty/glm/vec2.hpp>
#include <thirdparty/glm/vec3.hpp>

// ----------------------------------------------------------------------------------------------------

struct i3D_vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;

	// ----------------------------------------------------------------------------------------------------

	static size_t getPositionByteOffset();
	static size_t getNormalByteOffset();
	static size_t getTexCoordByteOffset();

	// ----------------------------------------------------------------------------------------------------

	bool operator == (const i3D_vertex& other) const
	{
		return (position == other.position)
			&& (normal == other.normal)
			&& (texCoord == other.texCoord);
	}
};

// ----------------------------------------------------------------------------------------------------

class i3D_staticMeshLoader
{
public:
	void initTestMesh();

	bool loadMeshFromObj(const char* objFilePath);
	void cleanup();

	size_t getVertexCount();
	size_t getIndexCount();

public:
	std::vector<i3D_vertex> vertices;
	std::vector<uint32_t> indices;
};