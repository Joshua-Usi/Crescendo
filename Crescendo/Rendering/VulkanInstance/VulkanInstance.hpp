#pragma once

#include "common.hpp"

#include "../Vulkan/Instance.hpp"
#include "cs_std/packed_vector.hpp"
#include "../Vulkan/Types/Types.hpp"

#include "cs_std/image.hpp"

CS_NAMESPACE_BEGIN
{
	class Texture { private: uint32_t id; };
	class Mesh { private: uint32_t id; };
	class Model { private: std::vector<Mesh> meshes; };
	class Pipeline { private: uint32_t id; };
	class Framebuffer { private: uint32_t id; };
	class RenderPass { private: uint32_t id; };

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

	enum class Colorspace
	{
		SRGB,
		Linear
	};

	class VulkanInstance
	{
	public:
		// Fixed setup
		Vulkan::Instance instance;
		Vulkan::Device device;
		Vulkan::TransferCommandQueue transferQueue;

		// Variable setup
		uint8_t frameIndex = 0;
		std::vector<Vulkan::RenderCommandQueue> renderCommandQueues;
		Vulkan::Swapchain swapchain;

		// Resources
		cs_std::packed_vector<Vulkan::RenderPass> renderPasses;
		cs_std::packed_vector<Vulkan::Framebuffer> framebuffers;
		cs_std::packed_vector<Vulkan::Pipelines> pipelines;
		cs_std::packed_vector<Vulkan::Mesh> meshes;
		cs_std::packed_vector<Vulkan::Texture> textures;
		std::vector<VkSampler> samplers;

		std::vector<Vulkan::SSBO> ssbo;

		SamplableFramebuffer shadowMap, depthPrepass, offscreen;

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
		SamplableFramebuffer CreateOffscreen(VkRenderPass pass, VkFormat colorFormat, uint32_t depthTextureIndex, VkSampleCountFlagBits multisamples, uint32_t width, uint32_t height);
		SamplableFramebuffer CreateShadowMap(VkRenderPass renderPass, VkFormat format, uint32_t width, uint32_t height);
		SamplableFramebuffer CreateDepthPrepass(VkRenderPass renderPass, VkFormat format, VkSampleCountFlagBits multisamples, uint32_t width, uint32_t height);

		Vulkan::Mesh UploadMesh(const cs_std::graphics::mesh& mesh);
		Vulkan::Texture UploadTexture(const cs_std::image& image, Colorspace colorSpace = Colorspace::SRGB, bool generateMipmaps = false);

		Vulkan::RenderCommandQueue& GetCurrentRenderCommandQueue() { return renderCommandQueues[frameIndex]; }
		void NextFrame() { frameIndex = (frameIndex + 1) % specs.framesInFlight; }
	};
}