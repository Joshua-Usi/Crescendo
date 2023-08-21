#include "RendererImpl.hpp"

namespace Crescendo
{
	constexpr vkb::PreferredDeviceType DEVICE_TYPE_MAPPING[2] = { vkb::PreferredDeviceType::discrete, vkb::PreferredDeviceType::integrated };
	constexpr VkPresentModeKHR PRESENT_MODE_MAPPING[2] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR };
	constexpr VkFormat DEFAULT_DEPTH_FORMAT = VK_FORMAT_D24_UNORM_S8_UINT;

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

		// Select physical device
		const vkb::PhysicalDevice physicalDeviceResult = vkb::PhysicalDeviceSelector(instance)
			.set_minimum_version(1, 3)
			.prefer_gpu_device_type(DEVICE_TYPE_MAPPING[static_cast<uint32_t>(info.preferredDeviceType)])
			.set_surface(this->surface)
			.set_required_features(deviceFeatures)
			.select().value();
		this->physicalDevice = physicalDeviceResult.physical_device;
		this->physicalDeviceProperties = physicalDeviceResult.properties;

		// Create logical device
		const vkb::Device deviceResult = vkb::DeviceBuilder(physicalDeviceResult).build().value();
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

		std::cout << "Renderer Stats:" << "\n";
		std::cout << "\tDevice: " << physicalDeviceResult.name << "\n";
		std::cout << "\tFrames in flight: " << info.framesInFlight << "\n";
		std::cout << "\tVertex Buffer Block Size: 4 x " << info.vertexBufferBlockSize / 1024 / 1024 << "MB\n";
		std::cout << "\tDescriptor Buffer Block Size: " << info.framesInFlight << " x " << info.descriptorBufferBlockSize / 1024 << "KB\n";
		std::cout << "\tMinimum uniform buffer offset alignment: " << this->physicalDeviceProperties.limits.minUniformBufferOffsetAlignment << "B\n";

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

		// Create the depth buffer
		VkExtent3D depthImageExtent = this->swapchain.GetExtent3D();
		VkImageCreateInfo depthImageInfo = Create::ImageCreateInfo(
			nullptr, 0, VK_IMAGE_TYPE_2D, DEFAULT_DEPTH_FORMAT, depthImageExtent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED
		);
		VkImageViewCreateInfo depthImageViewInfo = Create::ImageViewCreateInfo(
			0, nullptr, VK_IMAGE_VIEW_TYPE_2D, DEFAULT_DEPTH_FORMAT, {}, { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 }
		);

		this->depthBuffer = this->allocator.CreateImage(depthImageInfo, depthImageViewInfo, VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// Add to swapchain deletion queue
		this->swapChainDeletionQueue.Push([&]() {
			this->allocator.DestroyImage(this->depthBuffer);
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
		// Create the default renderpass
		VkAttachmentDescription colorAttachment = Create::AttachmentDescription(
			0, this->swapchain.imageFormat, VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		);
		VkAttachmentDescription depthAttachment = Create::AttachmentDescription(
			0, DEFAULT_DEPTH_FORMAT, VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);
		VkAttachmentReference colorAttachmentRef = Create::AttachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkAttachmentReference depthAttachmentRef = Create::AttachmentReference(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
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
		std::vector<VkSubpassDependency> dependencies{ colorDependency, depthDependency };
		const VkSubpassDescription subpass = Create::SubpassDescription(
			0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &colorAttachmentRef, nullptr, &depthAttachmentRef, 0, nullptr
		);
		this->defaultRenderPass = this->CreateRenderPass(attachments, { subpass }, dependencies);

		this->mainDeletionQueue.Push([&]() {
			vkDestroyRenderPass(this->device, this->defaultRenderPass, nullptr);
		});
	}
	void Renderer::RendererImpl::InitialiseFramebuffers(const BuilderInfo& info)
	{
		for (size_t i = 0, size = this->swapchain.images.size(); i < size; i++)
		{
			const std::vector<VkImageView> attachments = { this->swapchain.images[i].imageView, this->depthBuffer.imageView };
			VkFramebufferCreateInfo framebufferInfo = Create::FramebufferCreateInfo(
				nullptr, 0, this->defaultRenderPass, attachments.size(), attachments.data(), { info.windowExtent.width, info.windowExtent.height }, 1
			);

			VkFramebuffer fb;
			CS_ASSERT(vkCreateFramebuffer(this->device, &framebufferInfo, nullptr, &fb) == VK_SUCCESS, "Failed to create framebuffer!");
			this->framebuffers.push_back(fb);
		}
		// Add to swapchain deletion queue because it can be destroyed by window resizes
		this->swapChainDeletionQueue.Push([&]() {
			for (auto& framebuffer : this->framebuffers) vkDestroyFramebuffer(device, framebuffer, nullptr);
			this->framebuffers.clear();
		});
	}
	void Renderer::RendererImpl::InitialiseDescriptors(const BuilderInfo& info)
	{
		constexpr uint32_t SETS_PER_POOL = 128;
		this->descriptorManager = internal::DescriptorManager(this->device).Initialise(SETS_PER_POOL);
		this->mainDeletionQueue.Push([&]() {
			for (auto& descriptorSetLayout : this->descriptorSetLayouts) vkDestroyDescriptorSetLayout(this->device, descriptorSetLayout, nullptr);
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
			this->descriptorSetBuffers.push_back(this->allocator.CreateBuffer(info.descriptorBufferBlockSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU));
		}

		this->mainDeletionQueue.Push([&]() {
			// Delete vertex buffers
			for (auto& vertexBuffer : this->vertexBuffers) this->allocator.DestroyBuffer(vertexBuffer);
			// Delete descriptor buffers
			for (auto& descriptorSetBuffer : this->descriptorSetBuffers) this->allocator.DestroyBuffer(descriptorSetBuffer);
			// Delete texture buffers
			for (auto& image : this->images) this->allocator.DestroyImage(image);
		});
	}
}