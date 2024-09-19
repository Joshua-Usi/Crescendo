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

#include "ResourceHandles.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{

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
		enum class Filter : uint8_t { Nearest = VK_FILTER_NEAREST, Linear = VK_FILTER_LINEAR};
		enum class WrapMode : uint8_t { Repeat = VK_SAMPLER_ADDRESS_MODE_REPEAT, MirroredRepeat = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, ClampToEdge = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE };
		struct TextureSpecification
		{
			float anisotropy;
			Colorspace colorspace;
			Filter minFilter, magFilter;
			WrapMode wrapMode;
			bool generateMipmaps;
			TextureSpecification(Colorspace colorspace = Colorspace::SRGB, Filter minFilter = Filter::Linear, Filter magFilter = Filter::Linear, WrapMode wrapMode = WrapMode::Repeat, float anisotropy = 1.0f, bool generateMipmaps = true)
				: colorspace(colorspace), minFilter(minFilter), magFilter(magFilter), wrapMode(wrapMode), anisotropy(anisotropy), generateMipmaps(generateMipmaps) {}
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
		[[nodiscard]] TextureHandle CreateTexture(const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationCreateInfo, const TextureSpecification& textureSpec = {});
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
	private:
		VkFormat GetFormatFromColorSpace(ResourceManager::Colorspace colorSpace, uint16_t channels);
		VkSampler GetSamplerFromSpecification(const TextureSpecification& textureSpec);
	};
}