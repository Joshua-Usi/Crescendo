#include "ModelLoader.hpp"

#include "rapidobj/rapidobj.hpp"

#include "Core/common.hpp"

namespace Crescendo::IO
{
	Model LoadOBJ(const std::filesystem::path& path)
	{

		constexpr uint32_t VERTICES_PER_FACE = 3;

		const std::filesystem::path texturePathPrepend = path.parent_path();

		Model model;

		rapidobj::Result result = rapidobj::ParseFile(path);
		rapidobj::Triangulate(result);

		if (result.error)
		{
			CS_ASSERT(false, "Failed to load obj: " + path.string());
		}

		auto& attrib = result.attributes;
		auto& shapes = result.shapes;
		auto& materials = result.materials;

		model.meshes.resize(shapes.size());

		for (size_t s = 0, sSize = shapes.size(); s < sSize; s++)
		{
			model.meshes[s].transform = glm::mat4(1.0f);

			size_t indexOffset = 0;
			model.meshes[s].vertices.resize  (3 * VERTICES_PER_FACE * shapes[s].mesh.num_face_vertices.size());
			model.meshes[s].normals.resize   (3 * VERTICES_PER_FACE * shapes[s].mesh.num_face_vertices.size());
			model.meshes[s].textureUVs.resize(2 * VERTICES_PER_FACE * shapes[s].mesh.num_face_vertices.size());
			model.meshes[s].indices.resize   (3                     * shapes[s].mesh.num_face_vertices.size());

			const std::string ambient  = materials[shapes[s].mesh.material_ids[0]].ambient_texname;
			const std::string diffuse  = materials[shapes[s].mesh.material_ids[0]].diffuse_texname;
			const std::string specular = materials[shapes[s].mesh.material_ids[0]].specular_texname;
			const std::string normal   = materials[shapes[s].mesh.material_ids[0]].normal_texname;
			const std::string emissive = materials[shapes[s].mesh.material_ids[0]].emissive_texname;

			model.meshes[s].diffuse = (ambient.empty() ? "" : texturePathPrepend) / ambient;
			model.meshes[s].specular = "";
			model.meshes[s].normal = "";
			model.meshes[s].emissive = "";
			model.meshes[s].metallicRoughness = "";

			model.meshes[s].isDoubleSided = false;
			model.meshes[s].isTransparent = false;

			for (size_t f = 0, fSize = shapes[s].mesh.num_face_vertices.size(); f < fSize; f++)
			{
				// We are going to assume that all faces are triangles
				for (size_t v = 0; v < VERTICES_PER_FACE; v++)
				{
					const rapidobj::Index& index = shapes[s].mesh.indices[indexOffset + v];
					const size_t idx = f * VERTICES_PER_FACE + v;
					// Positions
					model.meshes[s].vertices[idx * 3 + 0] = attrib.positions[index.position_index * 3 + 0];
					model.meshes[s].vertices[idx * 3 + 1] = attrib.positions[index.position_index * 3 + 1];
					model.meshes[s].vertices[idx * 3 + 2] = attrib.positions[index.position_index * 3 + 2];
					// Normals
					model.meshes[s].normals[idx * 3 + 0] = attrib.normals[index.normal_index * 3 + 0];
					model.meshes[s].normals[idx * 3 + 1] = attrib.normals[index.normal_index * 3 + 1];
					model.meshes[s].normals[idx * 3 + 2] = attrib.normals[index.normal_index * 3 + 2];
					// Texture UVs
					model.meshes[s].textureUVs[idx * 2 + 0] = attrib.texcoords[index.texcoord_index * 2 + 0];
					model.meshes[s].textureUVs[idx * 2 + 1] = attrib.texcoords[index.texcoord_index * 2 + 1];
					// Indices
					model.meshes[s].indices[idx] = indexOffset + v;
				}
				indexOffset += VERTICES_PER_FACE;
			}
		}
		return model;
	}
}