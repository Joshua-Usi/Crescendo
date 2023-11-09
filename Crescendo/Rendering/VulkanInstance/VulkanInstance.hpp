#pragma once

#include "../Vulkan/Instance.hpp"
#include "cs_std/packed_vector.hpp"
#include "../Vulkan/Types/Types.hpp"

#include "cs_std/image.hpp"

namespace Crescendo
{
	class Texture
	{
	private:
		uint32_t id;
	};

	class Mesh
	{
	private:
		uint32_t id;
	};

	class Model
	{
	private:
		std::vector<Mesh> meshes;
	};

	class Pipeline
	{
	private:
		uint32_t id;
	};

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
		std::vector<Vulkan::Frame> frameData;
		Vulkan::Swapchain swapchain;

		// Resources
		cs_std::packed_vector<Vulkan::RenderPass> renderPasses;
		cs_std::packed_vector<Vulkan::Framebuffer> framebuffers;
		cs_std::packed_vector<Vulkan::Pipelines> pipelines;
		cs_std::packed_vector<Vulkan::Mesh> meshes;
		cs_std::packed_vector<Vulkan::Texture> textures;
		std::vector<VkSampler> samplers;

		std::vector<Vulkan::SSBO> ssbo;

		Vulkan::Image depthBuffer;
		Vulkan::Texture shadowMap;
		VkSampler shadowMapSampler;
		Vulkan::Framebuffer shadowMapFramebuffer;

		struct Specifications
		{
			uint32_t framesInFlight;
			uint32_t anisotropicSamples;
			uint32_t multisamples;
		} specs;
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

		Vulkan::Mesh UploadMesh(const cs_std::graphics::mesh& mesh);
		Vulkan::Texture UploadTexture(const cs_std::image& image, bool generateMipmaps);
	};
}