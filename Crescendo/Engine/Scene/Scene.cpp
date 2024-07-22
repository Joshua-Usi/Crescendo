#include "Scene.hpp"

#include "Engine/Application/Application.hpp"
#include "Assets/ImageLoader/ImageLoaders.hpp"

CS_NAMESPACE_BEGIN
{
	void Scene::LoadModels(std::vector<cs_std::graphics::model>& models)
	{
		{
			auto* app = Application::Get();
			auto& resourceManager = app->resourceManager;
			auto& taskQueue = app->taskQueue;
			auto& bindlessManager = app->instance.GetSurface(0).GetDevice().GetBindlessDescriptorManager();

			uint32_t textureIndex = 0;

			struct TextureInfo { uint32_t textureIdx; Vulkan::ResourceManager::Colorspace colorspace; };

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
					Vulkan::MeshHandle meshHandle = resourceManager.UploadMesh(mesh);

					if (!attributes.diffuse.empty() && textureMap.find(attributes.diffuse) == textureMap.end())
					{
						textureMap[attributes.diffuse].textureIdx = textureIndex;
						textureMap[attributes.diffuse].colorspace = Vulkan::ResourceManager::Colorspace::SRGB;
						textureIndex++;
					}
					if (!attributes.normal.empty() && textureMap.find(attributes.normal) == textureMap.end())
					{
						textureMap[attributes.normal].textureIdx = textureIndex;
						textureMap[attributes.normal].colorspace = Vulkan::ResourceManager::Colorspace::Linear;
						textureIndex++;
					}

					Entity entity = entityManager.CreateEntity();
					entity.EmplaceComponent<Name>("Mesh");
					entity.EmplaceComponent<Transform>(attributes.transform);
					entity.EmplaceComponent<MeshData>(cs_std::graphics::bounding_aabb(mesh.get_attribute(cs_std::graphics::Attribute::POSITION).data), meshHandle);
					entity.EmplaceComponent<Material>(0, Vulkan::TextureHandle(), Vulkan::TextureHandle(), attributes.isTransparent, attributes.isDoubleSided, !attributes.isTransparent);

					EntityTextureInfo info;
					info.entity = entity;
					info.expectedDiffuseTextureIdx = textureMap.find(attributes.diffuse) != textureMap.end() ? textureMap[attributes.diffuse].textureIdx : 0;
					info.expectedNormalTextureIdx = textureMap.find(attributes.normal) != textureMap.end() ? textureMap[attributes.normal].textureIdx : 0;

					entityTextureInfo.push_back(info);
				}
			}

			std::vector<std::filesystem::path> textureStrings(textureMap.size());
			for (const auto& [path, info] : textureMap) textureStrings[info.textureIdx] = path;

			struct TaggedImage { cs_std::image image; Vulkan::ResourceManager::Colorspace colorspace; };

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
				Vulkan::TextureHandle handle = resourceManager.UploadTexture(image, { textureMap[textureStrings[i]].colorspace, 1.0f, true });
				textures.push_back(handle);
				Vulkan::Texture& texture = resourceManager.GetTexture(handle);
				bindlessManager.StoreImage(texture.image, texture.sampler);
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