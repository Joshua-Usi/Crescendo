#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include "glm/glm.hpp"

namespace Crescendo::IO
{
	struct Model
	{
		struct Mesh
		{
			std::vector<float> vertices, normals, textureUVs;
			std::vector<uint32_t> indices;
			// Texture map paths
			std::string diffuse, specular, normal, emissive, occlusion;
			std::string metallicRoughness;
			glm::mat4 transform;
		};
		std::vector<Mesh> meshes;
	};

	Model LoadOBJ(const std::filesystem::path& path, const std::filesystem::path& texturePathPrepend = "");
	Model LoadGLTF(const std::filesystem::path& path, const std::filesystem::path& texturePathPrepend = "");
}