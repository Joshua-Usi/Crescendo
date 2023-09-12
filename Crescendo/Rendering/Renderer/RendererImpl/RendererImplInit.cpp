#include "RendererImpl.hpp"

namespace Crescendo
{
	constexpr vkb::PreferredDeviceType DEVICE_TYPE_MAPPING[2] = { vkb::PreferredDeviceType::discrete, vkb::PreferredDeviceType::integrated };
	constexpr VkPresentModeKHR PRESENT_MODE_MAPPING[2] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR };
	constexpr VkFormat DEFAULT_DEPTH_FORMAT = VK_FORMAT_D24_UNORM_S8_UINT;

	VkSampleCountFlagBits sampleMap(uint32_t samples)
	{
		switch (samples)
		{
			case std::numeric_limits<uint32_t>::max():
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
	uint32_t sampleCountMap(VkSampleCountFlagBits samples)
	{
		switch (samples)
		{
			case VK_SAMPLE_COUNT_64_BIT: return 64;
			case VK_SAMPLE_COUNT_32_BIT: return 32;
			case VK_SAMPLE_COUNT_16_BIT: return 16;
			case VK_SAMPLE_COUNT_8_BIT:  return 8;
			case VK_SAMPLE_COUNT_4_BIT:  return 4;
			case VK_SAMPLE_COUNT_2_BIT:  return 2;
		}
		return 1;
	}

	void Renderer::RendererImpl::InitialiseInstance(const BuilderInfo& info)
	{
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

		VkPhysicalDeviceDescriptorIndexingFeatures pddif = {};
		pddif.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

		// Enable when using bindless
		/*pddif.runtimeDescriptorArray = VK_TRUE;
		pddif.descriptorBindingPartiallyBound = VK_TRUE;
		pddif.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
		pddif.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		pddif.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
		pddif.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
		pddif.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
		pddif.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;*/

		// Create logical device
		const vkb::Device deviceResult = vkb::DeviceBuilder(physicalDeviceResult).add_pNext(&pddif).build().value();
		this->device = deviceResult.device;

		// Find queues
		this->queues.GetQueues(deviceResult);

		// Initialise Allocator
		this->allocator = internal::Allocator(instance, physicalDevice, device).Initialise();
		
		// Set values
		this->state.framesInFlight = info.framesInFlight;
		this->state.frameIndex = 0;
		this->state.frameData.resize(info.framesInFlight);
		this->window = static_cast<GLFWwindow*>(info.window);
		this->state.msaaSamples = std::min(sampleMap(info.msaaSamples), getMaxSampleCounts(this->physicalDeviceProperties));

		// Push to deletion queue
		this->mainDeletionQueue.Push([&]() {
			this->swapChainDeletionQueue.Flush();
			this->allocator.Destroy();
			vkDestroyDevice(this->device, nullptr);
			vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
			vkb::destroy_debug_utils_messenger(this->instance, this->debugMessenger);
			vkDestroyInstance(this->instance, nullptr);
		});
	}
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
		this->swapChainDeletionQueue.Push([&]() {
			for (auto& image : this->swapchain.images) this->allocator.DestroyImage(image);
			vkDestroySwapchainKHR(this->device, this->swapchain.swapchain, nullptr);
		});
	}
	void Renderer::RendererImpl::InitialiseCommands(const BuilderInfo& info)
	{
		// Render command queues
		for (uint32_t i = 0; i < this->state.framesInFlight; i++)
		{
			this->state.frameData[i].commandQueue = internal::CommandQueue(this->device, this->queues.universal, this->queues.universalFamily).Initialise(true);
		}
		// Upload command queues
		this->uploadQueue = internal::CommandQueue(this->device, this->queues.transfer, this->queues.transferFamily).Initialise(false);
		this->uploadTextureQueue = internal::CommandQueue(this->device, this->queues.universal, this->queues.universalFamily).Initialise(false);
		
		this->mainDeletionQueue.Push([&]() {
			for (auto& frame : this->state.frameData) frame.commandQueue.Destroy();
			this->uploadQueue.Destroy();
			this->uploadTextureQueue.Destroy();
		});
	}
	void Renderer::RendererImpl::InitialiseSyncStructures(const BuilderInfo& info)
	{
		// Create sync structures for frames in flight
		const VkSemaphoreCreateInfo semaphoreInfo = Create::SemaphoreCreateInfo();
		for (uint32_t i = 0; i < info.framesInFlight; i++)
		{
			CS_ASSERT(vkCreateSemaphore(this->device, &semaphoreInfo, nullptr, &this->state.frameData[i].presentSemaphore) == VK_SUCCESS, "Failed to create image available semaphore");
			CS_ASSERT(vkCreateSemaphore(this->device, &semaphoreInfo, nullptr, &this->state.frameData[i].renderSemaphore) == VK_SUCCESS, "Failed to create render finished semaphore");
		}
		this->mainDeletionQueue.Push([&]() {
			for (auto& frame : this->state.frameData)
			{
				vkDestroySemaphore(this->device, frame.presentSemaphore, nullptr);
				vkDestroySemaphore(this->device, frame.renderSemaphore, nullptr);
			}
		});
	}
	void Renderer::RendererImpl::InitialiseRenderpasses(const BuilderInfo& info)
	{
		bool isMultiSampling = sampleCountMap(this->state.msaaSamples) != 1;

		// Create the default renderpass
		VkAttachmentDescription colorAttachment = Create::AttachmentDescription(
			0, this->swapchain.imageFormat, this->state.msaaSamples,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, (!isMultiSampling) ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);
		VkAttachmentDescription depthAttachment = Create::AttachmentDescription(
			0, DEFAULT_DEPTH_FORMAT, this->state.msaaSamples,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);
		VkAttachmentDescription colorAttachmentResolve = Create::AttachmentDescription(
			0, this->swapchain.imageFormat, VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		);

		VkAttachmentReference colorAttachmentRef = Create::AttachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkAttachmentReference depthAttachmentRef = Create::AttachmentReference(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		VkAttachmentReference colorAttachmentResolveRef = Create::AttachmentReference(2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		constexpr VkSubpassDependency colorDependency = Create::SubpassDependency(
			VK_SUBPASS_EXTERNAL, 0,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0
		);
		constexpr VkSubpassDependency depthDependency = Create::SubpassDependency(
			VK_SUBPASS_EXTERNAL, 0,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, 0
		);
		std::vector<VkAttachmentDescription> attachments{ colorAttachment, depthAttachment };
		if (sampleCountMap(this->state.msaaSamples) != 1) attachments.push_back(colorAttachmentResolve);
		std::vector<VkSubpassDependency> dependencies{ colorDependency, depthDependency };
		const VkSubpassDescription subpass = Create::SubpassDescription(
			0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &colorAttachmentRef, (isMultiSampling) ? &colorAttachmentResolveRef : nullptr, &depthAttachmentRef, 0, nullptr
		);
		this->defaultRenderPass = this->CreateRenderPass(attachments, { subpass }, dependencies);

		this->mainDeletionQueue.Push([&]() {
			vkDestroyRenderPass(this->device, this->defaultRenderPass, nullptr);
		});
	}
	void Renderer::RendererImpl::InitialiseFramebuffers(const BuilderInfo& info)
	{
		// Create msaa buffer
		VkImageCreateInfo multisamplingImageInfo = Create::ImageCreateInfo(
			nullptr, 0, VK_IMAGE_TYPE_2D, this->swapchain.imageFormat, this->swapchain.GetExtent3D(), 1, 1, this->state.msaaSamples, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED
		);
		VkImageViewCreateInfo multisamplingImageViewInfo = Create::ImageViewCreateInfo(
			0, nullptr, VK_IMAGE_VIEW_TYPE_2D, this->swapchain.imageFormat, {}, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
		);
		this->multisamplingBuffer = this->allocator.CreateImage(multisamplingImageInfo, multisamplingImageViewInfo, VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// Create the depth buffer
		VkExtent3D depthImageExtent = this->swapchain.GetExtent3D();
		VkImageCreateInfo depthImageInfo = Create::ImageCreateInfo(
			nullptr, 0, VK_IMAGE_TYPE_2D, DEFAULT_DEPTH_FORMAT, depthImageExtent, 1, 1, this->state.msaaSamples, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED
		);
		VkImageViewCreateInfo depthImageViewInfo = Create::ImageViewCreateInfo(
			0, nullptr, VK_IMAGE_VIEW_TYPE_2D, DEFAULT_DEPTH_FORMAT, {}, { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 }
		);
		this->depthBuffer = this->allocator.CreateImage(depthImageInfo, depthImageViewInfo, VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		
		bool isMultiSampling = sampleCountMap(this->state.msaaSamples) != 1;
		uint32_t swapChainViewIndex = isMultiSampling ? 2 : 0;
		
		// Add attachments
		std::vector<VkImageView> attachments;
		if (isMultiSampling) attachments.push_back(this->multisamplingBuffer.imageView);
		attachments.push_back(isMultiSampling ? this->depthBuffer.imageView : nullptr);
		attachments.push_back(isMultiSampling ? nullptr : this->depthBuffer.imageView);

		for (const auto& swapChainImage : this->swapchain.images)
		{
			attachments[swapChainViewIndex] = swapChainImage.imageView;
			VkFramebufferCreateInfo framebufferInfo = Create::FramebufferCreateInfo(
				nullptr, 0, this->defaultRenderPass, attachments.size(), attachments.data(), {info.windowExtent.width, info.windowExtent.height}, 1
			);
			VkFramebuffer fb;
			CS_ASSERT(vkCreateFramebuffer(this->device, &framebufferInfo, nullptr, &fb) == VK_SUCCESS, "Failed to create framebuffer!");
			this->framebuffers.push_back(fb);
		}
		// Add to swapchain deletion queue because it can be destroyed by window resizes
		this->swapChainDeletionQueue.Push([&]() {
			for (auto& framebuffer : this->framebuffers) vkDestroyFramebuffer(device, framebuffer, nullptr);
			this->framebuffers.clear();
			this->allocator.DestroyImage(this->depthBuffer);
			this->allocator.DestroyImage(this->multisamplingBuffer);
		});
	}
	void Renderer::RendererImpl::InitialiseDescriptors(const BuilderInfo& info)
	{
		this->descriptorManager = internal::DescriptorManager(this->device).Initialise(info.desriptorSetsPerPool);

		// Create the default texture descriptor set layout
		std::vector<VkDescriptorSetLayoutBinding> samplerLayoutBinding = { Create::DescriptorSetLayoutBinding(
			0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr
		) };
		VkDescriptorSetLayoutCreateInfo layoutInfo = Create::DescriptorSetLayoutCreateInfo(samplerLayoutBinding);
		vkCreateDescriptorSetLayout(this->device, &layoutInfo, nullptr, &this->textureDescriptorSetLayout);

		this->mainDeletionQueue.Push([&]() {
			for (auto& sampler : this->samplers) vkDestroySampler(this->device, sampler, nullptr);
			vkDestroyDescriptorSetLayout(this->device, this->textureDescriptorSetLayout, nullptr);

			for (auto& descriptorSetLayout : this->dataDescriptorSetLayouts) vkDestroyDescriptorSetLayout(this->device, descriptorSetLayout, nullptr);
			this->descriptorManager.Destroy();
		});
	}
	void Renderer::RendererImpl::InitialisePipelines(const BuilderInfo& info)
	{
		this->mainDeletionQueue.Push([&]() {
			for (auto& pipeline : this->pipelines) vkDestroyPipeline(this->device, pipeline.pipeline, nullptr);
			for (auto& pipelineLayout : this->pipelineLayouts) vkDestroyPipelineLayout(this->device, pipelineLayout, nullptr);
		});
	}
	void Renderer::RendererImpl::InitialiseBuffers(const BuilderInfo& info)
	{
		constexpr size_t INDICES = 0, POSITION = 1, NORMALS = 2, TEXTURE_UVS = 3;

		this->vertexBuffers.resize(4);

		this->offsets = std::vector<uint32_t>(1, 0);
		this->indiceOffsets = std::vector<uint32_t>(1, 0);

		this->vertexBuffers[INDICES]     = this->allocator.CreateBuffer(info.vertexBufferBlockSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_DST_BIT,  VMA_MEMORY_USAGE_GPU_ONLY);
		this->vertexBuffers[POSITION]    = this->allocator.CreateBuffer(info.vertexBufferBlockSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,  VMA_MEMORY_USAGE_GPU_ONLY);
		this->vertexBuffers[NORMALS]     = this->allocator.CreateBuffer(info.vertexBufferBlockSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,  VMA_MEMORY_USAGE_GPU_ONLY);
		this->vertexBuffers[TEXTURE_UVS] = this->allocator.CreateBuffer(info.vertexBufferBlockSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,  VMA_MEMORY_USAGE_GPU_ONLY);
	
		// Descriptor buffer initialisation
		for (uint32_t i = 0; i < info.framesInFlight; i++)
		{
			this->dataDescriptorSetBuffers.push_back(this->allocator.CreateBuffer(info.descriptorBufferBlockSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU));
		}

		this->mainDeletionQueue.Push([&]() {
			// Delete vertex buffers
			for (auto& vertexBuffer : this->vertexBuffers) this->allocator.DestroyBuffer(vertexBuffer);
			// Delete descriptor buffers
			for (auto& descriptorSetBuffer : this->dataDescriptorSetBuffers) this->allocator.DestroyBuffer(descriptorSetBuffer);
			// Delete texture buffers
			for (auto& image : this->images) this->allocator.DestroyImage(image);
		});
	}
}