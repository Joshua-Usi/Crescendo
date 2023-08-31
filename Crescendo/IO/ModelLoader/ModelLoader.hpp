#pragma once

#include <vector>
#include <string>
#include <filesystem>

namespace Crescendo::IO
{
	struct Model
	{
		struct Mesh
		{
			std::vector<float> vertices, normals, textureUVs;
			std::vector<uint32_t> indices;
			// Texture map paths
			std::string albedo;
		};
		std::vector<Mesh> meshes;
	};

	Model LoadOBJ(const std::filesystem::path& path);
}