#include <ibex3D/core/entryPoint.h>

#include <stdio.h>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>

static bool load(std::filesystem::path path)
{
	fastgltf::Parser parser;

	auto data = fastgltf::GltfDataBuffer::FromPath(path);

	if (data.error() != fastgltf::Error::None)
	{
		return false;
	}

	auto asset = parser.loadGltf(data.get(), path.parent_path(), fastgltf::Options::None);

	if (asset.error() != fastgltf::Error::None)
	{
		return false;
	}

	for (auto& buffer : asset->buffers)
	{

	}

	// fastgltf::validate(asset.get());

	return true;
}

int ibex3D_entryPoint()
{
	if (load("assets/models/export3dcoat.glb"))
	{
		fprintf(stdout, "Successfully loaded the .glb model file.\n");
	}
	
	return 0;
}