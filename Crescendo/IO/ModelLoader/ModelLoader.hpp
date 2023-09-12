#pragma once

#include <vector>
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
			std::filesystem::path diffuse, specular, normal, emissive, occlusion, metallicRoughness;
			glm::mat4 transform;
			bool isDoubleSided, isTransparent;

			inline uint32_t GetTriangleCount() const { return this->indices.size() / 3; }
			inline uint32_t GetSize() const { return sizeof(float) * (this->vertices.size() + this->normals.size() + this->textureUVs.size()) + sizeof(uint32_t) * this->indices.size(); }
		};
		std::vector<Mesh> meshes;

		inline uint32_t GetTriangleCount() const
		{
			uint32_t count = 0;
			for (const auto& mesh : this->meshes) count += mesh.GetTriangleCount();
			return count;
		}
		inline uint32_t GetSize() const
		{
			uint32_t size = 0;
			for (const auto& mesh : this->meshes) size += mesh.GetSize();
			return size;
		}
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