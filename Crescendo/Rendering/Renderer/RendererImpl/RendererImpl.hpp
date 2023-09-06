#pragma once

#include "Core/common.hpp"

#include "Vulkan/Vulkan.hpp"
#include "VkBootstrap/VkBootstrap.h"
#include "GLFW/glfw3.h"

#include "Rendering/Renderer/Renderer.hpp"
#include "internal/FunctionQueue.hpp"
#include "internal/QueueManager.hpp"
#include "internal/CommandQueue.hpp"
#include "internal/Allocator.hpp"
#include "internal/DescriptorManager.hpp"

#include "./internal/Create.hpp"

#include <functional>

namespace Crescendo
{
	namespace Create = internal::Create;
	struct Swapchain
	{
		VkSwapchainKHR swapchain;
		VkFormat imageFormat;
		VkExtent2D extent;
		std::vector<internal::Allocator::Image> images;
		inline Swapchain(VkSwapchainKHR swapchain = nullptr, VkFormat imageFormat = {}, VkExtent2D extent = {}, const std::vector<internal::Allocator::Image>& images = {})
			: swapchain(swapchain), imageFormat(imageFormat), extent(extent), images(images) {}
		inline const VkExtent2D& GetExtent() const { return extent; }
		inline VkExtent3D GetExtent3D() const { return { extent.width, extent.height, 1 }; }
	};

	struct FrameData
	{
		internal::CommandQueue commandQueue;
		VkSemaphore presentSemaphore, renderSemaphore;
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

		VkSampleCountFlagBits msaaSamples;

		bool didFramebufferResize;

		inline RendererState() :
			frameData({}),
			framesInFlight(0),
			frameIndex(0),
			swapchainImageIndex(0),
			boundPipelineIndex(0),
			didFramebufferResize(false),
			msaaSamples(VK_SAMPLE_COUNT_1_BIT)
		{};
	};

	struct Pipeline
	{
		VkPipelineLayout layout;
		VkPipeline pipeline;
		// Shows the associated data descriptor layouts, sets and offsets
		std::vector<uint32_t> dataDescriptorHandles;
		// Set number of the sampler descriptor set
		uint32_t samplerDescriptorSet;

		inline Pipeline(VkPipelineLayout layout, VkPipeline pipeline, std::vector<uint32_t> dataDescriptorHandles, uint32_t samplerDescriptorSet) : layout(layout), pipeline(pipeline), dataDescriptorHandles(dataDescriptorHandles), samplerDescriptorSet(samplerDescriptorSet) {}
	};

	struct Framebuffer
	{
		VkImage image;
		VkImageView imageView;
		VkDeviceMemory memory;
	};

	class Renderer::RendererImpl
	{
	public:
		VkPhysicalDeviceProperties physicalDeviceProperties;
		internal::FunctionQueue mainDeletionQueue, swapChainDeletionQueue;
		internal::Allocator allocator;
		RendererState state;

		// Instance related members
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		GLFWwindow* window;
		internal::QueueManager queues;

		Swapchain swapchain;
		internal::Allocator::Image depthBuffer,	multisamplingBuffer;
		VkRenderPass defaultRenderPass;
		std::vector<VkFramebuffer> framebuffers;

		// universal buffers that contain all the data for the respective vertex attribute
		// By default holds 4 buffers, one for each vertex attribute: position, normal, textureUV, indices
		std::vector<internal::Allocator::Buffer> vertexBuffers;
		// Offsets for the start of each meshes vertex data
		std::vector<uint32_t> offsets;
		// Offsets for each meshes indices
		std::vector<uint32_t> indiceOffsets;

		std::vector<VkPipelineLayout> pipelineLayouts;
		std::vector<Pipeline> pipelines;

		internal::DescriptorManager descriptorManager;

		// Data only descriptor layouts
		std::vector<VkDescriptorSetLayout> dataDescriptorSetLayouts;
		// Offsets of each binding within a specific layout
		std::vector<std::vector<uint32_t>> dataDescriptorSetLayoutOffsets;
		// One per frame in flight, each descriptor set is stored in a contiguous block of memory
		std::vector<internal::Allocator::Buffer> dataDescriptorSetBuffers;
		// Data descriptors for each frame in flight
		std::vector<VkDescriptorSet> dataDescriptorSets;

		VkDescriptorSetLayout textureDescriptorSetLayout;
		// One per mip level, dynamically created when a texture is loaded
		std::vector<VkSampler> samplers;

		// I don't know how to transfer queue ownership, so I'm just going to create a universal queue for uploading textures
		// TODO figure out how switch texture uploading to dedicated transfer queue
		internal::CommandQueue uploadQueue, uploadTextureQueue;

		// List of all the texture images that have been uploaded to the GPU.
		// TODO switch to single buffer for all images
		std::vector<internal::Allocator::Image> images;
		std::vector<VkDescriptorSet> imageDescriptorSets;
 
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
		void BindTexture(uint32_t textureIndex);
		void UpdatePushConstant(ShaderStage stage, const void* data, uint32_t size);
		void Draw(uint32_t mesh);
		void PresentFrame();
		void UpdateDescriptorSet(uint32_t descriptorSetIndex, uint32_t binding, const void* data, size_t size);

		// Creation abstraction
		VkShaderModule CreateShaderModule(const std::vector<uint8_t>& code);
		VkRenderPass CreateRenderPass(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& subpassDependencies);
		VkPipeline CreatePipeline(PipelineBuilderInfo& info);

		// Upload commands
		void UploadPipeline(const std::vector<uint8_t>& vertexShader, const std::vector<uint8_t>& fragmentShader, const PipelineVariant& variant);
		void UploadMesh(const std::vector<float>& vertices, const std::vector<float>& normals, const std::vector<float>& textureUVs, const std::vector<uint32_t>& indices);
		void UploadTexture(const std::vector<uint8_t>& textureData, uint32_t width, uint32_t height, uint32_t channels, bool generateMipmaps);
	
		// Getters
		inline Pipeline GetPipeline(uint32_t pipeline) const { return this->pipelines[pipeline]; }
	};
}