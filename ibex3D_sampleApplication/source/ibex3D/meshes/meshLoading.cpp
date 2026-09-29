#include <ibex3D/meshes/meshLoading.h>

#include <ibex3D/core/logger.h>

#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL
#include <thirdparty/tinyobjloader/tiny_obj_loader.h>	// includes map, string and vector
#include <thirdparty/glm/gtx/hash.hpp>

// ----------------------------------------------------------------------------------------------------

namespace std
{
	template<> struct hash<i3D_vertex>
	{
		size_t operator()(i3D_vertex const& vertex) const
		{
			// The ^ is the bitwise XOR operator.
			// TODO: Look into hashing to better understand this bullshit. https://en.cppreference.com/cpp/utility/hash

			return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

size_t i3D_vertex::getPositionByteOffset()
{
	return offsetof(i3D_vertex, position);
}

size_t i3D_vertex::getNormalByteOffset()
{
	return offsetof(i3D_vertex, normal);
}

size_t i3D_vertex::getTexCoordByteOffset()
{
	return offsetof(i3D_vertex, texCoord);
}

// ----------------------------------------------------------------------------------------------------

void i3D_staticMeshLoader::initTestMesh()
{
	vertices =
	{
		{ { -0.5f, -0.5f,  0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
		{ {  0.5f, -0.5f,  0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f,  0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { -0.5f,  0.5f,  0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } }
	};

	indices = { 0, 1, 2, 2, 3, 0 };
}

bool i3D_staticMeshLoader::loadMeshFromObj(const char* objFilePath)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, objFilePath))
	{
		i3D_logErrorMessage("MESH LOADING ERROR: Failed to load the Wavefront .obj model file at path \"%s\". Have you ensured that the provided model file path is correct?\n", objFilePath);
		return false;
	}

	std::unordered_map<i3D_vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			i3D_vertex vertex = {};

			int startVertexIdx = 3 * index.vertex_index;
			int startNormalIdx = 3 * index.normal_index;
			int startTexCoordIdx = 2 * index.texcoord_index;

			vertex.position =
			{
				attrib.vertices[startVertexIdx],
				attrib.vertices[startVertexIdx + 1],
				attrib.vertices[startVertexIdx + 2]
			};

			vertex.normal =
			{
				attrib.normals[startNormalIdx],
				attrib.normals[startNormalIdx + 1],
				attrib.normals[startNormalIdx + 2]
			};

			vertex.texCoord =
			{
				attrib.texcoords[startTexCoordIdx],
				1.0f - attrib.texcoords[startTexCoordIdx + 1],
			};

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}

	return true;
}

void i3D_staticMeshLoader::cleanup()
{
	vertices.clear();
	indices.clear();
}

size_t i3D_staticMeshLoader::getVertexCount()
{
	return vertices.size();
}

size_t i3D_staticMeshLoader::getIndexCount()
{
	return indices.size();
}