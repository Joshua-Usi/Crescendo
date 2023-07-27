#include "RendererImpl.hpp"

#define VMA_IMPLEMENTATION
#include "VMA/vk_mem_alloc.h"
#include "internal/Create.hpp"

namespace Crescendo
{
	constexpr vkb::PreferredDeviceType DEVICE_TYPE_MAPPING[2] = { vkb::PreferredDeviceType::discrete, vkb::PreferredDeviceType::integrated };
	constexpr VkPresentModeKHR PRESENT_MODE_MAPPING[2] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR };
	constexpr VkFormat DEFAULT_DEPTH_FORMAT = VK_FORMAT_D32_SFLOAT;

	namespace Create = internal::Create;
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

		// Select physical device
		const vkb::PhysicalDevice physicalDeviceResult = vkb::PhysicalDeviceSelector(instance)
			.set_minimum_version(1, 3)
			.prefer_gpu_device_type(DEVICE_TYPE_MAPPING[static_cast<uint32_t>(info.preferredDeviceType)])
			.set_surface(this->surface)
			.select().value();
		this->physicalDevice = physicalDeviceResult.physical_device;

		// Create logical device
		const vkb::Device deviceResult = vkb::DeviceBuilder(physicalDeviceResult).build().value();
		this->device = deviceResult.device;

		// Find queues
		this->queues.GetQueues(deviceResult);

		// Initialise VMA
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = this->physicalDevice;
		allocatorInfo.device = this->device;
		allocatorInfo.instance = this->instance;
		vmaCreateAllocator(&allocatorInfo, &this->allocator);

		// Push to deletion queue
		this->mainDeletionQueue.Push([&]() {
			this->swapChainDeletionQueue.Flush();
			vmaDestroyAllocator(this->allocator);
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
		std::vector<Image> viewableImages(images.size());
		for (size_t i = 0; i < images.size(); i++) viewableImages[i] = Image(images[i], imageViews[i]);
		this->swapchain = Swapchain(vkbSwapchain.swapchain, vkbSwapchain.image_format, vkbSwapchain.extent, viewableImages);

		// Create the depth buffer
		VkExtent3D depthImageExtent = this->swapchain.GetExtent3D();
		VkImageCreateInfo depthImageInfo = Create::ImageCreateInfo(
			nullptr, 0, VK_IMAGE_TYPE_2D, DEFAULT_DEPTH_FORMAT, depthImageExtent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED
		);
		VmaAllocationCreateInfo depthImageAllocInfo = {};
		depthImageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		depthImageAllocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		CS_ASSERT(vmaCreateImage(this->allocator, &depthImageInfo, &depthImageAllocInfo, &this->depthBuffer.image, &this->depthBuffer.allocation, nullptr) == VK_SUCCESS, "Failed to create depth image");
		VkImageViewCreateInfo depthImageViewInfo = Create::ImageViewCreateInfo(
			nullptr, 0, this->depthBuffer.image, VK_IMAGE_VIEW_TYPE_2D, DEFAULT_DEPTH_FORMAT, {}, { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 }
		);
		CS_ASSERT(vkCreateImageView(this->device, &depthImageViewInfo, nullptr, &this->depthBuffer.imageView) == VK_SUCCESS, "Failed to create depth image view!");

		// Add to swapchain deletion queue
		this->swapChainDeletionQueue.Push([&]() {
			vkDestroyImageView(this->device, this->depthBuffer.imageView, nullptr);
			vmaDestroyImage(this->allocator, this->depthBuffer.image, this->depthBuffer.allocation);
			for (auto& image : this->swapchain.images) vkDestroyImageView(this->device, image.imageView, nullptr);
			vkDestroySwapchainKHR(this->device, this->swapchain.swapchain, nullptr);
		});
	}
	void Renderer::RendererImpl::InitialiseCommands(const BuilderInfo& info)
	{
		// Create the command pool
		VkCommandPoolCreateInfo commandPoolInfo = Create::CommandPoolCreateInfo(this->queues.universalFamily);
		CS_ASSERT(vkCreateCommandPool(this->device, &commandPoolInfo, nullptr, &this->state.frameData.commandPool) == VK_SUCCESS, "Failed to create command pool!");
	
		// Create the command buffer
		VkCommandBufferAllocateInfo cmdBufferInfo = Create::CommandBufferAllocateInfo(this->state.frameData.commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
		CS_ASSERT(vkAllocateCommandBuffers(this->device, &cmdBufferInfo, &this->state.frameData.commandBuffer) == VK_SUCCESS, "Failed to allocate command buffers!");

		this->mainDeletionQueue.Push([&]() {
			vkDestroyCommandPool(this->device, this->state.frameData.commandPool, nullptr);
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
	void Renderer::RendererImpl::InitialiseSyncStructures(const BuilderInfo& info)
	{
		// Create sync structures for frames in flight
		const VkFenceCreateInfo fenceInfo = Create::FenceCreateInfo(true);
		const VkSemaphoreCreateInfo semaphoreInfo = Create::SemaphoreCreateInfo();
		
		CS_ASSERT(vkCreateSemaphore(this->device, &semaphoreInfo, nullptr, &this->state.frameData.presentSemaphore) == VK_SUCCESS, "Failed to create image available semaphore");
		CS_ASSERT(vkCreateSemaphore(this->device, &semaphoreInfo, nullptr, &this->state.frameData.renderSemaphore) == VK_SUCCESS, "Failed to create render finished semaphore");
		CS_ASSERT(vkCreateFence(this->device, &fenceInfo, nullptr, &this->state.frameData.renderFence) == VK_SUCCESS, "Failed to create in flight fences");
	
		this->mainDeletionQueue.Push([&]() {
			vkDestroySemaphore(this->device, this->state.frameData.presentSemaphore, nullptr);
			vkDestroySemaphore(this->device, this->state.frameData.renderSemaphore, nullptr);
			vkDestroyFence(this->device, this->state.frameData.renderFence, nullptr);
		});
	}
	void Renderer::RendererImpl::InitialisePipelines(const BuilderInfo& info)
	{
		this->pipelineLayouts.resize(info.shaderData.size());
		this->pipelines.resize(info.shaderData.size());
		
		for (size_t i = 0, size = info.shaderData.size(); i < size; i++)
		{
			const BuilderInfo::ShaderStageData shaders = info.shaderData[i];
			// Load shader modules
			VkShaderModule vertShaderModule = this->CreateShaderModule(shaders.vertex);
			VkShaderModule fragShaderModule = this->CreateShaderModule(shaders.fragment);

			// Get binding and attribute descriptions
			std::vector<VkVertexInputBindingDescription> bindingDescriptions = shaders.vertexReflection.GetVertexBindings();
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions = shaders.vertexReflection.GetVertexAttributes();

			// Get push constant range
			VkPushConstantRange pushConstantRange = shaders.vertexReflection.GetPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT);

			// Create the pipeline layout
			VkPipelineLayoutCreateInfo pipelineLayoutInfo = Create::PipelineLayoutCreateInfo(0, 0, nullptr, 1, &pushConstantRange);
			CS_ASSERT(vkCreatePipelineLayout(this->device, &pipelineLayoutInfo, nullptr, &this->pipelineLayouts[i]) == VK_SUCCESS, "Failed to create pipeline layout!");

			// We we always use dyanmic states, there is no performance penalty for just viewports and scissors
			const std::vector<VkDynamicState> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
			// Create the information required to build the pipeline
			PipelineBuilderInfo pipelineBuilderInfo = {};
			pipelineBuilderInfo.dynamicState = Create::PipelineDynamicStateCreateInfo(
				dynamicStates.size(), dynamicStates.data()
			);
			pipelineBuilderInfo.renderPass = this->defaultRenderPass;
			pipelineBuilderInfo.shaderStagesInfo.push_back(Create::PipelineShaderStageCreateInfo(
				nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, vertShaderModule, "main", nullptr
			));
			pipelineBuilderInfo.shaderStagesInfo.push_back(Create::PipelineShaderStageCreateInfo(
				nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderModule, "main", nullptr
			));
			pipelineBuilderInfo.vertexInputInfo = Create::PipelineVertexInputStateCreateInfo(
				nullptr, static_cast<uint32_t>(bindingDescriptions.size()), bindingDescriptions.data(), static_cast<uint32_t>(attributeDescriptions.size()), attributeDescriptions.data()
			);
			pipelineBuilderInfo.inputAssemblyInfo = Create::PipelineInputAssemblyStateCreateInfo(
				VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE
			);
			pipelineBuilderInfo.rasterizerInfo = Create::PipelineRasterizationStateCreateInfo(
				nullptr, VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE,
				VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f
			);
			pipelineBuilderInfo.multisamplingInfo = Create::PipelineMultisampleStateCreateInfo(
				nullptr, VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 1.0f, nullptr, VK_FALSE, VK_FALSE
			);
			pipelineBuilderInfo.colorBlendAttachment = Create::PipelineColorBlendAttachmentState(
				VK_TRUE, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
			);
			pipelineBuilderInfo.depthStencilInfo = Create::PipelineDepthStencilStateCreateInfo(
				0, VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS, VK_FALSE, VK_FALSE, {}, {}, 0.0f, 1.0f
			);
			pipelineBuilderInfo.pipelineLayout = this->pipelineLayouts[i];

			// Create the pipeline
			this->pipelines[i] = this->CreatePipeline(pipelineBuilderInfo);

			// Destroy shader modules
			vkDestroyShaderModule(this->device, fragShaderModule, nullptr);
			vkDestroyShaderModule(this->device, vertShaderModule, nullptr);
		}

		this->mainDeletionQueue.Push([&]() {
			for (auto& pipeline : this->pipelines) vkDestroyPipeline(this->device, pipeline, nullptr);
			for (auto& pipelineLayout : this->pipelineLayouts) vkDestroyPipelineLayout(this->device, pipelineLayout, nullptr);
			this->pipelines.clear();
			this->pipelineLayouts.clear();
		});
	}
	void Renderer::RendererImpl::InitialiseBuffers(const BuilderInfo& info)
	{
		constexpr size_t INDICES_PER_TRIANGLE =  3;
		constexpr size_t VERTICES_PER_TRIANGLE = 3 * INDICES_PER_TRIANGLE;
		constexpr size_t NORMALS_PER_TRIANGLE =  3 * INDICES_PER_TRIANGLE;
		constexpr size_t UVS_PER_TRAINGLE =      2 * INDICES_PER_TRIANGLE;

		this->positionBuffer =  this->CreateBuffer(sizeof(float) *    VERTICES_PER_TRIANGLE  * info.triangleBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		this->normalBuffer =    this->CreateBuffer(sizeof(float) *    NORMALS_PER_TRIANGLE   * info.triangleBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		this->textureUVBuffer = this->CreateBuffer(sizeof(float) *	  UVS_PER_TRAINGLE       * info.triangleBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		this->indexBuffer =		this->CreateBuffer(sizeof(uint32_t) * INDICES_PER_TRIANGLE   * info.triangleBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,  VMA_MEMORY_USAGE_CPU_TO_GPU);
	
		this->mainDeletionQueue.Push([&]() {
			vmaUnmapMemory(this->allocator, this->positionBuffer.allocation);
			vmaUnmapMemory(this->allocator, this->normalBuffer.allocation);
			vmaUnmapMemory(this->allocator, this->textureUVBuffer.allocation);
			vmaUnmapMemory(this->allocator, this->indexBuffer.allocation);

			vmaDestroyBuffer(this->allocator, this->positionBuffer.buffer,  this->positionBuffer.allocation);
			vmaDestroyBuffer(this->allocator, this->normalBuffer.buffer,    this->normalBuffer.allocation);
			vmaDestroyBuffer(this->allocator, this->textureUVBuffer.buffer, this->textureUVBuffer.allocation);
			vmaDestroyBuffer(this->allocator, this->indexBuffer.buffer,     this->indexBuffer.allocation);
		});
	}
}