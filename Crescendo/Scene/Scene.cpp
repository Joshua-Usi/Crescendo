#include "Scene.hpp"
#include "Core/Application.hpp"
#include "Assets/ImageLoaders.hpp"

CS_NAMESPACE_BEGIN
{
	void Scene::LoadModels(std::vector<cs_std::graphics::model>& models)
	{
		{
			auto* app = Application::Get();
			auto& resourceManager = app->resourceManager;
			auto& taskQueue = app->taskQueue;

			uint32_t textureIndex = 0;

			struct TextureInfo { uint32_t textureIdx; RenderResourceManager::Colorspace colorspace; };

			struct EntityTextureInfo
			{
				Entity entity;
				uint32_t expectedDiffuseTextureIdx, expectedNormalTextureIdx;
			};

			std::unordered_map<std::filesystem::path, TextureInfo> textureMap;
			std::vector<Vulkan::TextureHandle> textures;
			std::vector<EntityTextureInfo> entityTextureInfo;

			for (auto& model : models)
			{
				for (size_t i = 0; i < model.meshes.size(); i++)
				{
					auto& mesh = model.meshes[i];
					auto& attributes = model.meshAttributes[i];

					if (!mesh.has_attribute(cs_std::graphics::Attribute::TANGENT)) cs_std::graphics::generate_tangents(mesh);
					Vulkan::Mesh meshHandle;
					meshHandle.vertexAttributes.emplace_back(
						resourceManager.CreateGPUBuffer(
							mesh.get_attribute(cs_std::graphics::Attribute::POSITION).data.data(),
							mesh.get_attribute(cs_std::graphics::Attribute::POSITION).data.size() * sizeof(float),
							RenderResourceManager::GPUBufferUsage::VertexBuffer
						),
						cs_std::graphics::Attribute::POSITION
					);
					meshHandle.vertexAttributes.emplace_back(
						resourceManager.CreateGPUBuffer(
							mesh.get_attribute(cs_std::graphics::Attribute::NORMAL).data.data(),
							mesh.get_attribute(cs_std::graphics::Attribute::NORMAL).data.size() * sizeof(float),
							RenderResourceManager::GPUBufferUsage::VertexBuffer
						),
						cs_std::graphics::Attribute::NORMAL
					);
					meshHandle.vertexAttributes.emplace_back(
						resourceManager.CreateGPUBuffer(
							mesh.get_attribute(cs_std::graphics::Attribute::TANGENT).data.data(),
							mesh.get_attribute(cs_std::graphics::Attribute::TANGENT).data.size() * sizeof(float),
							RenderResourceManager::GPUBufferUsage::VertexBuffer
						),
						cs_std::graphics::Attribute::TANGENT
					);
					meshHandle.vertexAttributes.emplace_back(
						resourceManager.CreateGPUBuffer(
							mesh.get_attribute(cs_std::graphics::Attribute::TEXCOORD_0).data.data(),
							mesh.get_attribute(cs_std::graphics::Attribute::TEXCOORD_0).data.size() * sizeof(float),
							RenderResourceManager::GPUBufferUsage::VertexBuffer
						),
						cs_std::graphics::Attribute::TEXCOORD_0
					);
					meshHandle.indexBuffer = resourceManager.CreateGPUBuffer(
						mesh.indices.data(),
						mesh.indices.size() * sizeof(uint32_t),
						RenderResourceManager::GPUBufferUsage::IndexBuffer
					);
					meshHandle.indexCount = mesh.indices.size();
					meshHandle.indexType = VK_INDEX_TYPE_UINT32;

					if (!attributes.diffuse.empty() && textureMap.find(attributes.diffuse) == textureMap.end())
					{
						textureMap[attributes.diffuse].textureIdx = textureIndex;
						textureMap[attributes.diffuse].colorspace = RenderResourceManager::Colorspace::SRGB;
						textureIndex++;
					}
					if (!attributes.normal.empty() && textureMap.find(attributes.normal) == textureMap.end())
					{
						textureMap[attributes.normal].textureIdx = textureIndex;
						textureMap[attributes.normal].colorspace = RenderResourceManager::Colorspace::Linear;
						textureIndex++;
					}

					Entity entity = entityManager.CreateEntity();
					entity.EmplaceComponent<Transform>(attributes.transform);
					entity.EmplaceComponent<MeshData>(cs_std::graphics::bounding_aabb(mesh.get_attribute(cs_std::graphics::Attribute::POSITION).data), meshHandle);
					entity.EmplaceComponent<Material>(Vulkan::PipelineHandle(), Vulkan::TextureHandle(), Vulkan::TextureHandle(), attributes.isTransparent, attributes.isDoubleSided, !attributes.isTransparent);

					EntityTextureInfo info;
					info.entity = entity;
					info.expectedDiffuseTextureIdx = textureMap.find(attributes.diffuse) != textureMap.end() ? textureMap[attributes.diffuse].textureIdx : 0;
					info.expectedNormalTextureIdx = textureMap.find(attributes.normal) != textureMap.end() ? textureMap[attributes.normal].textureIdx : 0;

					entityTextureInfo.push_back(info);
				}
			}

			std::vector<std::string> textureStrings(textureMap.size());
			for (const auto& [path, info] : textureMap) textureStrings[info.textureIdx] = path.string();

			struct TaggedImage { cs_std::image image; RenderResourceManager::Colorspace colorspace; };

			std::vector<cs_std::image> images(textureStrings.size());
			for (uint32_t i = 0; i < textureStrings.size(); i++)
			{
				taskQueue.push_back([&images, &textureStrings, i]() { images[i] = LoadImage(textureStrings[i]); });
			}

			uint32_t last = 0;
			while (!taskQueue.finished())
			{
				uint32_t local = textureStrings.size() - taskQueue.pending_task_count();
				for (uint32_t i = 0; i < local - last; i++) cs_std::console::raw("#");
				last = local;
			}
			cs_std::console::raw('\n');
			taskQueue.sleep();

			for (uint32_t i = 0; i < images.size(); i++)
			{
				auto& image = images[i];
				Vulkan::TextureHandle handle = resourceManager.UploadTexture(image, {
						textureMap[textureStrings[i]].colorspace, RenderResourceManager::Filter::Linear,
						RenderResourceManager::Filter::Linear, RenderResourceManager::WrapMode::Repeat,
						1.0f, true
					});
				textures.push_back(handle);
			}

			for (auto& info : entityTextureInfo)
			{
				Material& material = info.entity.GetComponent<Material>();
				if (info.expectedDiffuseTextureIdx != -1) material.diffuseHandle = textures[info.expectedDiffuseTextureIdx];
				if (info.expectedNormalTextureIdx != -1) material.normalHandle = textures[info.expectedNormalTextureIdx];

			}
		}
	}
}