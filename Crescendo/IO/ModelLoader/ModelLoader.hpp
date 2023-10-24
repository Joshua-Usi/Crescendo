#pragma once

#include <vector>
#include <filesystem>

#include "glm/glm.hpp"

#include "cs_std/graphics/mesh.hpp"

namespace Crescendo::IO
{
	struct Model
	{
		struct Mesh
		{
			cs_std::graphics::mesh meshData;
			// Texture map paths
			// Sometimes metallicRoughness is used instead of metallic and roughness
			std::filesystem::path diffuse, normal, emissive, occlusion, metallicRoughness;
			glm::mat4 transform;
			bool isDoubleSided, isTransparent;
		};
		std::vector<Mesh> meshes;
	};
	/// <summary>
	/// Load data from an OBJ file
	/// </summary>
	/// <param name="path">Path to obj, Texture files will automatically get the path</param>
	/// <returns>Model data</returns>
	Model LoadOBJ(const std::filesystem::path& path);
	/// <summary>
	/// Load data from a GLTF file
	/// </summary>
	/// <param name="path">Path to gltf, Texture files will automatically get the path</param>
	/// <returns>Model data</returns>
	Model LoadGLTF(const std::filesystem::path& path);
}