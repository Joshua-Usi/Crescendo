#include "RendererImpl.hpp"

namespace Crescendo
{
	constexpr vkb::PreferredDeviceType DEVICE_TYPE_MAPPING[2] = { vkb::PreferredDeviceType::discrete, vkb::PreferredDeviceType::integrated };
	constexpr VkPresentModeKHR PRESENT_MODE_MAPPING[2] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR };
	constexpr VkFormat DEFAULT_DEPTH_FORMAT = VK_FORMAT_D32_SFLOAT;

	/* ---------------------------------------------------------------- Helper functions ---------------------------------------------------------------- */

	VkSampleCountFlagBits sampleMap(uint32_t samples)
	{
		if (samples > 64) return VK_SAMPLE_COUNT_64_BIT;
		switch (samples)
		{
			case 64: return VK_SAMPLE_COUNT_64_BIT;
			case 32: return VK_SAMPLE_COUNT_32_BIT;
			case 16: return VK_SAMPLE_COUNT_16_BIT;
			case 8:  return VK_SAMPLE_COUNT_8_BIT;
			case 4:  return VK_SAMPLE_COUNT_4_BIT;
			case 2:  return VK_SAMPLE_COUNT_2_BIT;
		}
		// If an invalid value is provided , default to 1 sample
		return VK_SAMPLE_COUNT_1_BIT;
	}
	VkSampleCountFlagBits getMaxSampleCounts(VkPhysicalDeviceProperties properties)
	{
		VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
		if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
		if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
		if (counts & VK_SAMPLE_COUNT_8_BIT)  return VK_SAMPLE_COUNT_8_BIT;
		if (counts & VK_SAMPLE_COUNT_4_BIT)  return VK_SAMPLE_COUNT_4_BIT;
		if (counts & VK_SAMPLE_COUNT_2_BIT)  return VK_SAMPLE_COUNT_2_BIT;
		return VK_SAMPLE_COUNT_1_BIT;
	}
	/* ---------------------------------------------------------------- Fixed initialisations ---------------------------------------------------------------- */

	void Renderer::RendererImpl::InitialiseInstance(const BuilderInfo& info)
	{
		this->rendererInfo = info;

		// Create instance and debug messenger
		vkb::Instance instance = vkb::InstanceBuilder()
			.set_app_name(info.appName.c_str())
			.set_engine_name(info.engineName.c_str())
			.request_validation_layers(info.useValidationLayers)
			.require_api_version(1, 3, 0) // Using 1.3 at minimum
			.use_default_debug_messenger()
			.build().value();
		this->instance = instance.instance;
		this->debugMessenger = instance.debug_messenger;

		// Create surface
		CS_ASSERT(glfwCreateWindowSurface(this->instance, static_cast<GLFWwindow*>(info.window), nullptr, &this->surface) == VK_SUCCESS, "Failed to create window surface!");
		
		// Device features
		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.fillModeNonSolid = VK_TRUE;
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.sampleRateShading = VK_TRUE;

		// Select physical device
		const vkb::PhysicalDevice physicalDeviceResult = vkb::PhysicalDeviceSelector(instance)
			.set_minimum_version(1, 3) // We use bindless, since we are using 1.3 we don't need to enable it
			.prefer_gpu_device_type(DEVICE_TYPE_MAPPING[static_cast<uint32_t>(info.preferredDeviceType)])
			.set_surface(this->surface)
			.set_required_features(deviceFeatures)
			.select().value();
		this->physicalDevice = physicalDeviceResult.physical_device;
		this->physicalDeviceProperties = physicalDeviceResult.properties;

		// Create logical device
		const vkb::Device deviceResult = vkb::DeviceBuilder(physicalDeviceResult).build().value();
		this->device = internal::Device(deviceResult.device);

		// Find queues
		this->queues.GetQueues(deviceResult);

		// Initialise Allocator
		this->allocator = internal::Allocator(device).Initialise(this->instance, this->physicalDevice);
		
		// Set values
		this->state.frameIndex = 0;
		this->state.frameData.resize(info.framesInFlight);
		this->state.msaaSamples = std::min(sampleMap(info.msaaSamples), getMaxSampleCounts(this->physicalDeviceProperties));

		// Push to deletion queue
		this->mainDeletionQueue.push([&]() {
			this->swapChainDeletionQueue.flush();
			this->allocator.Destroy();
			vkDestroyDevice(this->device, nullptr);
			vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
			vkb::destroy_debug_utils_messenger(this->instance, this->debugMessenger);
			vkDestroyInstance(this->instance, nullptr);
		});
	}
	void Renderer::RendererImpl::InitialiseTransferQueues(const BuilderInfo& info)
	{
		// Upload command queues
		this->uploadQueue = internal::CommandQueue(this->device, this->queues.transfer).Initialise(false);
		this->uploadTextureQueue = internal::CommandQueue(this->device, this->queues.universal).Initialise(false);

		this->mainDeletionQueue.push([&]() {
			this->uploadQueue.Destroy();
			this->uploadTextureQueue.Destroy();
		});
	}
	void Renderer::RendererImpl::InitialiseDescriptors(const BuilderInfo& info)
	{
		this->descriptorManager = internal::DescriptorManager(this->device).Initialise(info.desriptorSetsPerPool);

		// Create the default texture descriptor set layout
		this->textureDescriptorSetLayout = this->device.CreateDescriptorSetLayout(Create::DescriptorSetLayoutBinding(
			0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr
		));

		this->shadowMapDescriptorSet = this->descriptorManager.AllocateSet(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->textureDescriptorSetLayout);

		this->mainDeletionQueue.push([&]() {
			for (auto& sampler : this->samplers) vkDestroySampler(this->device, sampler, nullptr);
			this->samplers.clear();

			for (auto& descriptorSetLayout : this->dataDescriptorSetLayouts) vkDestroyDescriptorSetLayout(this->device, descriptorSetLayout, nullptr);
			this->dataDescriptorSetLayouts.clear();

			vkDestroyDescriptorSetLayout(this->device, this->textureDescriptorSetLayout, nullptr);

			this->descriptorManager.Destroy();
		});
	}
	void Renderer::RendererImpl::InitialisePipelines(const BuilderInfo& info)
	{
		this->mainDeletionQueue.push([&]() {
			for (auto& pipeline : this->pipelines) vkDestroyPipeline(this->device, pipeline.pipeline, nullptr);
			this->pipelines.clear();

			for (auto& pipelineLayout : this->pipelineLayouts) vkDestroyPipelineLayout(this->device, pipelineLayout, nullptr);
			this->pipelineLayouts.clear();
		});
	}
	void Renderer::RendererImpl::InitialiseBuffers(const BuilderInfo& info)
	{
		/* -------------------------------- 1. Vertex buffers -------------------------------- */

		constexpr size_t SHADER_ATTRIBUTE_BUFFER_COUNT = static_cast<size_t>(ShaderAttributeFlag::SHADER_ATTRIBUTE_FLAG_COUNT);

		this->vertexBuffers.resize(SHADER_ATTRIBUTE_BUFFER_COUNT + 1);
		this->offsets = std::vector<std::vector<uint32_t>>(SHADER_ATTRIBUTE_BUFFER_COUNT + 1, std::vector<uint32_t>(1, 0));

		// Zeroth buffer is the index buffer
		this->vertexBuffers[0] = this->allocator.CreateBuffer(info.vertexBufferBlockSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		
		for (size_t i = 1; i < SHADER_ATTRIBUTE_BUFFER_COUNT + 1; i++)
		{
			this->vertexBuffers[i] = this->allocator.CreateBuffer(info.vertexBufferBlockSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		}

		/* -------------------------------- 2. Descriptor buffers -------------------------------- */

		this->dataDescriptorSetBuffers.resize(info.framesInFlight);
		for (uint32_t i = 0; i < info.framesInFlight; i++)
		{
			this->dataDescriptorSetBuffers[i] = this->allocator.CreateBuffer(info.descriptorBufferBlockSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		}

		this->mainDeletionQueue.push([&]() {
			// Delete vertex buffers
			for (auto& vertexBuffer : this->vertexBuffers) this->allocator.DestroyBuffer(vertexBuffer);
			this->vertexBuffers.clear();
			// Delete descriptor buffer
			for (auto& buffer : this->dataDescriptorSetBuffers) this->allocator.DestroyBuffer(buffer);
			this->dataDescriptorSetBuffers.clear();
			// Delete texture buffers
			for (auto& image : this->images) this->allocator.DestroyImage(image);
			this->images.clear();
		});
	}

	/* ---------------------------------------------------------------- Variable initialisations ---------------------------------------------------------------- */

	void Renderer::RendererImpl::InitialiseSwapchain(const BuilderInfo& info)
	{
		// Create swapchain
		vkb::Swapchain vkbSwapchain = vkb::SwapchainBuilder(this->physicalDevice, this->device, this->surface)
			.use_default_format_selection()
			.set_desired_present_mode(PRESENT_MODE_MAPPING[static_cast<uint32_t>(info.preferredPresentMode)])
			.set_desired_extent(info.windowExtent.width, info.windowExtent.height)
			.build().value();

		// Merge into images
		std::vector<VkImage> images = vkbSwapchain.get_images().value();
		std::vector<VkImageView> imageViews = vkbSwapchain.get_image_views().value();
		std::vector<internal::Allocator::Image> viewableImages(images.size());
		for (size_t i = 0; i < images.size(); i++) viewableImages[i] = internal::Allocator::Image(images[i], imageViews[i]);
		this->swapchain = Swapchain(vkbSwapchain.swapchain, vkbSwapchain.image_format, vkbSwapchain.extent, viewableImages);

		// Add to swapchain deletion queue
		this->swapChainDeletionQueue.push([&]() {
			for (auto& image : this->swapchain.images) this->allocator.DestroyImage(image);
			this->swapchain.images.clear();

			vkDestroySwapchainKHR(this->device, this->swapchain.swapchain, nullptr);
		});
	}
	void Renderer::RendererImpl::InitialiseFlightFrames(const BuilderInfo& info)
	{
		this->state.frameData.resize(info.framesInFlight);
		// Render command queues
		for (uint32_t i = 0; i < this->rendererInfo.framesInFlight; i++)
		{
			this->state.frameData[i].commandQueue = internal::CommandQueue(this->device, this->queues.universal).Initialise(true);
			this->state.frameData[i].presentSemaphore = this->device.CreateSemaphore();
			this->state.frameData[i].renderSemaphore = this->device.CreateSemaphore();
		}

		this->swapChainDeletionQueue.push([&]() {
			for (auto& frame : this->state.frameData)
			{
				frame.commandQueue.Destroy();
				vkDestroySemaphore(this->device, frame.presentSemaphore, nullptr);
				vkDestroySemaphore(this->device, frame.renderSemaphore, nullptr);
			}
			this->state.frameData.clear();
		});
	}
	void Renderer::RendererImpl::InitialiseFramebuffers(const BuilderInfo& info)
	{
		/* -------------------------------- 0. RenderPass creation -------------------------------- */

		this->renderPasses.push_back({ this->device.CreateDefaultRenderPass(this->swapchain.imageFormat, DEFAULT_DEPTH_FORMAT, this->state.msaaSamples), true, true });
		if (info.shadowMapResolution > 0)
		{
			this->renderPasses.push_back({ this->device.CreateDefaultShadowRenderPass(DEFAULT_DEPTH_FORMAT), false, true });
		}

		/* -------------------------------- 1. Image buffer creation -------------------------------- */

		// Create the depth buffer
		VkImageCreateInfo depthImageInfo = Create::ImageCreateInfo(
			VK_IMAGE_TYPE_2D, DEFAULT_DEPTH_FORMAT, this->swapchain.GetExtent3D(), 1, 1, this->state.msaaSamples, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED
		);
		this->depthBuffer = this->allocator.CreateImage(depthImageInfo, VMA_MEMORY_USAGE_GPU_ONLY);

		// Create msaa buffer
		if (info.msaaSamples > 1)
		{
			VkImageCreateInfo multisamplingImageInfo = Create::ImageCreateInfo(
				VK_IMAGE_TYPE_2D, this->swapchain.imageFormat, this->swapchain.GetExtent3D(), 1, 1, this->state.msaaSamples, VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED
			);
			this->multisamplingBuffer = this->allocator.CreateImage(multisamplingImageInfo, VMA_MEMORY_USAGE_GPU_ONLY);
		}

		// Create the shadow map buffer
		if (info.shadowMapResolution > 0)
		{
			VkImageCreateInfo shadowMapImageInfo = Create::ImageCreateInfo(
				VK_IMAGE_TYPE_2D, DEFAULT_DEPTH_FORMAT, { info.shadowMapResolution, info.shadowMapResolution, 1 }, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED
			);
			this->shadowMapBuffer = this->allocator.CreateImage(shadowMapImageInfo, VMA_MEMORY_USAGE_GPU_ONLY);
		}

		bool isMultiSampling = this->state.msaaSamples != VK_SAMPLE_COUNT_1_BIT;

		/* -------------------------------- 2. Framebuffer creation -------------------------------- */
		
		// Add attachments
		// When there is no multisampling, it is swapchain -> depth
		// When there is multisampling, it is multisampling -> depth -> swapchain for some reason
		std::vector<VkImageView> attachments;
		if (isMultiSampling) attachments.push_back(this->multisamplingBuffer.imageView);
		attachments.push_back(isMultiSampling ? this->depthBuffer.imageView : nullptr);
		attachments.push_back(isMultiSampling ? nullptr : this->depthBuffer.imageView);

		uint32_t swapChainViewIndex = isMultiSampling ? 2 : 0;

		for (const auto& swapChainImage : this->swapchain.images)
		{
			attachments[swapChainViewIndex] = swapChainImage.imageView;
			this->framebuffers.push_back(this->device.CreateFramebuffer(this->renderPasses[DEFAULT_RENDER_PASS], attachments, info.windowExtent.width, info.windowExtent.height));
		}

		// Create shadowmap framebuffer
		if (info.shadowMapResolution > 0)
		{
			this->shadowMapFramebuffer = this->device.CreateFramebuffer(this->renderPasses[SHADOW_RENDER_PASS], this->shadowMapBuffer.imageView, info.shadowMapResolution, info.shadowMapResolution);
		}

		/* -------------------------------- 3. Shadow map descriptor set -------------------------------- */

		this->shadowMapSampler = this->device.CreateSampler(Create::SamplerCreateInfo(
			VK_FILTER_LINEAR, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			1.0f, 1.0f, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
		));
		VkDescriptorImageInfo imageInfo = Create::DescriptorImageInfo(
			shadowMapSampler, this->shadowMapBuffer.imageView, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
		);
		VkWriteDescriptorSet write = Create::WriteDescriptorSet(
			this->shadowMapDescriptorSet, 0, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &imageInfo, nullptr, nullptr
		);
		vkUpdateDescriptorSets(this->device, 1, &write, 0, nullptr);

		// Add to swapchain deletion queue because it can be destroyed by window resizes
		this->swapChainDeletionQueue.push([&]() {

			for (auto& framebuffer : this->framebuffers) vkDestroyFramebuffer(this->device, framebuffer, nullptr);
			this->framebuffers.clear();
			vkDestroyFramebuffer(this->device, this->shadowMapFramebuffer, nullptr);

			this->allocator.DestroyImage(this->depthBuffer);
			this->allocator.DestroyImage(this->multisamplingBuffer);
			this->allocator.DestroyImage(this->shadowMapBuffer);

			for (auto& renderPass : this->renderPasses) vkDestroyRenderPass(this->device, renderPass, nullptr);
			this->renderPasses.clear();

			vkDestroySampler(this->device, this->shadowMapSampler, nullptr);
		});
	}
}