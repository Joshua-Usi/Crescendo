#pragma once

#include "common.hpp"

#include "../Vulkan/Instance.hpp"
#include "../Vulkan/Device.hpp"
#include "../Vulkan/Swapchain.hpp"
#include "cs_std/packed_vector.hpp"
#include "../Vulkan/Types/Types.hpp"

#include "cs_std/image.hpp"

#define CS_VULKAN_HANDLE(name, type, storageName) \
	class name\
	{\
	private:\
		VulkanInstance* instance; uint32_t id;\
	public:\
		name() : instance(nullptr), id(0) {};\
		name(VulkanInstance* instance, uint32_t id) : instance(instance), id(id) {};\
		operator uint32_t() const { return id; };\
		operator type& () { return instance->storageName[id]; };\
		operator const type& () const { return instance->storageName[id]; };\
	}

CS_NAMESPACE_BEGIN
{

	struct VulkanInstanceSpecification
	{
		bool enableValidationLayers;
		std::string appName;
		std::string engineName;
		void* window;
		uint32_t descriptorSetsPerPool;
		uint32_t framesInFlight;
		uint32_t anisotropicSamples;
		uint32_t multisamples;
		float renderScale;
	};

	struct SamplableFramebuffer
	{
		VkSampler sampler;
		std::vector<uint32_t> textureIndices;
		uint32_t framebufferIndex;

		SamplableFramebuffer() : sampler(nullptr), textureIndices(0), framebufferIndex(0) {}
	};

	enum class Colorspace : uint8_t { SRGB, Linear };

	class VulkanInstance
	{
	public:
		// Fixed setup
		Vulkan::Instance instance;
		Vulkan::Device* deviceRef;
		Vulkan::TransferCommandQueue transferQueue;

		// Variable setup
		uint8_t frameIndex = 0;
		std::vector<Vulkan::RenderCommandQueue> renderCommandQueues;

		// Resources
		cs_std::packed_vector<Vulkan::RenderPass> renderPasses;
		cs_std::packed_vector<Vulkan::Framebuffer> framebuffers;
		cs_std::packed_vector<Vulkan::Pipelines> pipelines;
		cs_std::packed_vector<Vulkan::Mesh> meshes;
		cs_std::packed_vector<Vulkan::Texture> textures;
		cs_std::packed_vector<Vulkan::SSBO> SSBOs;
		cs_std::packed_vector<SamplableFramebuffer> samplableFramebuffers;
		std::vector<VkSampler> samplers;

		uint32_t depthPrepassIdx, offscreenIdx;

		uint32_t defaultRenderPassIdx, postProcessRenderPassIdx, shadowMapRenderPassIdx, depthPrepassRenderPassIdx;

		 VulkanInstanceSpecification specs;
	public:
		// Constructors
		VulkanInstance() = default;
		VulkanInstance(const VulkanInstanceSpecification& spec);
		// No copy
		VulkanInstance(const VulkanInstance&) = delete;
		VulkanInstance& operator=(const VulkanInstance&) = delete;
		// Move
		VulkanInstance(VulkanInstance&& other) noexcept = default;
		VulkanInstance& operator=(VulkanInstance&& other) noexcept = default;
		// Destructor
		~VulkanInstance();
	public:
		void CreateSwapchain();
		uint32_t CreateOffscreen(VkRenderPass pass, VkFormat colorFormat, uint32_t depthTextureIndex, VkSampleCountFlagBits multisamples, uint32_t width, uint32_t height);
		uint32_t CreateShadowMap(VkRenderPass renderPass, VkFormat format, uint32_t width, uint32_t height);
		uint32_t CreateDepthPrepass(VkRenderPass renderPass, VkFormat format, VkSampleCountFlagBits multisamples, uint32_t width, uint32_t height);
		uint32_t CreateSSBO(size_t size, VkShaderStageFlags shaderStage);

		uint32_t UploadMesh(const cs_std::graphics::mesh& mesh);
		uint32_t UploadTexture(const cs_std::image& image, Colorspace colorSpace = Colorspace::SRGB, bool generateMipmaps = false);

		Vulkan::RenderCommandQueue& GetCurrentRenderCommandQueue() { return renderCommandQueues[frameIndex]; }
		void NextFrame() { frameIndex = (frameIndex + 1) % specs.framesInFlight; }
	};

	CS_VULKAN_HANDLE(TextureHandle, Vulkan::Texture, textures);
	CS_VULKAN_HANDLE(MeshHandle, Vulkan::Mesh, meshes);
	CS_VULKAN_HANDLE(PipelinesHandle, Vulkan::Pipelines, pipelines);
	CS_VULKAN_HANDLE(FramebufferHandle, Vulkan::Framebuffer, framebuffers);
	CS_VULKAN_HANDLE(SSBOHandle, Vulkan::SSBO, SSBOs);
}