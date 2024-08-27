#pragma once

#include "common.hpp"
#include "vulkan/vulkan.h"
#include "raii/Buffer.hpp"
#include "raii/Device.hpp"
#include "raii/Image.hpp"
#include "raii/CommandQueue.hpp"
#include "raii/Sampler.hpp"
#include "cs_std/slot_map.hpp"
#include "cs_std/graphics/model.hpp"
#include "cs_std/image.hpp"

#define CS_RESOURCE_MANAGER_CREATE_HANDLE(type, name)\
class name\
{\
private:\
	union k\
	{\
		/* If set to 4,293,967,295, the key will be invalid */\
		uint32_t index;\
		cs_std::slotmap_key<type> key; \
	} key;\
public:\
	name() { key.index = 0xFFFFFFFF; }\
	name(cs_std::slotmap_key<type> k) { key.key = k; }\
	operator cs_std::slotmap_key<type>() const { return key.key; }\
	uint32_t GetIndex() const { return cs_std::slotmap_key<type>::toIndex(key.key); }\
	bool IsValid() const { return key.index != 0xFFFFFFFF; }\
}

CS_NAMESPACE_BEGIN::Vulkan
{
	struct Buffer
	{
		Vk::Buffer buffer;
		Buffer() = default;
		Buffer(Vk::Buffer&& b) : buffer(std::move(b)) {}
	};

	struct Texture
	{
		Vk::Image image;
		VkSampler sampler;
		Texture() = default;
		Texture(Vk::Image&& i, VkSampler sampler) : image(std::move(i)), sampler(sampler) {}
	};

	struct Mesh
	{
		struct Attribute { Vk::Buffer buffer; uint32_t elements; cs_std::graphics::Attribute attribute; };
		std::vector<Attribute> vertexAttributes;
		Vk::Buffer indexBuffer;
		uint32_t indexCount;
		VkIndexType indexType;
		Mesh(std::vector<Attribute>&& vertexAttributes = {}, Vk::Buffer&& indexBuffer = {}, uint32_t indexCount = 0, VkIndexType indexType = VK_INDEX_TYPE_UINT32) : vertexAttributes(std::move(vertexAttributes)), indexBuffer(std::move(indexBuffer)), indexCount(indexCount), indexType(indexType) {}
		Vk::Buffer* GetAttributeBuffer(cs_std::graphics::Attribute attribute)
		{
			for (auto& vertexAttribute : vertexAttributes)
			{
				if (vertexAttribute.attribute == attribute) return &vertexAttribute.buffer;
			}
			return nullptr;
		}
	};

	CS_RESOURCE_MANAGER_CREATE_HANDLE(Buffer, BufferHandle);
	CS_RESOURCE_MANAGER_CREATE_HANDLE(Texture, TextureHandle);
	CS_RESOURCE_MANAGER_CREATE_HANDLE(Mesh, MeshHandle);

	class ResourceManager
	{
	public:
		struct ResourceManagerSpecification
		{
			uint32_t maxBuffers, maxImages;
		};

	private:
		Vk::Device* device;
		Vk::TransferCommandQueue transferQueue;
		cs_std::slotmap<Buffer> buffers;
		cs_std::slotmap<Texture> textures;
		cs_std::slotmap<Mesh> meshes;
		std::vector<Vk::Sampler> samplers;

		VkDescriptorSetLayout layout;
		VkDescriptorPool pool;
		VkDescriptorSet set;
	public:
		enum class Colorspace : uint8_t { SRGB, Linear };
		struct TextureSpecification
		{
			Colorspace colorspace;
			float anisotropy;
			bool generateMipmaps;
			TextureSpecification(Colorspace colorspace = Colorspace::SRGB, float anisotropy = 1.0f, bool generateMipmaps = false) : colorspace(colorspace), anisotropy(anisotropy), generateMipmaps(generateMipmaps) {}
		};
	public:
		ResourceManager();
		ResourceManager(Vk::Device& device, const ResourceManagerSpecification& resourceManagerSpec);
		~ResourceManager();
		ResourceManager(const ResourceManager&) = delete;
		ResourceManager& operator=(const ResourceManager&) = delete;
		ResourceManager(ResourceManager&& other) noexcept;
		ResourceManager& operator=(ResourceManager&& other) noexcept;
	public:
		// These methods will block the thread until task is complete
		// TODO add an async method
		[[nodiscard]] BufferHandle CreateBuffer(VkDeviceSize size, VkShaderStageFlags shaderStage);
		[[nodiscard]] TextureHandle CreateTexture(const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationCreateInfo);
		[[nodiscard]] TextureHandle UploadTexture(const cs_std::image& image, const TextureSpecification& textureSpec = {}); 
		template<cs_std::graphics::valid_index_type indice_type>
		[[nodiscard]] MeshHandle UploadMesh(const cs_std::graphics::mesh<indice_type>& mesh);
	public:
		Buffer& GetBuffer(BufferHandle handle);
		Texture& GetTexture(TextureHandle handle);
		Mesh& GetMesh(MeshHandle handle);
	public:
		void DestroyBuffer(BufferHandle handle);
		void DestroyTexture(TextureHandle handle);
		void DestroyMesh(MeshHandle handle);
	public:
		void ReplaceBuffer(BufferHandle handle, VkDeviceSize size, VkShaderStageFlags shaderStage);
		void ReplaceTexture(TextureHandle handle, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationCreateInfo);
		void ReplaceAndUploadTexture(TextureHandle handle, const cs_std::image& image, const TextureSpecification& textureSpec = {});
		template<cs_std::graphics::valid_index_type indice_type>
		void ReplaceMesh(MeshHandle handle, const cs_std::graphics::mesh<indice_type>& mesh);
	public:
		size_t GetBufferCount() const;
		size_t GetTextureCount() const;
		size_t GetMeshCount() const;
		VkDescriptorSetLayout GetDescriptorSetLayout() const;
		VkDescriptorSet GetDescriptorSet() const;
	};
}