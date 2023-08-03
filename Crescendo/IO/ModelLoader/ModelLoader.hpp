#pragma once

#include <vector>
#include <filesystem>

namespace Crescendo::IO
{
	struct Mesh
	{
		std::vector<float> vertices, normals, textureUVs;
		std::vector<uint32_t> indices;
	};
	struct Model
	{
		std::vector<Mesh> meshes;
	};

	Model LoadOBJ(const std::filesystem::path& path);
}