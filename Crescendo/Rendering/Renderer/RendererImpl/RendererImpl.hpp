#pragma once

#include "Core/common.hpp"

#include "Vulkan/Vulkan.hpp"
#include "VkBootstrap/VkBootstrap.h"
#include "VMA/vk_mem_alloc.h"
#include "GLFW/glfw3.h"

#include "Rendering/Renderer/Renderer.hpp"
#include "internal/DeletionQueue.hpp"
#include "internal/QueueManager.hpp"

namespace Crescendo
{
	struct Buffer
	{
		VkBuffer buffer;
		VmaAllocation allocation;
		void* memoryLocation;
	};

	struct Image
	{
		VkImage image;
		VkImageView imageView;
		VmaAllocation allocation;
	};

	struct Swapchain
	{
		VkSwapchainKHR swapchain;
		VkFormat imageFormat;
		VkExtent2D extent;
		std::vector<Image> images;
		inline Swapchain(VkSwapchainKHR swapchain = nullptr, VkFormat imageFormat = {}, VkExtent2D extent = {}, const std::vector<Image>& images = {})
			: swapchain(swapchain), imageFormat(imageFormat), extent(extent), images(images) {}
		inline const VkExtent2D& GetExtent() const { return extent; }
		inline VkExtent3D GetExtent3D() const { return { extent.width, extent.height, 1 }; }
	};

	struct FrameData
	{
		Buffer descriptorBuffer;
		VkDescriptorSet descriptorSet;

		VkSemaphore presentSemaphore, renderSemaphore;
		VkFence renderFence;

		VkCommandPool commandPool;
		VkCommandBuffer commandBuffer;
	};

	struct PipelineBuilderInfo
	{
		VkPipelineDynamicStateCreateInfo dynamicState;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStagesInfo;
		VkPipelineVertexInputStateCreateInfo vertexInputInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineTessellationStateCreateInfo tessellationInfo;
		VkPipelineRasterizationStateCreateInfo rasterizerInfo;
		VkPipelineMultisampleStateCreateInfo multisamplingInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		VkPipelineLayout pipelineLayout;
		VkRenderPass renderPass;
	};

	struct RendererState
	{
		std::vector<FrameData> frameData;

		uint32_t framesInFlight;
		uint32_t frameIndex;
		uint32_t swapchainImageIndex;
		uint32_t boundPipelineIndex;

		bool didFramebufferResize;

		inline RendererState() : frameData({}), framesInFlight(0), frameIndex(0), swapchainImageIndex(0), boundPipelineIndex(0), didFramebufferResize(false) {};
	};

	class Renderer::RendererImpl
	{
	public:
		internal::FunctionQueue mainDeletionQueue, swapChainDeletionQueue;

		VkPhysicalDeviceProperties physicalDeviceProperties;

		// Instance related members
		VmaAllocator allocator;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		GLFWwindow* window;

		internal::QueueManager queues;

		RendererState state;

		Swapchain swapchain;
		Image depthBuffer;
		VkRenderPass defaultRenderPass;
		std::vector<VkFramebuffer> framebuffers;

		std::vector<VkPipelineLayout> pipelineLayouts;
		std::vector<VkPipeline> pipelines;

		// universal buffers that contain all the data for the respective vertex attribute
		// By default holds 4 buffers, one for each vertex attribute: position, normal, textureUV, indices
		std::vector<Buffer> vertexBuffers;
		// Offsets for the start of each meshes vertex data
		std::vector<uint32_t> offsets;
		// Offsets for each meshes indices
		std::vector<uint32_t> indiceOffsets;

		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		// Offsets of each set within a specific layout
		std::vector<std::vector<uint32_t>> descriptorSetLayoutOffsets;
		// One per frame in flight, each descriptor set is stored in a contiguous block of memory
		std::vector<Buffer> descriptorSetBuffers;
		std::vector<VkDescriptorSet> descriptorSets;
 
	public:
		RendererImpl() = default;
		~RendererImpl() = default;

		inline uint32_t GetFrameIndex() const { return this->state.frameIndex; };
		FrameData& GetCurrentFrameData();

		void InitialiseInstance(const BuilderInfo& info);
		void InitialiseSwapchain(const BuilderInfo& info);
		void InitialiseCommands(const BuilderInfo& info);
		void InitialiseSyncStructures(const BuilderInfo& info);

		void InitialiseRenderpasses(const BuilderInfo& info); // Also initialises the default renderpass
		void InitialiseFramebuffers(const BuilderInfo& info);
		void InitialiseDescriptors(const BuilderInfo& info);
		void InitialisePipelines(const BuilderInfo& info);
		// Doesn't necessarily load buffers with data. But it does create and allocate them
		void InitialiseBuffers(const BuilderInfo& info);

		void RecreateSwapchain();
		inline void Resize() { this->state.didFramebufferResize = true; }

		// Commands
		void BeginFrame(const VkClearValue& clearColor);
		void EndFrame();
		void BindPipeline(uint32_t pipelineIndex);
		void UpdatePushConstant(ShaderStage stage, const void* data, size_t size);
		void Draw(uint32_t mesh);
		void PresentFrame();
		void UpdateDescriptorSet(uint32_t descriptorSetIndex, uint32_t binding, const void* data, size_t size);

		// Creation abstraction
		VkShaderModule CreateShaderModule(const std::vector<uint8_t>& code);
		VkRenderPass CreateRenderPass(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& subpassDependencies);
		VkPipeline CreatePipeline(PipelineBuilderInfo& info);
		Buffer CreateBuffer(size_t allocationSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

		// Upload commands
		void UploadMesh(const std::vector<float>& vertices, const std::vector<float>& normals, const std::vector<float>& textureUVs, const std::vector<uint32_t>& indices);
		void UploadPipeline(const std::vector<uint8_t>& vertexShader, const std::vector<uint8_t>& fragmentShader, const PipelineVariant& variant);
	};
}