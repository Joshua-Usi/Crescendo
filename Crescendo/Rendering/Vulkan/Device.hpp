#pragma once

#include "common.hpp"

#include "volk/volk.h"
#include "VkBootstrap/VkBootstrap.h"

#include "Allocator.hpp"
#include "CommandQueue.hpp"
#include "DescriptorManager.hpp"

#include "Types/Queues.hpp"
#include "Types/ShaderReflection.hpp"
#include "Types/Framebuffer.hpp"
#include "Types/RenderPass.hpp"
#include "Types/ShaderModule.hpp"
#include "Types/Buffer.hpp"
#include "Types/Image.hpp"
#include "Types/PipelineVariants.hpp"
#include "Types/RenderCommandQueue.hpp"
#include "Types/Pipelines.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	class Device
	{
	private:
		Allocator allocator;
		DescriptorManager descriptorManager;
		Queues queues;
		VkDevice device;

		// Universally shared resources
		VkDescriptorSetLayout fragmentSamplerSetLayout;
		VkDescriptorSetLayout ssboSetLayout;
		VkSampler directionalShadowMapSampler;
		VkSampler postProcessingSampler;

		// Device properties
		uint32_t minUniformBufferOffsetAlignment;
	public:
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
	public:
		// Constructors
		Device() = default;
		Device(const vkb::Device& device, VkInstance instance, uint32_t descriptorSetsPerPool, uint32_t minUniformBufferOffsetAlignment);
		// Destructors
		~Device();
		// No copy
		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;
		// Move
		Device(Device&& other) noexcept;
		Device& operator=(Device&& other) noexcept;
	public:
		// Specialized functions
		Buffer CreateBuffer(size_t allocationSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
		Image CreateImage(const VkImageCreateInfo& imageInfo, VmaMemoryUsage memoryUsage);
		GraphicsCommandQueue CreateGraphicsCommandQueue();
		TransferCommandQueue CreateTransferCommandQueue();
		RenderPass CreateRenderPass(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& subpassDependencies);
		RenderPass CreateDefaultRenderPass(VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
		RenderPass CreateDefaultShadowRenderPass(VkFormat depthFormat, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
		RenderPass CreateDefaultDepthPrePassRenderPass(VkFormat depthFormat, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
		RenderPass CreateDefaultPostProcessingRenderPass(VkFormat colorFormat, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
		Framebuffer CreateFramebuffer(VkRenderPass renderPass, const std::vector<VkImageView>& attachments, VkExtent2D extent, bool hasColorAttachment, bool hasDepthAttachment);
		ShaderModule CreateShaderModule(const std::vector<uint8_t>& code);
		ShaderReflection CreateShaderReflection(const std::vector<uint8_t>& code);
		Pipelines CreatePipelines(const std::vector<uint8_t>& vertexCode, const std::vector<uint8_t>& fragmentCode, const PipelineVariants& variant);
		SSBO CreateSSBO(size_t allocationSize, VmaMemoryUsage memoryUsage);

		// Creates a set writes the texture
		VkDescriptorSet CreateTextureDescriptorSet(VkSampler sampler, const Vulkan::Image& image, VkImageLayout layout);
		
		// Low-level vulkan primitives
		VkCommandPool CreateCommandPool(uint32_t queueFamilyIndex);
		VkCommandBuffer AllocateCommandBuffer(VkCommandPool commandPool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		VkFence CreateFence(bool signaled = false);
		VkSemaphore CreateSemaphore();
		VkPipeline CreatePipeline(const PipelineBuilderInfo& info);
		VkPipelineLayout CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, const std::vector<VkPushConstantRange>& pushConstantRanges);
		VkDescriptorSetLayout CreateDescriptorSetLayout(const VkDescriptorSetLayoutBinding& binding);
		VkDescriptorSetLayout CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
		VkSampler CreateSampler(const VkSamplerCreateInfo& info);
		VkDescriptorSet AllocateDescriptorSet(VkDescriptorType type, VkDescriptorSetLayout layout);

		void WriteDescriptorSet(const VkWriteDescriptorSet& descriptorWrite);
		void WriteDescriptorSets(const std::vector<VkWriteDescriptorSet>& descriptorWrites);

		void WaitIdle() const;

		VkDescriptorSetLayout GetFragmentSamplerLayout() const { return fragmentSamplerSetLayout; }
		VkDescriptorSetLayout GetSSBOLayout() const { return ssboSetLayout; }
		VkSampler GetDirectionalShadowMapSampler() const { return directionalShadowMapSampler; }
		VkSampler GetPostProcessingSampler() const { return postProcessingSampler; }
	public:
		operator VkDevice() const { return device; }
	};
}
