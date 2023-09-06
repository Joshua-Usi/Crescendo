#include "ModelLoader.hpp"

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

namespace Crescendo::IO
{
	Model LoadGLTF(const std::filesystem::path& path, const std::filesystem::path& texturePathPrepend)
	{
		Model model = {};

		tinygltf::TinyGLTF loader;
		tinygltf::Model gltfModel;
		std::string err, warn;

		bool ret = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, path.string());

		if (!ret) return model;

		for (const auto& gltfMesh : gltfModel.meshes)
		{
			Model::Mesh mesh = {};
            for (const auto& primitive : gltfMesh.primitives)
            {
                if (primitive.indices >= 0)
                {
                    const auto& bufferView = gltfModel.bufferViews[gltfModel.accessors[primitive.indices].bufferView];
                    const auto& buffer = gltfModel.buffers[bufferView.buffer];
                    mesh.indices.insert(mesh.indices.end(),
                        reinterpret_cast<const uint32_t*>(&buffer.data[bufferView.byteOffset]),
                        reinterpret_cast<const uint32_t*>(&buffer.data[bufferView.byteOffset + bufferView.byteLength]));
                }

                for (const auto& attribute : primitive.attributes)
                {
                    const auto& accessor = gltfModel.accessors[attribute.second];
                    const auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
                    const auto& buffer = gltfModel.buffers[bufferView.buffer];
                    if (attribute.first == "POSITION")
                    {
                        mesh.vertices.insert(mesh.vertices.end(),
                            reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset]),
                            reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + bufferView.byteLength]));
                    }
                    else if (attribute.first == "NORMAL")
                    {
                        mesh.normals.insert(mesh.normals.end(),
                            reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset]),
                            reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + bufferView.byteLength]));
                    }
                    else if (attribute.first == "TEXCOORD_0")
                    {
                        mesh.textureUVs.insert(mesh.textureUVs.end(),
                            reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset]),
                            reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + bufferView.byteLength]));
                    }
                }

                if (primitive.material >= 0)
                {
                    const auto& gltfMaterial = gltfModel.materials[primitive.material];
                    for (const auto& texture : gltfMaterial.values)
                    {
                        if (texture.first == "baseColorTexture")
                        {
                            mesh.albedo = texturePathPrepend.string() + gltfModel.images[texture.second.TextureIndex()].uri;
                        }
                    }
                }
            }
            model.meshes.push_back(mesh);
        }



		return model;
	}
}