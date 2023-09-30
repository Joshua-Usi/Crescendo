 #pragma once

#include "Core/common.hpp"

#include "Vulkan/Vulkan.hpp"
#include "VkBootstrap/VkBootstrap.h"
#include "GLFW/glfw3.h"

#include "Rendering/Renderer/Renderer.hpp"
#include "internal/QueueManager.hpp"
#include "internal/CommandQueue.hpp"
#include "internal/Allocator/Allocator.hpp"
#include "internal/DescriptorManager.hpp"
#include "internal/Device/Device.hpp"

#include "Rendering/Reflection/Reflection.hpp"

#include "./internal/Create.hpp"

#include "cs_std/function_queue.hpp"

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

	struct RendererState
	{
		std::vector<FrameData> frameData;

		uint32_t frameIndex;
		uint32_t swapchainImageIndex;
		uint32_t boundPipelineIndex;

		VkSampleCountFlagBits msaaSamples;

		bool didFramebufferResize;

		inline RendererState() :
			frameData({}),
			frameIndex(0),
			swapchainImageIndex(0),
			boundPipelineIndex(0),
			didFramebufferResize(false),
			msaaSamples(VK_SAMPLE_COUNT_1_BIT)
		{};
	};
	
	// Minimum required data to build a pipeline from scratch
	// Also contains useful metadata about the pipeline
	struct PipelineData {
		SpirvReflection vertexReflection, fragmentReflection;
		VkShaderModule vertexShader, fragmentShader;

		inline PipelineData() = default;
		inline PipelineData(const SpirvReflection& vertexReflection, const SpirvReflection& fragmentReflection, VkShaderModule vertexShader, VkShaderModule fragmentShader) :
			vertexReflection(vertexReflection), fragmentReflection(fragmentReflection), vertexShader(vertexShader), fragmentShader(fragmentShader) {}

		inline void Destroy(VkDevice device) {
			vkDestroyShaderModule(device, this->vertexShader, nullptr);
			vkDestroyShaderModule(device, this->fragmentShader, nullptr);
		}
	};

	// Perfectly fits in 8 bits
	// Minimum required support by the GLTF format
	//enum class PipelineVertexAttribute : uint8_t
	//{
	//	Position = 0,
	//	Normal,
	//	Tangents,
	//	TexCoord0,
	//	TexCoord1,
	//	Color0,
	//	Joints0,
	//	Weights0,
	//};
		
	struct Pipeline
	{
		VkPipelineLayout layout;
		VkPipeline pipeline;
		// Shows the associated data descriptor layouts, sets and offsets
		std::vector<uint32_t> dataDescriptorHandles;
		// Set number of the sampler descriptor set
		std::vector<uint32_t> samplerDescriptorHandles;

		inline Pipeline(VkPipelineLayout layout, VkPipeline pipeline, const std::vector<uint32_t>& dataDescriptorHandles, const std::vector<uint32_t>& samplerDescriptorHandles) : layout(layout), pipeline(pipeline), dataDescriptorHandles(dataDescriptorHandles), samplerDescriptorHandles(samplerDescriptorHandles) {}
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
		BuilderInfo rendererInfo;
		VkPhysicalDeviceProperties physicalDeviceProperties;
		cs_std::function_queue mainDeletionQueue, swapChainDeletionQueue;
		internal::Allocator allocator;
		RendererState state;

		// Instance related members
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;
		internal::Device device;
		internal::QueueManager queues;

		Swapchain swapchain;
		internal::Allocator::Image depthBuffer,	multisamplingBuffer, shadowMapBuffer;
		VkRenderPass defaultRenderPass, shadowMapRenderPass;
		std::vector<VkFramebuffer> framebuffers;
		VkFramebuffer shadowMapFramebuffer;

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
		// One per mip level, dynamically created when a texture is loaded, supports non-mipped textures too
		std::vector<VkSampler> samplers;

		// I don't know how to transfer queue ownership, so I'm just going to use the universal queue for uploading textures
		// TODO figure out how switch texture uploading to dedicated transfer queue
		internal::CommandQueue uploadQueue, uploadTextureQueue;

		// List of all the texture images that have been uploaded to the GPU.
		std::vector<internal::Allocator::Image> images;
		std::vector<VkDescriptorSet> imageDescriptorSets;
 
	public:
		RendererImpl() = default;
		~RendererImpl() = default;

		inline uint32_t GetFrameIndex() const { return this->state.frameIndex; };
		FrameData& GetCurrentFrameData();

		void InitialiseInstance(const BuilderInfo& info);
		void InitialiseTransferQueues(const BuilderInfo& info);
		void InitialiseDescriptors(const BuilderInfo& info);
		void InitialisePipelines(const BuilderInfo& info);
		void InitialiseBuffers(const BuilderInfo& info);

		void InitialiseFlightFrames(const BuilderInfo& info);
		void InitialiseSwapchain(const BuilderInfo& info);
		void InitialiseRenderpasses(const BuilderInfo& info);
		void InitialiseFramebuffers(const BuilderInfo& info);

		void RecreateSwapchain();
		inline void Resize() { this->state.didFramebufferResize = true; }

		// Commands
		void BeginFrame(const VkClearValue& clearColor);
		void EndFrame();
		void BindPipeline(uint32_t pipelineIndex);
		void BindTexture(uint32_t set, uint32_t textureIndex);
		void UpdatePushConstant(ShaderStage stage, const void* data, uint32_t size);
		void Draw(uint32_t mesh);
		void PresentFrame();
		void UpdateDescriptorSet(uint32_t descriptorSetIndex, uint32_t binding, const void* data, size_t size);

		// Upload commands
		void UploadPipeline(const std::vector<uint8_t>& vertexShader, const std::vector<uint8_t>& fragmentShader, const PipelineVariant& variant);
		void UploadMesh(const std::vector<float>& vertices, const std::vector<float>& normals, const std::vector<float>& textureUVs, const std::vector<float>& tangents, const std::vector<uint32_t>& indices);
		void UploadTexture(const void* textureData, uint32_t width, uint32_t height, uint32_t channels, bool generateMipmaps);
	};
}