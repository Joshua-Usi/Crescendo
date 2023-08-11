#include "ModelLoader.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobj/tiny_obj_loader.h"

#include <iostream>

namespace Crescendo::IO
{
	Model LoadOBJ(const std::filesystem::path& path)
	{
		Model model;

		tinyobj::ObjReader reader;

		if (!reader.ParseFromFile(path.string().c_str()) && !reader.Error().empty())
		{
			std::cout << "TinyOBJ Error: " << reader.Error() << std::endl;
			return Model();
		}

		auto& attrib = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		auto& materials = reader.GetMaterials();

		for (size_t s = 0, sSize = shapes.size(); s < sSize; s++)
		{
			Mesh mesh;
			size_t indexOffset = 0;
			for (size_t f = 0, fSize = shapes[s].mesh.num_face_vertices.size(); f < fSize; f++)
			{
				constexpr uint32_t VERTICES_PER_FACE = 3;
				for (size_t v = 0; v < VERTICES_PER_FACE; v++)
				{
					const tinyobj::index_t& index = shapes[s].mesh.indices[indexOffset + v];
					// Positions
					mesh.vertices.push_back(attrib.vertices[index.vertex_index * 3 + 0]);
					mesh.vertices.push_back(attrib.vertices[index.vertex_index * 3 + 1]);
					mesh.vertices.push_back(attrib.vertices[index.vertex_index * 3 + 2]);
					// Normals
					mesh.normals.push_back(attrib.normals[index.normal_index * 3 + 0]);
					mesh.normals.push_back(attrib.normals[index.normal_index * 3 + 1]);
					mesh.normals.push_back(attrib.normals[index.normal_index * 3 + 2]);
					// Texture UVs
					mesh.textureUVs.push_back(attrib.texcoords[index.texcoord_index * 2 + 0]);
					mesh.textureUVs.push_back(attrib.texcoords[index.texcoord_index * 2 + 1]);
					// Indices
					mesh.indices.push_back(indexOffset + v);
				}
				indexOffset += VERTICES_PER_FACE;
			}
			model.meshes.push_back(mesh);
		}
		return model;
	}
}