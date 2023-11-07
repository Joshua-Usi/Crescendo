#pragma once

#include <vector>
#include <filesystem>

#include "glm/glm.hpp"

#include "cs_std/graphics/model.hpp"

namespace Crescendo::IO
{
	/// <summary>
	/// Load data from an OBJ file
	/// </summary>
	/// <param name="path">Path to obj, Texture files will automatically get the path</param>
	/// <returns>Model data</returns>
	cs_std::graphics::model LoadOBJ(const std::filesystem::path& path);
	/// <summary>
	/// Load data from a GLTF file
	/// </summary>
	/// <param name="path">Path to gltf, Texture files will automatically get the path</param>
	/// <returns>Model data</returns>
	cs_std::graphics::model LoadGLTF(const std::filesystem::path& path);
}