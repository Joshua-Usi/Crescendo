#include "RenderResourceManager.hpp"
#include "Vulkan/Create.hpp"
#include "Volk/volk.h"

#define RRM_UNIFORM_BINDING 0u
#define RRM_BUFFER_BINDING 1u
#define RRM_IMAGE_BINDING 2u

CS_NAMESPACE_BEGIN
{
	RenderResourceManager::RenderResourceManager(Vulkan::Vk::Device& device, uint32_t maxBuffers, uint32_t maxImages)
		: device(&device), transferQueue(device, device.GetTransferQueue().queue, device.GetTransferQueue().family, false)
	{
		constexpr std::array<VkDescriptorType, 3> types{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		// For simplicity, we treat buffers and uniforms as the same thing, so the true maximum is 2x the maxBuffers
		const std::array<uint32_t, 3> setsSizes = { { maxBuffers, maxBuffers, maxImages } };
		std::array<VkDescriptorSetLayoutBinding, 3> bindings = {};
		std::array<VkDescriptorBindingFlags, 3> flags = {};
		std::array<VkDescriptorPoolSize, 3> sizes = {};
		for (uint8_t i = 0; i < types.size(); i++)
		{
			bindings[i] = Vulkan::Create::DescriptorSetLayoutBinding(i, types[i], setsSizes[i], VK_SHADER_STAGE_ALL);
			flags[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
			sizes[i] = { types[i], setsSizes[i] };
		}
		const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags = Vulkan::Create::DescriptorSetLayoutBindingFlagsCreateInfo(flags);
		const VkDescriptorSetLayoutCreateInfo layoutCreateInfo = Vulkan::Create::DescriptorSetLayoutCreateInfo(&bindingFlags, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, bindings);
		vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &layout);
		// Create the pool
		const VkDescriptorPoolCreateInfo poolInfo = Vulkan::Create::DescriptorPoolCreateInfo(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT, 1, sizes);
		vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool);
		// Create the global set
		const VkDescriptorSetAllocateInfo allocateInfo = Vulkan::Create::DescriptorSetAllocateInfo(pool, &layout);
		vkAllocateDescriptorSets(device, &allocateInfo, &set);

		VkSamplerCreateInfo samplerInfo = Vulkan::Create::SamplerCreateInfo(
			VK_FILTER_MAX_ENUM, VK_FILTER_MAX_ENUM, VK_SAMPLER_MIPMAP_MODE_MAX_ENUM, VK_SAMPLER_ADDRESS_MODE_MAX_ENUM,
			1.0f, VK_LOD_CLAMP_NONE
		);

		// Create the 60 unique samplers, I reckon it's alright to create all of these as they are not too expensive
		constexpr std::array<VkFilter, 2> filters { VK_FILTER_NEAREST, VK_FILTER_LINEAR };
		constexpr std::array<VkSamplerAddressMode, 3> addressModes { VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE };

		samplers.reserve(2 * 2 * 3 * 5);
		// Ugliest code I've ever written, but it'll do
		for (const auto& magFilter : filters)
		{
			for (const auto& minFilter : filters)
			{
				for (const auto& addressMode : addressModes)
				{
					float anisotropy = 1.0f;
					// I would just do 1 variable without the uint32_t but floating point precision is a thing
					for (uint32_t i = 0; i < 5; i++, anisotropy *= 2.0f)
					{
						samplerInfo.magFilter = magFilter;
						samplerInfo.minFilter = minFilter;
						samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
						samplerInfo.addressModeU = addressMode;
						samplerInfo.addressModeV = addressMode;
						samplerInfo.addressModeW = addressMode;
						samplerInfo.anisotropyEnable = i != 1;
						samplerInfo.maxAnisotropy = anisotropy;
						samplers.emplace_back(device, samplerInfo);
					}
				}
			}
		}
	}
	RenderResourceManager::~RenderResourceManager()
	{
		if (device == nullptr)
			return;

		vkDestroyDescriptorSetLayout(*device, layout, nullptr);
		vkDestroyDescriptorPool(*device, pool, nullptr);
	}
	RenderResourceManager::RenderResourceManager(RenderResourceManager&& other) noexcept
		 : device(other.device), transferQueue(std::move(transferQueue)),
		ssboBuffers(std::move(other.ssboBuffers)), textures(std::move(other.textures)), framebuffers(std::move(other.framebuffers)),
		renderpasses(std::move(other.renderpasses)), pipelines(std::move(other.pipelines)), samplers(std::move(other.samplers)),
		layout(other.layout), pool(other.pool), set(other.set) {
		other.device = nullptr;
	}
	RenderResourceManager& RenderResourceManager::operator=(RenderResourceManager&& other) noexcept
	{
		if (this != &other)
		{
			device = other.device; other.device = nullptr;
			transferQueue = std::move(other.transferQueue);
			ssboBuffers = std::move(other.ssboBuffers);
			textures = std::move(other.textures);
			framebuffers = std::move(other.framebuffers);
			renderpasses = std::move(other.renderpasses);
			pipelines = std::move(other.pipelines);
			samplers = std::move(other.samplers);
			layout = other.layout;
			pool = other.pool;
			set = other.set;
		}
		return *this;
	}
	Vulkan::SSBOBufferHandle RenderResourceManager::CreateSSBOBuffer(VkDeviceSize size, void* data)
	{
		CS_ASSERT(size > 0, "Buffer size must be greater than 0");

		Vulkan::SSBOBufferHandle bufferHandle(ssboBuffers.emplace(Vulkan::Vk::Buffer(device->GetAllocator(),
			Vulkan::Create::BufferCreateInfo(0, size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, nullptr),
			Vulkan::Create::AllocationCreateInfo(VMA_MEMORY_USAGE_CPU_TO_GPU)
		)));
		Vulkan::Vk::Buffer* bufferPtr = ssboBuffers.get(bufferHandle);

		CS_ASSERT(bufferPtr != nullptr, "Failed to add buffer to slotmap");

		const VkDescriptorBufferInfo descriptorBufferInfo = Vulkan::Create::DescriptorBufferInfo(bufferPtr->GetBuffer(), 0, VK_WHOLE_SIZE);
		const VkWriteDescriptorSet writeDescriptorSet = Vulkan::Create::WriteDescriptorSet(set, RRM_BUFFER_BINDING, bufferHandle.GetIndex(), 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &descriptorBufferInfo);
		vkUpdateDescriptorSets(*device, 1, &writeDescriptorSet, 0, nullptr);

		if (data != nullptr)
			bufferPtr->memcpy(data, size);

		return bufferHandle;
	}
	Vulkan::GPUBufferHandle RenderResourceManager::CreateGPUBuffer(void* data, VkDeviceSize size, GPUBufferUsage usageFlags)
	{
		CS_ASSERT(data != nullptr, "Data must not be nullptr");
		CS_ASSERT(size > 0, "Buffer size must be greater than 0");

		Vulkan::Vk::Buffer gpuBuffer(device->GetAllocator(),
			Vulkan::Create::BufferCreateInfo(0, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | static_cast<VkBufferUsageFlags>(usageFlags), VK_SHARING_MODE_EXCLUSIVE, nullptr),
			Vulkan::Create::AllocationCreateInfo(VMA_MEMORY_USAGE_GPU_ONLY)
		);
		Vulkan::Vk::Buffer staging(device->GetAllocator(),
			Vulkan::Create::BufferCreateInfo(0, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, nullptr),
			Vulkan::Create::AllocationCreateInfo(VMA_MEMORY_USAGE_CPU_ONLY)
		);
		staging.memcpy(data, size);

		transferQueue.InstantSubmit([&](const Vulkan::Vk::TransferCommandQueue& cmd) {
			cmd.CopyBuffer(staging, gpuBuffer, Vulkan::Create::BufferCopy(0, 0, size));
		});

		return Vulkan::GPUBufferHandle(gpuBuffers.emplace(std::move(gpuBuffer)));
	}
	Vulkan::TextureHandle RenderResourceManager::CreateTexture(const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationCreateInfo, const TextureSpecification& textureSpec)
	{
		Vulkan::TextureHandle textureHandle(textures.emplace(Vulkan::Vk::Image(*device, device->GetAllocator(), createInfo, allocationCreateInfo)));
		Vulkan::Vk::Image* texturePtr = textures.get(textureHandle);

		CS_ASSERT(texturePtr != nullptr, "Failed to add texture to slotmap");

		const VkDescriptorImageInfo descriptorImageInfo = Vulkan::Create::DescriptorImageInfo(GetSamplerFromSpecification(textureSpec), texturePtr->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		const VkWriteDescriptorSet writeDescriptorSet = Vulkan::Create::WriteDescriptorSet(set, RRM_IMAGE_BINDING, textureHandle.GetIndex(), 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &descriptorImageInfo);
		vkUpdateDescriptorSets(*device, 1, &writeDescriptorSet, 0, nullptr);

		return textureHandle;
	}
	Vulkan::TextureHandle RenderResourceManager::UploadTexture(const cs_std::image& image, const TextureSpecification& textureSpec)
	{
		VkFormat format = GetFormatFromColorSpace(textureSpec.colorspace, image.channels);
		/* ---------------------------------------------------------------- 0. - Data validation ---------------------------------------------------------------- */
		CS_ASSERT(format != VK_FORMAT_UNDEFINED, "Invalid image data. Image format is not supported");
		CS_ASSERT(image.width > 0 && image.height > 0, "Invalid image data. Image width and height must be greater than 0");

		const size_t imageSize = static_cast<size_t>(image.width) * static_cast<size_t>(image.height) * static_cast<size_t>(image.channels);
		const uint32_t mipLevels = 1 + (textureSpec.generateMipmaps ? static_cast<uint8_t>(std::log2(std::max(image.width, image.height))) : 0);
		/* ----------------------------------------------------------------  1. - Staging ---------------------------------------------------------------- */
		Vulkan::Vk::Buffer staging(
			device->GetAllocator(),
			Vulkan::Create::BufferCreateInfo(0, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, nullptr),
			Vulkan::Create::AllocationCreateInfo(VMA_MEMORY_USAGE_CPU_ONLY)
		);
		staging.memcpy(image.data.data(), imageSize, 0);
		const VkExtent3D extent = Vulkan::Create::Extent3D(image.width, image.height, 1);
		Vulkan::Vk::Image texture(*device, device->GetAllocator(),
			Vulkan::Create::ImageCreateInfo(
				VK_IMAGE_TYPE_2D, format, extent,
				mipLevels, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED
			),
			Vulkan::Create::AllocationCreateInfo(VMA_MEMORY_USAGE_GPU_ONLY)
		);
		/* ----------------------------------------------------------------  2. - MipMap Generation ---------------------------------------------------------------- */
		this->transferQueue.InstantSubmit([&](const Vulkan::Vk::TransferCommandQueue& cmd) {
			VkImageMemoryBarrier barrier = Vulkan::Create::ImageMemoryBarrier(
				0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, texture,
				Vulkan::Create::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 1)
			);
			cmd.ResourceBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);
			const VkBufferImageCopy region = Vulkan::Create::BufferImageCopy(0, 0, 0, Vulkan::Create::ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1), Vulkan::Create::Offset3D(0, 0, 0), extent);
			cmd.CopyBufferToImage(staging, texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, region);
			barrier = Vulkan::Create::ImageMemoryBarrier(
				0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, texture,
				Vulkan::Create::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1)
			);
			for (uint32_t i = 1, mipWidth = image.width, mipHeight = image.height; i < mipLevels; i++, mipWidth = std::max(mipWidth / 2, 1u), mipHeight = std::max(mipHeight / 2, 1u))
			{
				barrier.subresourceRange.baseMipLevel = i - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				cmd.ResourceBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);
				const VkImageBlit blit = Vulkan::Create::ImageBlit(
					Vulkan::Create::ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, i - 1, 0, 1), Vulkan::Create::Offset3D(0, 0, 0), Vulkan::Create::Offset3D(static_cast<int32_t>(mipWidth), static_cast<int32_t>(mipHeight), 1),
					Vulkan::Create::ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, i, 0, 1), Vulkan::Create::Offset3D(0, 0, 0), Vulkan::Create::Offset3D(static_cast<int32_t>(std::max(mipWidth / 2, 1u)), static_cast<int32_t>(std::max(mipHeight / 2, 1u)), 1)
				);
				cmd.BlitImage(texture, texture, blit);
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				cmd.ResourceBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);
			}
			barrier.subresourceRange.baseMipLevel = mipLevels - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			cmd.ResourceBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);
		});
		/* ----------------------------------------------------------------  3. - Handle Creation ---------------------------------------------------------------- */
		Vulkan::TextureHandle textureHandle(textures.emplace(std::move(texture)));
		Vulkan::Vk::Image* texturePtr = textures.get(textureHandle);

		CS_ASSERT(texturePtr != nullptr, "Failed to add texture to slotmap");

		const VkDescriptorImageInfo descriptorImageInfo = Vulkan::Create::DescriptorImageInfo(GetSamplerFromSpecification(textureSpec), texturePtr->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		const VkWriteDescriptorSet writeDescriptorSet = Vulkan::Create::WriteDescriptorSet(set, RRM_IMAGE_BINDING, textureHandle.GetIndex(), 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &descriptorImageInfo);
		vkUpdateDescriptorSets(*device, 1, &writeDescriptorSet, 0, nullptr);

		return textureHandle;
	}
	Vulkan::FramebufferHandle RenderResourceManager::CreateFramebuffer(VkRenderPass renderPass, const std::vector<VkImageView>& attachments, VkExtent2D extent)
	{
		return Vulkan::FramebufferHandle(framebuffers.emplace(Vulkan::Vk::Framebuffer(*device, Vulkan::Create::FramebufferCreateInfo(renderPass, attachments, extent, 1))));
	}
	Vulkan::RenderPassHandle RenderResourceManager::CreateRenderPass(const VkRenderPassCreateInfo& createInfo)
	{
		return Vulkan::RenderPassHandle(renderpasses.emplace(Vulkan::Vk::RenderPass(*device, createInfo)));
	}
	Vulkan::PipelineHandle RenderResourceManager::CreatePipeline(const Vulkan::Vk::Pipeline::PipelineCreateInfo& createInfo)
	{
		return Vulkan::PipelineHandle(pipelines.emplace(*device, createInfo));
	}
	Vulkan::SSBOBufferHandle RenderResourceManager::ReserveSSBOBuffer()
	{
		return Vulkan::SSBOBufferHandle(ssboBuffers.emplace());
	}
	Vulkan::GPUBufferHandle RenderResourceManager::ReserveGPUBuffer()
	{
		return Vulkan::GPUBufferHandle(gpuBuffers.emplace());
	}
	Vulkan::TextureHandle RenderResourceManager::ReserveTexture()
	{
		return Vulkan::TextureHandle(textures.emplace());
	}
	Vulkan::FramebufferHandle RenderResourceManager::ReserveFramebuffer()
	{
		return Vulkan::FramebufferHandle(framebuffers.emplace());
	}
	Vulkan::RenderPassHandle RenderResourceManager::ReserveRenderPass()
	{
		return Vulkan::RenderPassHandle(renderpasses.emplace());
	}
	Vulkan::PipelineHandle RenderResourceManager::ReservePipeline()
	{
		return Vulkan::PipelineHandle(pipelines.emplace());
	}
	bool RenderResourceManager::IsValidSSBOBuffer(const Vulkan::SSBOBufferHandle& handle) const { return ssboBuffers.has_key(handle); }
	bool RenderResourceManager::IsValidGPUBuffer(const Vulkan::GPUBufferHandle& handle) const { return gpuBuffers.has_key(handle); }
	bool RenderResourceManager::IsValidTexture(const Vulkan::TextureHandle& handle) const { return textures.has_key(handle); }
	bool RenderResourceManager::IsValidFramebuffer(const Vulkan::FramebufferHandle& handle) const { return framebuffers.has_key(handle); }
	bool RenderResourceManager::IsValidRenderPass(const Vulkan::RenderPassHandle& handle) const { return renderpasses.has_key(handle); }
	bool RenderResourceManager::IsValidPipeline(const Vulkan::PipelineHandle& handle) const { return pipelines.has_key(handle); }
	void RenderResourceManager::ChangeTextureSampler(const Vulkan::TextureHandle& handle, const TextureSpecification& textureSpec)
	{
		Vulkan::Vk::Image* texture = textures.get(handle);
		CS_DEBUG_ASSERT(texture != nullptr, "Texture handle is invalid");
		const VkSampler sampler = GetSamplerFromSpecification(textureSpec);
		
		const VkDescriptorImageInfo descriptorImageInfo = Vulkan::Create::DescriptorImageInfo(GetSamplerFromSpecification(textureSpec), texture->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		const VkWriteDescriptorSet writeDescriptorSet = Vulkan::Create::WriteDescriptorSet(set, RRM_IMAGE_BINDING, handle.GetIndex(), 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &descriptorImageInfo);
		vkUpdateDescriptorSets(*device, 1, &writeDescriptorSet, 0, nullptr);
	}
	void RenderResourceManager::DestroySSBOBuffer(const Vulkan::SSBOBufferHandle& handle) { ssboBuffers.erase(handle); }
	void RenderResourceManager::DestroyGPUBuffer(const Vulkan::GPUBufferHandle& handle) { gpuBuffers.erase(handle); }
	void RenderResourceManager::DestroyTexture(const Vulkan::TextureHandle& handle) { textures.erase(handle); }
	void RenderResourceManager::DestroyFramebuffer(const Vulkan::FramebufferHandle& handle) { framebuffers.erase(handle); }
	void RenderResourceManager::DestroyRenderPass(const Vulkan::RenderPassHandle& handle) { renderpasses.erase(handle); }
	void RenderResourceManager::DestroyPipeline(const Vulkan::PipelineHandle& handle) { pipelines.erase(handle); }
	Vulkan::Vk::Buffer& RenderResourceManager::GetSSBOBuffer(const Vulkan::SSBOBufferHandle& handle)
	{
		Vulkan::Vk::Buffer* buffer = ssboBuffers.get(handle);
		CS_DEBUG_ASSERT(buffer != nullptr, "SSBO Buffer handle is invalid");
		return *buffer;
	}
	Vulkan::Vk::Buffer& RenderResourceManager::GetGPUBuffer(const Vulkan::GPUBufferHandle& handle)
	{
		Vulkan::Vk::Buffer* buffer = gpuBuffers.get(handle);
		CS_DEBUG_ASSERT(buffer != nullptr, "GPU Buffer handle is invalid");
		return *buffer;
	}
	Vulkan::Vk::Image& RenderResourceManager::GetTexture(const Vulkan::TextureHandle& handle)
	{
		Vulkan::Vk::Image* texture = textures.get(handle);
		CS_DEBUG_ASSERT(texture != nullptr, "Texture handle is invalid");
		return *texture;
	}
	Vulkan::Vk::Framebuffer& RenderResourceManager::GetFramebuffer(const Vulkan::FramebufferHandle& handle)
	{
		Vulkan::Vk::Framebuffer* framebuffer = framebuffers.get(handle);
		CS_DEBUG_ASSERT(framebuffer != nullptr, "Framebuffer handle is invalid");
		return *framebuffer;
	}
	Vulkan::Vk::RenderPass& RenderResourceManager::GetRenderPass(const Vulkan::RenderPassHandle& handle)
	{
		Vulkan::Vk::RenderPass* renderpass = renderpasses.get(handle);
		CS_DEBUG_ASSERT(renderpass != nullptr, "RenderPass handle is invalid");
		return *renderpass;
	}
	Vulkan::Vk::Pipeline& RenderResourceManager::GetPipeline(const Vulkan::PipelineHandle& handle)
	{
		Vulkan::Vk::Pipeline* pipeline = pipelines.get(handle);
		CS_DEBUG_ASSERT(pipeline != nullptr, "Pipeline handle is invalid");
		return *pipeline;
	}
	size_t RenderResourceManager::GetSSBOBufferCount() const { return ssboBuffers.size(); }
	size_t RenderResourceManager::GetGPUBufferCount() const { return gpuBuffers.size(); }
	size_t RenderResourceManager::GetTextureCount() const { return textures.size(); }
	size_t RenderResourceManager::GetFramebufferCount() const { return framebuffers.size(); }
	size_t RenderResourceManager::GetRenderPassCount() const { return renderpasses.size(); }
	size_t RenderResourceManager::GetPipelineCount() const { return pipelines.size(); }
	VkDescriptorSetLayout RenderResourceManager::GetDescriptorSetLayout() const { return layout;}
	VkDescriptorSet RenderResourceManager::GetDescriptorSet() const { return set; }
	VkFormat RenderResourceManager::GetFormatFromColorSpace(Colorspace colorSpace, uint16_t channels)
	{
		switch (colorSpace)
		{
		case RenderResourceManager::Colorspace::SRGB:
			switch (channels)
			{
			case 1: return VK_FORMAT_R8_SRGB;
			case 2: return VK_FORMAT_R8G8_SRGB;
			case 3: return VK_FORMAT_R8G8B8_SRGB;
			case 4: return VK_FORMAT_R8G8B8A8_SRGB;
			}
		case RenderResourceManager::Colorspace::Linear:
			switch (channels)
			{
			case 1: return VK_FORMAT_R8_UNORM;
			case 2: return VK_FORMAT_R8G8_UNORM;
			case 3: return VK_FORMAT_R8G8B8_UNORM;
			case 4: return VK_FORMAT_R8G8B8A8_UNORM;
			}
		}
		return VK_FORMAT_UNDEFINED;
	}
	VkSampler RenderResourceManager::GetSamplerFromSpecification(const TextureSpecification& textureSpec) const noexcept
	{
		constexpr size_t numFilters = 2;
		constexpr size_t numAddressModes = 3;
		constexpr size_t numAnisotropyLevels = 5;

		uint32_t magFilterIdx = static_cast<uint32_t>(textureSpec.magFilter);
		uint32_t minFilterIdx = static_cast<uint32_t>(textureSpec.minFilter);
		uint32_t wrapModeIdx = static_cast<uint32_t>(textureSpec.wrapMode);

		uint32_t anisotropyIdx = std::clamp(static_cast<uint32_t>(log2(textureSpec.anisotropy)), 0u, 4u);

		CS_DEBUG_ASSERT(magFilterIdx < numFilters, "Mag filter index out of range");
		CS_DEBUG_ASSERT(minFilterIdx < numFilters, "Min filter index out of range");
		CS_DEBUG_ASSERT(wrapModeIdx < numAddressModes, "Wrap mode index out of range");
		CS_DEBUG_ASSERT(anisotropyIdx < numAnisotropyLevels, "Anisotropy index out of range");

		uint32_t samplerIdx = anisotropyIdx
			+ (wrapModeIdx * numAnisotropyLevels)
			+ (minFilterIdx * numAddressModes * numAnisotropyLevels)
			+ (magFilterIdx * numFilters * numAddressModes * numAnisotropyLevels);

		// If an invalid sampler is somehow requested, return the first one
		return (samplerIdx < samplers.size()) ? samplers[samplerIdx] : samplers[0];
	}
}