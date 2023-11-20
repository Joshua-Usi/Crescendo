#include "VulkanInstance.hpp"
#include "Core/common.hpp"

#include "glfw/glfw3.h"

namespace Crescendo
{
	VulkanInstance::VulkanInstance(const VulkanInstanceSpecification& spec)
	{
		const size_t SSBO_OBJECT_COUNT = 8192;

		this->specs.framesInFlight = spec.framesInFlight;
		this->specs.anisotropicSamples = spec.anisotropicSamples;
		this->specs.multisamples = spec.multisamples;

		this->instance = Vulkan::Instance(spec.enableValidationLayers, spec.appName, spec.engineName, static_cast<GLFWwindow*>(spec.window));
		this->device = this->instance.CreateDevice(spec.descriptorSetsPerPool);
		this->transferQueue = this->device.CreateTransferCommandQueue();

		for (uint32_t i = 0; i < spec.framesInFlight; i++)
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
		constexpr uint32_t SHADOW_MAP_RES = 16384;

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
		if (this->textures.capacity() > 0) this->textures.erase(this->offscreen.textureIndex);
		this->offscreenDepth.~Image();

		/* ---------------------------------------------------------------- 2 - Swapchain framebuffers ---------------------------------------------------------------- */

		this->swapchain = this->instance.CreateSwapchain(this->device, VK_PRESENT_MODE_MAILBOX_KHR, VkExtent2D(width, height));

		// Create renderpasses
		uint32_t drpIdx = this->renderPasses.insert(this->device.CreateDefaultRenderPass(OFFSCREEN_FORMAT, DEPTH_FORMAT));
		uint32_t pprpIdx = this->renderPasses.insert(this->device.CreatePostProcessingRenderPass(this->swapchain.GetImageFormat()));

		// Create images
		Vulkan::Texture offscreenTexture{};
		offscreenTexture.image = this->device.CreateImage(Vulkan::Create::ImageCreateInfo(VK_IMAGE_TYPE_2D, OFFSCREEN_FORMAT, this->swapchain.GetExtent3D(), 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
		offscreenTexture.set = this->device.CreateTextureDescriptorSet(this->device.GetPostProcessingSampler(), offscreenTexture.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		this->offscreenDepth = this->device.CreateImage(Vulkan::Create::ImageCreateInfo(VK_IMAGE_TYPE_2D, DEPTH_FORMAT, this->swapchain.GetExtent3D(), 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT), VMA_MEMORY_USAGE_GPU_ONLY);

		// Create the frame buffers
		for (const auto& swapChainImage : this->swapchain)
		{
			this->framebuffers.insert(this->device.CreateFramebuffer(this->renderPasses[pprpIdx], { swapChainImage.imageView }, this->swapchain.GetExtent(), true, false));
		}

		this->offscreen.sampler = this->device.GetPostProcessingSampler();
		this->offscreen.framebufferIndex = this->framebuffers.insert(this->device.CreateFramebuffer(this->renderPasses[drpIdx], { offscreenTexture.image.imageView, this->offscreenDepth.imageView }, this->swapchain.GetExtent(), true, true));
		this->offscreen.textureIndex = this->textures.insert(std::move(offscreenTexture));

		const uint32_t smrpi = this->renderPasses.insert(this->device.CreateDefaultShadowRenderPass(SHADOW_MAP_FORMAT));
		this->shadowMap = this->CreateShadowMap(this->renderPasses[smrpi], SHADOW_MAP_FORMAT, SHADOW_MAP_RES, SHADOW_MAP_RES);
	}
	ShadowMap VulkanInstance::CreateShadowMap(VkRenderPass renderPass, VkFormat format, uint32_t width, uint32_t height)
	{
		ShadowMap map {};

		Vulkan::Texture shadowTexture{};
		shadowTexture.image = this->device.CreateImage(Crescendo::Vulkan::Create::ImageCreateInfo(VK_IMAGE_TYPE_2D, format, Crescendo::Vulkan::Create::Extent3D(width, height, 1), 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
		shadowTexture.set = this->device.CreateTextureDescriptorSet(this->device.GetDirectionalShadowMapSampler(), shadowTexture.image, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
		map.sampler = this->device.GetDirectionalShadowMapSampler();
		map.framebufferIndex = this->framebuffers.insert(this->device.CreateFramebuffer(renderPass, { shadowTexture.image.imageView }, { width, height }, false, true));
		map.textureIndex = this->textures.insert(std::move(shadowTexture));

		return map;
	}
	Vulkan::Mesh VulkanInstance::UploadMesh(const cs_std::graphics::mesh& mesh)
	{
		/* ---------------------------------------------------------------- 0 - Mesh validation ---------------------------------------------------------------- */

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
		for (const auto& attribute : mesh.attributes) CS_ASSERT(attribute.data.size() % ELEMENTS_PER_ATTRIBUTE[static_cast<size_t>(attribute.attribute)] == 0, "Invalid mesh data! mesh has " + std::to_string(attribute.data.size()) + " elements! but expected a multiple of " + std::to_string(ELEMENTS_PER_ATTRIBUTE[static_cast<size_t>(attribute.attribute)]) + "!");
		CS_ASSERT(mesh.indices.size() % 3 == 0, "Invalid mesh data! mesh has " + std::to_string(mesh.indices.size()) + " indices! but expected a multiple of 3!");
		// TODO assert for potential buffer overflows

		/* ---------------------------------------------------------------- 1 - Create GPU buffers ---------------------------------------------------------------- */

		Vulkan::Mesh gpuMesh = {};
		gpuMesh.indexCount = static_cast<uint32_t>(mesh.indices.size());
		gpuMesh.indexBuffer = this->device.CreateBuffer(
			sizeof(uint32_t) * mesh.indices.size(),
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY
		);
		for (const auto& attribute : mesh.attributes)
		{
			gpuMesh.vertexAttributes.emplace_back(
				this->device.CreateBuffer(
					sizeof(float) * attribute.data.size(),
					VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
					VMA_MEMORY_USAGE_GPU_ONLY
				), static_cast<uint32_t>(attribute.data.size()), attribute.attribute
			);
		}

		/* ---------------------------------------------------------------- 2 - Staging ---------------------------------------------------------------- */

		// Determine the buffer offsets
		std::vector<uint32_t> bufferOffsets(1, 0);
		bufferOffsets.push_back(bufferOffsets.back() + mesh.indices.size() * sizeof(uint32_t));
		for (const auto& attribute : mesh.attributes) bufferOffsets.push_back(bufferOffsets.back() + attribute.data.size() * sizeof(float));

		// Stage all data into one buffer, reduces allocations
		Crescendo::Vulkan::Buffer staging = this->device.CreateBuffer(bufferOffsets.back(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		staging.Fill(0, mesh.indices.data(), mesh.indices.size() * sizeof(uint32_t));
		for (size_t i = 0; i < mesh.attributes.size(); i++) staging.Fill(bufferOffsets[i + 1], mesh.attributes[i].data.data(), mesh.attributes[i].data.size() * sizeof(float));

		/* ---------------------------------------------------------------- 2 - Transfer ---------------------------------------------------------------- */

		// Upload data to the GPU
		this->transferQueue.InstantSubmit([&](const Crescendo::Vulkan::TransferCommandQueue& cmd) {
			// Copy index data
			VkBufferCopy copy = Crescendo::Vulkan::Create::BufferCopy(0, 0, mesh.indices.size() * sizeof(uint32_t));
			cmd.CopyBuffer(staging.buffer, gpuMesh.indexBuffer, copy);

			// Copy other vertex attributes
			uint32_t offset = 1;
			for (const auto& attribute : gpuMesh.vertexAttributes)
			{
				VkBufferCopy copy = Crescendo::Vulkan::Create::BufferCopy(bufferOffsets[offset], 0, attribute.elements * sizeof(float));
				cmd.CopyBuffer(staging.buffer, attribute.buffer, copy);
				offset++;
			}
			});

		return gpuMesh;
	}
	Vulkan::Texture VulkanInstance::UploadTexture(const cs_std::image& image, bool generateMipmaps)
	{
		/* ----------------------------------------------------------------  0 - Dynamic sampler creation ---------------------------------------------------------------- */

		// For now every image uses the same format. This can be very wasteful, at least until variable formats are implemented
		// Or we have texture compression
		constexpr VkFormat DEFAULT_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;

		const size_t imageSize = static_cast<size_t>(image.width) * static_cast<size_t>(image.height) * static_cast<size_t>(image.channels);
		const uint8_t mipLevels = 1 + generateMipmaps ? static_cast<uint8_t>(std::log2(std::max(image.width, image.height))) : 0;

		// Create samplers dynamically as required for mip levels
		// TODO add anisotropy
		VkSamplerCreateInfo samplerInfo = Crescendo::Vulkan::Create::SamplerCreateInfo(
			VK_FILTER_LINEAR, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
			this->specs.anisotropicSamples, 1.0f
		);
		for (uint8_t i = static_cast<uint8_t>(samplers.size()); i < mipLevels; i++)
		{
			samplerInfo.maxLod = static_cast<float>(i);
			this->samplers.push_back(this->device.CreateSampler(samplerInfo));
		}

		/* ----------------------------------------------------------------  1 - Staging ---------------------------------------------------------------- */

		// Stage the image data
		Crescendo::Vulkan::Buffer staging = this->device.CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY).Fill(0, image.data.data(), imageSize);

		// Create the image
		const VkExtent3D extent = Crescendo::Vulkan::Create::Extent3D(image.width, image.height, 1);
		Crescendo::Vulkan::Image texture = this->device.CreateImage(Crescendo::Vulkan::Create::ImageCreateInfo(
			VK_IMAGE_TYPE_2D, DEFAULT_FORMAT, extent,
			mipLevels, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED
		), VMA_MEMORY_USAGE_GPU_ONLY);

		/* ----------------------------------------------------------------  2 - Generate mipmaps ---------------------------------------------------------------- */

		this->transferQueue.InstantSubmit([&](const Crescendo::Vulkan::TransferCommandQueue& cmd) {
			// Transition to blit compatible
			VkImageMemoryBarrier barrier = Crescendo::Vulkan::Create::ImageMemoryBarrier(
				0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, texture.image,
				Crescendo::Vulkan::Create::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 1)
			);
			cmd.ResourceBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);
			// Copy buffer to image
			const VkBufferImageCopy region = Crescendo::Vulkan::Create::BufferImageCopy(
				0, 0, 0, Crescendo::Vulkan::Create::ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1), Crescendo::Vulkan::Create::Offset3D(0, 0, 0), extent
			);
			cmd.CopyBufferToImage(staging.buffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, region);
			// Transition the image to a transfer destination
			barrier = Crescendo::Vulkan::Create::ImageMemoryBarrier(
				0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, texture.image,
				Crescendo::Vulkan::Create::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1)
			);
			// Generate each of the mips
			uint32_t mipWidth = image.width, mipHeight = image.height;
			for (uint32_t i = 1; i < mipLevels; i++)
			{
				barrier.subresourceRange.baseMipLevel = i - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				cmd.ResourceBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);
				const VkImageBlit blit = Crescendo::Vulkan::Create::ImageBlit(
					Crescendo::Vulkan::Create::ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, i - 1, 0, 1),
					Crescendo::Vulkan::Create::Offset3D(0, 0, 0), Crescendo::Vulkan::Create::Offset3D(static_cast<int32_t>(mipWidth), static_cast<int32_t>(mipHeight), 1),
					Crescendo::Vulkan::Create::ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, i, 0, 1),
					Crescendo::Vulkan::Create::Offset3D(0, 0, 0), Crescendo::Vulkan::Create::Offset3D(static_cast<int32_t>(std::max(mipWidth / 2, 1u)), static_cast<int32_t>(std::max(mipHeight / 2, 1u)), 1)
				);
				cmd.BlitImage(texture.image, texture.image, blit);
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				cmd.ResourceBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);
				mipWidth = std::max(mipWidth / 2, 1u);
				mipHeight = std::max(mipHeight / 2, 1u);
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