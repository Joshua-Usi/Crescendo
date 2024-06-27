#pragma once

#include "common.hpp"
#include "vulkan/vulkan.h"
#include "raii/Buffer.hpp"
#include "raii/Image.hpp"
#include "raii/CommandQueue.hpp"
#include "raii/Sampler.hpp"
#include "cs_std/slot_map.hpp"
#include "cs_std/graphics/model.hpp"
#include "Device.hpp"
#include "cs_std/image.hpp"
#include "BindlessDescriptorManager.hpp"

#define CS_RESOURCE_MANAGER_CREATE_HANDLE(type, name)\
class name\
{\
private:\
	cs_std::slotmap_key64<type> key;\
public:\
	name() = default;\
	name(cs_std::slotmap_key64<type> key) : key(key) {}\
	operator cs_std::slotmap_key64<type>() const { return key; }\
};

CS_NAMESPACE_BEGIN::Vulkan
{
	struct Buffer
	{
		Vk::Buffer buffer;
		BindlessDescriptorManager::BufferHandle handle;
		Buffer() = default;
		Buffer(Vk::Buffer&& Buffer, BindlessDescriptorManager::BufferHandle handle) : buffer(std::move(buffer)), handle(handle) {}
	};

	struct Texture
	{
		Vk::Image image;
		VkSampler sampler;
		BindlessDescriptorManager::ImageHandle handle;
		Texture() = default;
		Texture(Vk::Image&& image, VkSampler sampler, BindlessDescriptorManager::ImageHandle handle) : image(std::move(image)), sampler(sampler), handle(handle) {}
	};

	struct Mesh
	{
		struct Attribute { Vk::Buffer buffer; uint32_t elements; cs_std::graphics::Attribute attribute; };
		std::vector<Attribute> vertexAttributes;
		Vk::Buffer indexBuffer;
		uint32_t indexCount;
		VkIndexType indexType;
		Mesh(std::vector<Attribute>&& vertexAttributes = {}, Vk::Buffer&& indexBuffer = {}, uint32_t indexCount = 0, VkIndexType indexType = VK_INDEX_TYPE_UINT32) : vertexAttributes(std::move(vertexAttributes)), indexBuffer(std::move(indexBuffer)), indexCount(indexCount), indexType(indexType) {}
	};

	CS_RESOURCE_MANAGER_CREATE_HANDLE(Buffer, BufferHandle);
	CS_RESOURCE_MANAGER_CREATE_HANDLE(Texture, TextureHandle);
	CS_RESOURCE_MANAGER_CREATE_HANDLE(Mesh, MeshHandle);

	class ResourceManager
	{
	private:
		Device* device;
		Vk::TransferCommandQueue transferQueue;
		cs_std::slotmap<Buffer> buffers;
		cs_std::slotmap<Texture> textures;
		cs_std::slotmap<Mesh> meshes;
		std::vector<Vk::Sampler> samplers;
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
		ResourceManager(Device& device);
		~ResourceManager() = default;
		ResourceManager(const ResourceManager&) = delete;
		ResourceManager& operator=(const ResourceManager&) = delete;
		ResourceManager(ResourceManager&& other) noexcept;
		ResourceManager& operator=(ResourceManager&& other) noexcept;
	public:
		// These methods will block the thread until task is complete
		// TODO add an async method
		[[nodiscard]] BufferHandle CreateBuffer(VkDeviceSize size, VkShaderStageFlags shaderStage);
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
		size_t GetBufferCount() const;
		size_t GetTextureCount() const;
		size_t GetMeshCount() const;
	};
}