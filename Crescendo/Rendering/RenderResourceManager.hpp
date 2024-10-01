#pragma once
#include "common.hpp"
#include "cs_std/slot_map.hpp"
#include "ResourceHandles.hpp"
#include "Vulkan/raii/Device.hpp"
#include "Vulkan/raii/CommandQueue.hpp"
#include "Vulkan/raii/Buffer.hpp"
#include "Vulkan/raii/Image.hpp"
#include "cs_std/image.hpp"
#include "Vulkan/raii/Framebuffer.hpp"
#include "Vulkan/raii/RenderPass.hpp"
#include "Vulkan/raii/Pipeline.hpp"
#include "Vulkan/raii/Sampler.hpp"

CS_NAMESPACE_BEGIN
{
	class RenderResourceManager
	{
	private:
		Vulkan::Vk::Device* device;
		Vulkan::Vk::TransferCommandQueue transferQueue;

		cs_std::slotmap<Vulkan::Vk::Buffer> ssboBuffers;
		cs_std::slotmap<Vulkan::Vk::Buffer> gpuBuffers;
		cs_std::slotmap<Vulkan::Vk::Image> textures;
		cs_std::slotmap<Vulkan::Vk::Framebuffer> framebuffers;
		cs_std::slotmap<Vulkan::Vk::RenderPass> renderpasses;
		cs_std::slotmap<Vulkan::Vk::Pipeline> pipelines;

		// Fixed list of samplers
		std::vector<Vulkan::Vk::Sampler> samplers;

		VkDescriptorSetLayout layout;
		VkDescriptorPool pool;
		VkDescriptorSet set;
	public:
		enum class GPUBufferUsage : uint8_t { VertexBuffer = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, IndexBuffer = VK_BUFFER_USAGE_INDEX_BUFFER_BIT, StorageBuffer = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT };
		enum class Colorspace : uint8_t { SRGB, Linear };
		enum class Filter : uint8_t { Nearest = VK_FILTER_NEAREST, Linear = VK_FILTER_LINEAR };
		enum class WrapMode : uint8_t { Repeat = VK_SAMPLER_ADDRESS_MODE_REPEAT, MirroredRepeat = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, ClampToEdge = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE };
		struct TextureSpecification
		{
			float anisotropy;
			Colorspace colorspace;
			Filter minFilter, magFilter;
			WrapMode wrapMode;
			bool generateMipmaps;
			TextureSpecification(Colorspace colorspace = Colorspace::SRGB, Filter minFilter = Filter::Linear, Filter magFilter = Filter::Linear, WrapMode wrapMode = WrapMode::Repeat, float anisotropy = 1.0f, bool generateMipmaps = false)
				: colorspace(colorspace), minFilter(minFilter), magFilter(magFilter), wrapMode(wrapMode), anisotropy(anisotropy), generateMipmaps(generateMipmaps) {}
		};
	public:
		RenderResourceManager() = default;
		RenderResourceManager(Vulkan::Vk::Device& device, uint32_t maxBuffers, uint32_t maxImages);
		~RenderResourceManager();
		RenderResourceManager(const RenderResourceManager&) = delete;
		RenderResourceManager& operator=(const RenderResourceManager&) = delete;
		RenderResourceManager(RenderResourceManager&& other) noexcept;
		RenderResourceManager& operator=(RenderResourceManager&& other) noexcept;
	public:
		// Returns a handle to the resource

		// CPU visible buffer used for shaders, can be written to later
		[[nodiscard]] Vulkan::SSBOBufferHandle CreateSSBOBuffer(VkDeviceSize size, void* data = nullptr);
		// GPU buffer, must be written to in advance, usually for vertex / index buffers
		[[nodiscard]] Vulkan::GPUBufferHandle CreateGPUBuffer(void* data, VkDeviceSize size, GPUBufferUsage usageFlags);
		[[nodiscard]] Vulkan::TextureHandle CreateTexture(const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationCreateInfo, const TextureSpecification& textureSpec = {});
		[[nodiscard]] Vulkan::TextureHandle UploadTexture(const cs_std::image& image, const TextureSpecification& textureSpec = {});
		[[nodiscard]] Vulkan::FramebufferHandle CreateFramebuffer(VkRenderPass renderPass, const std::vector<VkImageView>& attachments, VkExtent2D extent);
		[[nodiscard]] Vulkan::RenderPassHandle CreateRenderPass(const VkRenderPassCreateInfo& createInfo);
		[[nodiscard]] Vulkan::PipelineHandle CreatePipeline(const Vulkan::Vk::Pipeline::PipelineCreateInfo& createInfo);
	public:
		// Reserve a handle for a resource, which can be updated later

		[[nodiscard]] Vulkan::SSBOBufferHandle ReserveSSBOBuffer();
		[[nodiscard]] Vulkan::GPUBufferHandle ReserveGPUBuffer();
		[[nodiscard]] Vulkan::TextureHandle ReserveTexture();
		[[nodiscard]] Vulkan::FramebufferHandle ReserveFramebuffer();
		[[nodiscard]] Vulkan::RenderPassHandle ReserveRenderPass();
		[[nodiscard]] Vulkan::PipelineHandle ReservePipeline();
	public:
		bool IsValidSSBOBuffer(const Vulkan::SSBOBufferHandle& handle) const;
		bool IsValidGPUBuffer(const Vulkan::GPUBufferHandle& handle) const;
		bool IsValidTexture(const Vulkan::TextureHandle& handle) const;
		bool IsValidFramebuffer(const Vulkan::FramebufferHandle& handle) const;
		bool IsValidRenderPass(const Vulkan::RenderPassHandle& handle) const;
		bool IsValidPipeline(const Vulkan::PipelineHandle& handle) const;
	public:
		// Ignores switches between SRGB and Linear and will not remipmap, but will change the sampler
		void ChangeTextureSampler(const Vulkan::TextureHandle& handle, const TextureSpecification& textureSpec);
	public:
		// Destroy the resource, destroys immediately, does not care if the resource is in use. If the handle is invalid, it will do nothing

		void DestroySSBOBuffer(const Vulkan::SSBOBufferHandle& handle);
		void DestroyGPUBuffer(const Vulkan::GPUBufferHandle& handle);
		void DestroyTexture(const Vulkan::TextureHandle& handle);
		void DestroyFramebuffer(const Vulkan::FramebufferHandle& handle);
		void DestroyRenderPass(const Vulkan::RenderPassHandle& handle);
		void DestroyPipeline(const Vulkan::PipelineHandle& handle);
	public:
		// Get the underlying resource from the handle

		Vulkan::Vk::Buffer& GetSSBOBuffer(const Vulkan::SSBOBufferHandle& handle);
		Vulkan::Vk::Buffer& GetGPUBuffer(const Vulkan::GPUBufferHandle& handle);
		Vulkan::Vk::Image& GetTexture(const Vulkan::TextureHandle& handle);
		Vulkan::Vk::Framebuffer& GetFramebuffer(const Vulkan::FramebufferHandle& handle);
		Vulkan::Vk::RenderPass& GetRenderPass(const Vulkan::RenderPassHandle& handle);
		Vulkan::Vk::Pipeline& GetPipeline(const Vulkan::PipelineHandle& handle);
	public:
		size_t GetSSBOBufferCount() const;
		size_t GetGPUBufferCount() const;
		size_t GetTextureCount() const;
		size_t GetFramebufferCount() const;
		size_t GetRenderPassCount() const;
		size_t GetPipelineCount() const;
		VkDescriptorSetLayout GetDescriptorSetLayout() const;
		VkDescriptorSet GetDescriptorSet() const;
	private:
		static VkFormat GetFormatFromColorSpace(Colorspace colorSpace, uint16_t channels);
		VkSampler GetSamplerFromSpecification(const TextureSpecification& textureSpec) const noexcept;
	};
}