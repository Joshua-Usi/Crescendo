#include "ModelLoader.hpp"

#include "rapidobj/rapidobj.hpp"

#include "Core/common.hpp"

namespace Crescendo::IO
{
	cs_std::graphics::model LoadOBJ(const std::filesystem::path& path)
	{
		constexpr uint32_t VERTICES_PER_FACE = 3;
		const std::filesystem::path texturePathPrepend = path.parent_path();

		cs_std::graphics::model model;

		rapidobj::Result result = rapidobj::ParseFile(path);
		rapidobj::Triangulate(result);

		if (result.error) CS_ASSERT(false, "Failed to load obj: " + path.string());

		auto& attrib = result.attributes;
		auto& shapes = result.shapes;
		auto& materials = result.materials;
		
		for (const auto& shape : shapes)
		{
			cs_std::graphics::mesh mesh;
			cs_std::graphics::mesh_attributes attributes;

			size_t indexOffset = 0;

			mesh.add_attribute(cs_std::graphics::Attribute::POSITION, std::vector<float>(3 * VERTICES_PER_FACE * shape.mesh.num_face_vertices.size()));
			mesh.add_attribute(cs_std::graphics::Attribute::NORMAL, std::vector<float>(3 * VERTICES_PER_FACE * shape.mesh.num_face_vertices.size()));
			mesh.add_attribute(cs_std::graphics::Attribute::TEXCOORD_0, std::vector<float>(2 * VERTICES_PER_FACE * shape.mesh.num_face_vertices.size()));

			mesh.indices.resize(3 * shape.mesh.num_face_vertices.size());

			auto& vertices = mesh.get_attribute(cs_std::graphics::Attribute::POSITION).data;
			auto& normals = mesh.get_attribute(cs_std::graphics::Attribute::NORMAL).data;
			auto& textureUVs = mesh.get_attribute(cs_std::graphics::Attribute::TEXCOORD_0).data;
			auto& indices = mesh.indices;

			const std::string& ambient   = materials[shape.mesh.material_ids[0]].ambient_texname;
			const std::string& diffuse   = materials[shape.mesh.material_ids[0]].diffuse_texname;
			const std::string& specular  = materials[shape.mesh.material_ids[0]].specular_texname;
			const std::string& normal    = materials[shape.mesh.material_ids[0]].normal_texname;
			const std::string& emissive  = materials[shape.mesh.material_ids[0]].emissive_texname;
			const std::string& metallic  = materials[shape.mesh.material_ids[0]].metallic_texname;
			const std::string& roughness = materials[shape.mesh.material_ids[0]].roughness_texname;

			attributes.transform = glm::mat4(1.0f);

			attributes.diffuse =   (ambient.empty()   ? "" : texturePathPrepend) / ambient;
			attributes.normal =    (normal.empty()    ? "" : texturePathPrepend) / normal;
			attributes.emissive =  (emissive.empty()  ? "" : texturePathPrepend) / emissive;
			attributes.metallic =  (metallic.empty()  ? "" : texturePathPrepend) / metallic;
			attributes.roughness = (roughness.empty() ? "" : texturePathPrepend) / roughness;
			attributes.metallicRoughness = "";

			attributes.isDoubleSided = false;
			attributes.isTransparent = false;

			for (uint32_t f = 0, fSize = shape.mesh.num_face_vertices.size(); f < fSize; f++)
			{
				// We are going to assume that all faces are triangles
				for (uint32_t v = 0; v < VERTICES_PER_FACE; v++)
				{
					const rapidobj::Index& index = shape.mesh.indices[indexOffset + v];
					const uint32_t idx = f * VERTICES_PER_FACE + v;
					// Positions
					vertices[idx * 3 + 0] = attrib.positions[index.position_index * 3 + 0];
					vertices[idx * 3 + 1] = attrib.positions[index.position_index * 3 + 1];
					vertices[idx * 3 + 2] = attrib.positions[index.position_index * 3 + 2];
					// Normals
					normals[idx * 3 + 0] = attrib.normals[index.normal_index * 3 + 0];
					normals[idx * 3 + 1] = attrib.normals[index.normal_index * 3 + 1];
					normals[idx * 3 + 2] = attrib.normals[index.normal_index * 3 + 2];
					// Texture UVs
					textureUVs[idx * 2 + 0] = attrib.texcoords[index.texcoord_index * 2 + 0];
					textureUVs[idx * 2 + 1] = attrib.texcoords[index.texcoord_index * 2 + 1];
					// Indices
					indices[idx] = indexOffset + v;
				}
				indexOffset += VERTICES_PER_FACE;
			}
			model.meshes.push_back(mesh);
			model.meshAttributes.push_back(attributes);
		}
		return model;
	}
}