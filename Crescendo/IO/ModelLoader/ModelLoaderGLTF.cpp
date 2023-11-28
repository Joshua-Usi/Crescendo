#include "ModelLoaders.hpp"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"

#define TINYGLTF_NOEXCEPTION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_RAPIDJSON
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_USE_RAPIDJSON
#define TINYGLTF_USE_CPP14
#define TINYGLTF_IMPLEMENTATION
#include "tinygltf/tiny_gltf.h"

#include <unordered_map>

#include "glm/gtc/quaternion.hpp"

CS_NAMESPACE_BEGIN
{
	struct NodeData
	{
		tinygltf::Node* node;
		glm::mat4 transform;
	};
    size_t ComponentEnumToSize(int componentType)
    {
        switch (componentType)
        {
		    case TINYGLTF_COMPONENT_TYPE_BYTE: return sizeof(char);
		    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: return sizeof(unsigned char);
		    case TINYGLTF_COMPONENT_TYPE_SHORT: return sizeof(short);
		    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return sizeof(unsigned short);
		    case TINYGLTF_COMPONENT_TYPE_INT: return sizeof(int);
		    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: return sizeof(unsigned int);
		    case TINYGLTF_COMPONENT_TYPE_FLOAT: return sizeof(float);
		    case TINYGLTF_COMPONENT_TYPE_DOUBLE: return sizeof(double);
		}
		// Just so that we don't multiply by 0
        return 1;
    }
    uint32_t TypeEnumToSize(int type)
    {
        switch (type)
        {
		    case TINYGLTF_TYPE_SCALAR: return 1;
		    case TINYGLTF_TYPE_VEC2: return 2;
		    case TINYGLTF_TYPE_VEC3: return 3;
		    case TINYGLTF_TYPE_VEC4: return 4;
		    case TINYGLTF_TYPE_MAT2: return 4;
		    case TINYGLTF_TYPE_MAT3: return 9;
		    case TINYGLTF_TYPE_MAT4: return 16;
        }
        // Just so that we don't multiply by 0
        return 1; 
    }
	void traverseNode(tinygltf::Model& model, std::unordered_map<uint32_t, NodeData>& nodeMap, tinygltf::Node& node, glm::mat4 parentTransform = glm::mat4(1.0f))
	{
		const glm::vec3 scale = (node.scale.size() == 0) ? glm::vec3(1.0f, 1.0f, 1.0f) : glm::vec3(
			static_cast<float>(node.scale[0]), static_cast<float>(node.scale[1]), static_cast<float>(node.scale[2])
		);
		// GLM uses WXYZ
		const glm::quat rotation = (node.rotation.size() == 0) ? glm::quat(1.0f, 0.0f, 0.0f, 0.0f) : glm::quat(
			static_cast<float>(node.rotation[3]), static_cast<float>(node.rotation[0]), static_cast<float>(node.rotation[1]), static_cast<float>(node.rotation[2])
		);
		const glm::vec3 translate = (node.translation.size() == 0) ? glm::vec3(0.0f, 0.0f, 0.0f) : glm::vec3(
			static_cast<float>(node.translation[0]), static_cast<float>(node.translation[1]), static_cast<float>(node.translation[2])
		);

		glm::mat4 childTransform = glm::mat4(1.0f);
		childTransform = glm::translate(childTransform, translate);
		childTransform = childTransform * glm::mat4_cast(rotation);
		childTransform = glm::scale(childTransform, scale);

		const glm::mat4 finalTransform = parentTransform * childTransform;

		nodeMap[node.mesh].node = &node;
		nodeMap[node.mesh].transform = finalTransform;

		for (const int child : node.children) traverseNode(model, nodeMap, model.nodes[child], finalTransform);
	}
	cs_std::graphics::model LoadGLTF(const std::filesystem::path& path)
	{
		const std::filesystem::path texturePathPrepend = path.parent_path();

		cs_std::graphics::model model = {};

		tinygltf::TinyGLTF loader;
		tinygltf::Model gltfModel;
		std::string err, warn;

		if (!loader.LoadASCIIFromFile(&gltfModel, &err, &warn, path.string()))
		{
			cs_std::console::error("Failed to load GLTF file: ", path.string(), ". Reason: ", err);
		}

		std::unordered_map<uint32_t, NodeData> nodeToMesh;
		for (auto& node : gltfModel.scenes[gltfModel.defaultScene].nodes) traverseNode(gltfModel, nodeToMesh, gltfModel.nodes[node]);

		for (uint32_t i = 0; i < gltfModel.meshes.size(); i++)
		{

			const glm::mat4& transform = nodeToMesh[i].transform;
			for (const auto& primitive : gltfModel.meshes[i].primitives)
			{
				cs_std::graphics::mesh mesh = {};
				cs_std::graphics::mesh_attributes attributes {};

				attributes.transform = transform;

				if (primitive.indices >= 0)
				{
					const auto& accessor = gltfModel.accessors[primitive.indices];
					const auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
					const auto& buffer = gltfModel.buffers[bufferView.buffer];

					// Convert from unsigned short to unsigned int
					if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
					{
						for (size_t i = 0; i < accessor.count; i++)
						{
							mesh.indices.push_back(static_cast<uint32_t>(*(reinterpret_cast<const uint16_t*>(buffer.data.data() + bufferView.byteOffset + accessor.byteOffset) + i)));
						}
					}
					// Fast path if we don't need to convert
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
					{
						mesh.indices.insert(mesh.indices.end(),
							reinterpret_cast<const uint32_t*>(buffer.data.data() + bufferView.byteOffset + accessor.byteOffset),
							reinterpret_cast<const uint32_t*>(buffer.data.data() + bufferView.byteOffset + accessor.byteOffset + accessor.count * TypeEnumToSize(accessor.type) * ComponentEnumToSize(accessor.componentType))
						);
					}
				}
				for (const auto& attribute : primitive.attributes)
				{
					const auto& accessor = gltfModel.accessors[attribute.second];
					const auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
					const auto& buffer = gltfModel.buffers[bufferView.buffer];

					cs_std::graphics::Attribute type = cs_std::graphics::Attribute::UNKNOWN;

					if (attribute.first == "POSITION") type = cs_std::graphics::Attribute::POSITION;
					if (attribute.first == "NORMAL") type = cs_std::graphics::Attribute::NORMAL;
					if (attribute.first == "TANGENT") type = cs_std::graphics::Attribute::TANGENT;
					if (attribute.first == "TEXCOORD_0") type = cs_std::graphics::Attribute::TEXCOORD_0;
					if (attribute.first == "TEXCOORD_1") type = cs_std::graphics::Attribute::TEXCOORD_1;
					if (attribute.first == "COLOR_0") type = cs_std::graphics::Attribute::COLOR_0;
					if (attribute.first == "JOINTS_0") type = cs_std::graphics::Attribute::JOINTS_0;
					if (attribute.first == "WEIGHTS_0") type = cs_std::graphics::Attribute::WEIGHTS_0;

					mesh.add_attribute(type, std::vector<float>(
						reinterpret_cast<const float*>(buffer.data.data() + bufferView.byteOffset + accessor.byteOffset),
						reinterpret_cast<const float*>(buffer.data.data() + bufferView.byteOffset + accessor.byteOffset + accessor.count * TypeEnumToSize(accessor.type) * ComponentEnumToSize(accessor.componentType))
					));
				}
				if (primitive.material >= 0)
				{
					const auto& gltfMaterial = gltfModel.materials[primitive.material];
					attributes.isDoubleSided = gltfMaterial.doubleSided;
					attributes.isTransparent = gltfMaterial.alphaMode == "BLEND";
					for (const auto& texture : gltfMaterial.values)
					{
						const int	& baseColorTextureIndex			= gltfMaterial.pbrMetallicRoughness.baseColorTexture.index,
									& metallicRoughnessTextureIndex	= gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index,
									& normalTextureIndex			= gltfMaterial.normalTexture.index,
									& occlusionTextureIndex			= gltfMaterial.occlusionTexture.index,
									& emissiveTextureIndex			= gltfMaterial.emissiveTexture.index;

						if (baseColorTextureIndex			!= -1) attributes.diffuse			= texturePathPrepend / gltfModel.images[baseColorTextureIndex].uri;
						if (metallicRoughnessTextureIndex	!= -1) attributes.metallicRoughness	= texturePathPrepend / gltfModel.images[metallicRoughnessTextureIndex].uri;
						if (normalTextureIndex				!= -1) attributes.normal			= texturePathPrepend / gltfModel.images[normalTextureIndex].uri;
						if (occlusionTextureIndex			!= -1) attributes.occlusion			= texturePathPrepend / gltfModel.images[occlusionTextureIndex].uri;
						if (emissiveTextureIndex			!= -1) attributes.emissive			= texturePathPrepend / gltfModel.images[emissiveTextureIndex].uri;
					}
				}
				model.meshes.push_back(mesh);
				model.meshAttributes.push_back(attributes);
			}
		}
		return model;
	}
}