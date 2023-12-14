#include "VulkanInstance.hpp"

#include "glfw/glfw3.h"

CS_NAMESPACE_BEGIN
{
	VkSampleCountFlagBits MaxMultisampleCount(const VkPhysicalDeviceProperties& properties) {
		VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
	}
	VkSampleCountFlagBits ConvertSamplesToVkFlag(uint32_t samples)
	{
		switch (samples)
	{
			case 64: return VK_SAMPLE_COUNT_64_BIT;
			case 32: return VK_SAMPLE_COUNT_32_BIT;
			case 16: return VK_SAMPLE_COUNT_16_BIT;
			case 8: return VK_SAMPLE_COUNT_8_BIT;
			case 4: return VK_SAMPLE_COUNT_4_BIT;
			case 2: return VK_SAMPLE_COUNT_2_BIT;
		}
		// Default to 1 sample if invalid
		return VK_SAMPLE_COUNT_1_BIT;
	}
	VulkanInstance::VulkanInstance(const VulkanInstanceSpecification& spec) : specs(spec)
	{
		const size_t SSBO_OBJECT_COUNT = 8192;

		this->instance = Vulkan::Instance(specs.enableValidationLayers, specs.appName, specs.engineName, static_cast<GLFWwindow*>(specs.window));
		this->device = this->instance.CreateDevice(specs.descriptorSetsPerPool);
		this->transferQueue = this->device.CreateTransferCommandQueue();

		// Modify spec if some values are invalid
		float maxAnisotropy = this->instance.GetPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
		uint32_t maxMultisamples = MaxMultisampleCount(this->instance.GetPhysicalDeviceProperties());

		specs.anisotropicSamples = std::clamp(specs.anisotropicSamples, 1u, static_cast<uint32_t>(maxAnisotropy));
		specs.multisamples = std::clamp(specs.multisamples, 1u, maxMultisamples);

		for (uint32_t i = 0; i < specs.framesInFlight; i++)
		{
			this->renderCommandQueues.emplace_back(this->device.CreateGraphicsCommandQueue(), this->device.CreateSemaphore(), this->device.CreateSemaphore());
			this->ssbo.emplace_back(this->device.CreateSSBO(sizeof(glm::mat4) * SSBO_OBJECT_COUNT, VMA_MEMORY_USAGE_CPU_TO_GPU));
		}
		this->CreateSwapchain();
	}
	VulkanInstance::~VulkanInstance()
	{
		if (!this->device) return;
		this->device.WaitIdle();
		// Destroy samplers
		for (auto& sampler : this->samplers) vkDestroySampler(this->device, sampler, nullptr);
	}
	void VulkanInstance::CreateSwapchain()
	{
		/* ---------------------------------------------------------------- 0. - Wait for resize finish ---------------------------------------------------------------- */

		constexpr VkFormat DEPTH_FORMAT = VK_FORMAT_D32_SFLOAT;
		constexpr VkFormat OFFSCREEN_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT; // For HDR
		constexpr VkFormat SHADOW_MAP_FORMAT = VK_FORMAT_D16_UNORM;
		constexpr uint32_t SHADOW_MAP_RES = 4096;
		const VkSampleCountFlagBits multisamplesCount = ConvertSamplesToVkFlag(this->specs.multisamples);

		this->device.WaitIdle();
		// Get glfw window size
		int width = 0, height = 0;
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(static_cast<GLFWwindow*>(this->instance.GetWindow()), &width, &height);
			glfwWaitEvents();
		}

		/* ---------------------------------------------------------------- 1. - Resource deletion ---------------------------------------------------------------- */

		this->framebuffers.clear();
		if (this->renderPasses.capacity() > 0) this->renderPasses.erase(0);
		// Explicitly destroy swapchain and depth buffer
		this->swapchain.~Swapchain();
		if (this->textures.capacity() > 0)
		{
			for (const auto& index : this->offscreen.textureIndices) this->textures.erase(index);
			for (const auto& index : this->shadowMap.textureIndices) this->textures.erase(index);
		}

		/* ---------------------------------------------------------------- 2 - Swapchain framebuffers ---------------------------------------------------------------- */

		this->swapchain = this->instance.CreateSwapchain(this->device, VK_PRESENT_MODE_MAILBOX_KHR, VkExtent2D(width, height));

		// Create renderpasses
		const uint32_t drpIdx = this->renderPasses.insert(this->device.CreateDefaultRenderPass(OFFSCREEN_FORMAT, DEPTH_FORMAT, multisamplesCount));
		const uint32_t pprpIdx = this->renderPasses.insert(this->device.CreateDefaultPostProcessingRenderPass(this->swapchain.GetImageFormat()));
		const uint32_t smrpIdx = this->renderPasses.insert(this->device.CreateDefaultShadowRenderPass(SHADOW_MAP_FORMAT));
		const uint32_t dpprpIdx = this->renderPasses.insert(this->device.CreateDefaultDepthPrePassRenderPass(DEPTH_FORMAT, multisamplesCount));

		// Create the swapchain frame buffers
		for (const auto& swapChainImage : this->swapchain)
		{
			this->framebuffers.insert(this->device.CreateFramebuffer(this->renderPasses[pprpIdx], { swapChainImage.imageView }, this->swapchain.GetExtent(), true, false));
		}

		// Create offscreen images and framebuffers
		VkExtent3D offscreenExtent = this->swapchain.GetExtent3D();
		offscreenExtent.width = static_cast<uint32_t>(static_cast<float>(offscreenExtent.width) * specs.renderScale);
		offscreenExtent.height = static_cast<uint32_t>(static_cast<float>(offscreenExtent.height) * specs.renderScale);

		// Create depth prepass framebuffer
		this->depthPrepass = this->CreateDepthPrepass(this->renderPasses[dpprpIdx], DEPTH_FORMAT, multisamplesCount, offscreenExtent.width, offscreenExtent.height);
		// Create offscreen framebuffer
		this->offscreen = this->CreateOffscreen(this->renderPasses[drpIdx], OFFSCREEN_FORMAT, this->depthPrepass.textureIndices[0], multisamplesCount, offscreenExtent.width, offscreenExtent.height);
		// Create shadow map
		this->shadowMap = this->CreateShadowMap(this->renderPasses[smrpIdx], SHADOW_MAP_FORMAT, SHADOW_MAP_RES, SHADOW_MAP_RES);
	}
	SamplableFramebuffer VulkanInstance::CreateOffscreen(VkRenderPass pass, VkFormat colorFormat, uint32_t depthTextureIndex, VkSampleCountFlagBits multisamples, uint32_t width, uint32_t height)
	{
		bool isMultisampling = multisamples != VK_SAMPLE_COUNT_1_BIT;

		SamplableFramebuffer offscreen {};

		Vulkan::Texture offscreenMultisampleTexture {};
		if (isMultisampling) offscreenMultisampleTexture.image = this->device.CreateImage(Vulkan::Create::ImageCreateInfo(VK_IMAGE_TYPE_2D, colorFormat, { width, height, 1 }, 1, 1, multisamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VMA_MEMORY_USAGE_GPU_ONLY);

		Vulkan::Texture offscreenResolveTexture {};
		offscreenResolveTexture.image = this->device.CreateImage(Vulkan::Create::ImageCreateInfo(VK_IMAGE_TYPE_2D, colorFormat, { width, height, 1 }, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
		offscreenResolveTexture.set = this->device.CreateTextureDescriptorSet(this->device.GetPostProcessingSampler(), offscreenResolveTexture.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		Vulkan::Texture& offscreenTextureDepth = this->textures[depthTextureIndex];

		std::vector<VkImageView> attachments = (isMultisampling) ? std::vector<VkImageView>{offscreenMultisampleTexture.image.imageView, offscreenTextureDepth.image.imageView, offscreenResolveTexture.image.imageView} : std::vector<VkImageView>{offscreenResolveTexture.image.imageView, offscreenTextureDepth.image.imageView};
		std::vector<uint32_t> textureIndices = {
			static_cast<uint32_t>(this->textures.insert(std::move(offscreenResolveTexture))),
			depthTextureIndex
		};
		if (isMultisampling) textureIndices.push_back(static_cast<uint32_t>(this->textures.insert(std::move(offscreenMultisampleTexture))));

		offscreen.sampler = this->device.GetPostProcessingSampler();
		offscreen.framebufferIndex = this->framebuffers.insert(this->device.CreateFramebuffer(pass, attachments, { width, height }, true, true));
		offscreen.textureIndices = textureIndices;
			
		return offscreen;
	}
	SamplableFramebuffer VulkanInstance::CreateShadowMap(VkRenderPass renderPass, VkFormat format, uint32_t width, uint32_t height)
	{
		SamplableFramebuffer map {};

		Vulkan::Texture shadowTexture{};
		shadowTexture.image = this->device.CreateImage(Vulkan::Create::ImageCreateInfo(VK_IMAGE_TYPE_2D, format, Vulkan::Create::Extent3D(width, height, 1), 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
		shadowTexture.set = this->device.CreateTextureDescriptorSet(this->device.GetDirectionalShadowMapSampler(), shadowTexture.image, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
		map.sampler = this->device.GetDirectionalShadowMapSampler();
		map.framebufferIndex = this->framebuffers.insert(this->device.CreateFramebuffer(renderPass, { shadowTexture.image.imageView }, { width, height }, false, true));
		map.textureIndices.push_back(this->textures.insert(std::move(shadowTexture)));

		return map;
	}
	SamplableFramebuffer VulkanInstance::CreateDepthPrepass(VkRenderPass renderPass, VkFormat format, VkSampleCountFlagBits multisamples, uint32_t width, uint32_t height)
	{
		SamplableFramebuffer depthPrepass {};

		Vulkan::Texture depthTexture{};
		depthTexture.image = this->device.CreateImage(Vulkan::Create::ImageCreateInfo(VK_IMAGE_TYPE_2D, format, Vulkan::Create::Extent3D(width, height, 1), 1, 1, multisamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
		depthTexture.set = nullptr;
		depthPrepass.sampler = nullptr;
		depthPrepass.framebufferIndex = this->framebuffers.insert(this->device.CreateFramebuffer(renderPass, { depthTexture.image.imageView }, { width, height }, false, true));
		depthPrepass.textureIndices.push_back(this->textures.insert(std::move(depthTexture)));

		return depthPrepass;
	}
	Vulkan::Mesh VulkanInstance::UploadMesh(const cs_std::graphics::mesh & mesh)
	{
		constexpr size_t ELEMENTS_PER_ATTRIBUTE[static_cast<size_t>(cs_std::graphics::Attribute::ATTRIBUTE_COUNT)]{
			3, // POSITION
			3, // NORMAL
			4, // TANGENT
			2, // TEXCOORD0
			2, // TEXCOORD1
			4, // COLOR0
			4, // JOINTS0
			4  // WEIGHTS0
		};
		/* ---------------------------------------------------------------- 0 - Mesh validation ---------------------------------------------------------------- */
		for (const auto& attribute : mesh.attributes) CS_ASSERT(attribute.data.size() % ELEMENTS_PER_ATTRIBUTE[static_cast<size_t>(attribute.attribute)] == 0, "Invalid mesh data! mesh has " + std::to_string(attribute.data.size()) + " elements! but expected a multiple of " + std::to_string(ELEMENTS_PER_ATTRIBUTE[static_cast<size_t>(attribute.attribute)]) + "!");
		CS_ASSERT(mesh.indices.size() % 3 == 0, "Invalid mesh data! mesh has " + std::to_string(mesh.indices.size()) + " indices! but expected a multiple of 3!");
		// TODO assert for potential buffer overflows
		/* ---------------------------------------------------------------- 1 - Create GPU buffers ---------------------------------------------------------------- */
		Vulkan::Mesh gpuMesh = {};
		gpuMesh.indexCount = static_cast<uint32_t>(mesh.indices.size());
		gpuMesh.indexBuffer = this->device.CreateBuffer(sizeof(uint32_t) * mesh.indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		for (const auto& attribute : mesh.attributes)
		{
			gpuMesh.vertexAttributes.emplace_back(this->device.CreateBuffer(sizeof(float) * attribute.data.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY), static_cast<uint32_t>(attribute.data.size()), attribute.attribute);
		}
		/* ---------------------------------------------------------------- 2 - Staging ---------------------------------------------------------------- */
		std::vector<uint32_t> bufferOffsets(1, 0);
		bufferOffsets.push_back(bufferOffsets.back() + mesh.indices.size() * sizeof(uint32_t));
		for (const auto& attribute : mesh.attributes) bufferOffsets.push_back(bufferOffsets.back() + attribute.data.size() * sizeof(float));

		Vulkan::Buffer staging = this->device.CreateBuffer(bufferOffsets.back(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		staging.Fill(0, mesh.indices.data(), mesh.indices.size() * sizeof(uint32_t));
		for (uint32_t i = 0; i < mesh.attributes.size(); i++) staging.Fill(bufferOffsets[i + 1], mesh.attributes[i].data.data(), mesh.attributes[i].data.size() * sizeof(float));
		/* ---------------------------------------------------------------- 2 - Transfer ---------------------------------------------------------------- */
		this->transferQueue.InstantSubmit([&](const Vulkan::TransferCommandQueue& cmd) {
			cmd.CopyBuffer(staging.buffer, gpuMesh.indexBuffer, Vulkan::Create::BufferCopy(0, 0, mesh.indices.size() * sizeof(uint32_t)));
			for (uint32_t i = 0; i < gpuMesh.vertexAttributes.size(); i++)
			{
				cmd.CopyBuffer(staging.buffer, gpuMesh.vertexAttributes[i].buffer, Vulkan::Create::BufferCopy(bufferOffsets[i + 1], 0, gpuMesh.vertexAttributes[i].elements * sizeof(float)));
			}
		});
		return gpuMesh;
	}
	Vulkan::Texture VulkanInstance::UploadTexture(const cs_std::image & image, bool generateMipmaps)
	{
		// For now every image uses the same format. This can be very wasteful, at least until variable formats are implemented
		// Or we have texture compression
		constexpr VkFormat DEFAULT_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;
		/* ----------------------------------------------------------------  0 - Dynamic sampler creation ---------------------------------------------------------------- */
		const size_t imageSize = static_cast<size_t>(image.width) * static_cast<size_t>(image.height) * static_cast<size_t>(image.channels);
		const uint8_t mipLevels = 1 + generateMipmaps ? static_cast<uint8_t>(std::log2(std::max(image.width, image.height))) : 0;
		VkSamplerCreateInfo samplerInfo = Vulkan::Create::SamplerCreateInfo(
			VK_FILTER_LINEAR, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
			this->specs.anisotropicSamples, 1.0f
		);
		for (uint8_t i = static_cast<uint8_t>(samplers.size()); i < mipLevels; i++)
		{
			samplerInfo.maxLod = static_cast<float>(i);
			this->samplers.push_back(this->device.CreateSampler(samplerInfo));
		}
		/* ----------------------------------------------------------------  1 - Staging ---------------------------------------------------------------- */
		Vulkan::Buffer staging = this->device.CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY).Fill(0, image.data.data(), imageSize);
		const VkExtent3D extent = Vulkan::Create::Extent3D(image.width, image.height, 1);
		Vulkan::Image texture = this->device.CreateImage(Vulkan::Create::ImageCreateInfo(
			VK_IMAGE_TYPE_2D, DEFAULT_FORMAT, extent,
			mipLevels, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED
		), VMA_MEMORY_USAGE_GPU_ONLY);
		/* ----------------------------------------------------------------  2 - Generate mipmaps ---------------------------------------------------------------- */
		this->transferQueue.InstantSubmit([&](const Vulkan::TransferCommandQueue& cmd) {
			VkImageMemoryBarrier barrier = Vulkan::Create::ImageMemoryBarrier(
				0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, texture.image,
				Vulkan::Create::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 1)
			);
			cmd.ResourceBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);
			const VkBufferImageCopy region = Vulkan::Create::BufferImageCopy(0, 0, 0, Vulkan::Create::ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1), Vulkan::Create::Offset3D(0, 0, 0), extent);
			cmd.CopyBufferToImage(staging.buffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, region);
			barrier = Vulkan::Create::ImageMemoryBarrier(
				0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, texture.image,
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
				cmd.BlitImage(texture.image, texture.image, blit);
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
		/* ----------------------------------------------------------------  3 - Create descriptor set ---------------------------------------------------------------- */
		VkDescriptorSet descriptorSet = this->device.CreateTextureDescriptorSet(this->samplers[mipLevels - 1], texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		return Vulkan::Texture(std::move(texture), descriptorSet);
	}
}