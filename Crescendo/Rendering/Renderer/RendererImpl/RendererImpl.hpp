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

	class Renderer::RendererImpl
	{
	public:
		internal::FunctionQueue mainDeletionQueue, swapChainDeletionQueue;

		// Instance related members
		VmaAllocator allocator;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		internal::QueueManager queues;

		struct State
		{
			std::vector<FrameData> frameData;

			struct Temp
			{
				BuilderInfo::WindowExtent sizeToResize;
			} temp;

			uint32_t framesInFlight;
			uint32_t frameIndex;
			uint32_t swapchainImageIndex;
			uint32_t boundPipelineIndex;
			// Maximum number of triangles for each vertex buffer
			uint32_t maxBufferSize;

			bool didFramebufferResize = false;
		} state;

		Swapchain swapchain;
		Image depthBuffer;
		VkRenderPass defaultRenderPass;
		std::vector<VkFramebuffer> framebuffers;

		std::vector<VkPipelineLayout> pipelineLayouts;
		std::vector<VkPipeline> pipelines;

		// universal buffers that contain all the data for the respective vertex attribute
		// By default holds 4 buffers, one for each vertex attribute: position, normal, textureUV, indices
		std::vector<Buffer> vertexBuffers;

		// Offsets for the other buffers
		std::vector<uint32_t> offsets;
		// Stores offsets of a mesh's data in the universal buffers
		// Each unit is based on a triangle, this is the offset buffer for the indices
		std::vector<uint32_t> indiceOffsets;

		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
 
	public:
		RendererImpl() = default;
		~RendererImpl() = default;

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
		inline void Resize(const BuilderInfo::WindowExtent& windowExtent) { this->state.didFramebufferResize = true; this->state.temp.sizeToResize = windowExtent; }

		void BeginFrame(const VkClearValue& clearColor);
		void EndFrame();
		void BindPipeline(uint32_t pipelineIndex);
		void UpdatePushConstant(ShaderStage stage, const void* data, size_t size);
		void Draw(uint32_t mesh);
		void PresentFrame();

		VkShaderModule CreateShaderModule(const std::vector<uint8_t>& code);
		VkRenderPass CreateRenderPass(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& subpassDependencies);
		VkPipeline CreatePipeline(PipelineBuilderInfo& info);
		Buffer CreateBuffer(size_t allocationSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

		void UploadMesh(const std::vector<float>& vertices, const std::vector<float>& normals, const std::vector<float>& textureUVs, const std::vector<uint32_t>& indices);
	};
}