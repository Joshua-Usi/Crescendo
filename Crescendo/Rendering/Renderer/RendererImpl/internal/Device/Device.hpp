#pragma once

#include "vulkan/vulkan.h"

#include <vector>

namespace Crescendo::internal
{
	class Device
	{
	private:
		VkDevice device;
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
		inline Device(VkDevice device = nullptr) : device(device) {}
		~Device() = default;

		inline VkDevice GetDevice() const { return device; }
		inline operator VkDevice() const { return GetDevice(); }

		VkCommandPool CreateCommandPool(uint32_t queueFamilyIndex);
		VkCommandBuffer AllocateCommandBuffer(VkCommandPool commandPool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		VkFence CreateFence(bool signaled = false);
		VkSemaphore CreateSemaphore();

		VkShaderModule CreateShaderModule(const std::vector<uint8_t>& code);

		VkRenderPass CreateRenderPass(
			const std::vector<VkAttachmentDescription>& attachments,
			const std::vector<VkSubpassDescription>& subpasses,
			const std::vector<VkSubpassDependency>& subpassDependencies
		);
		// More specific render pass creation
		// Can creates a default 3d render pass with a color and depth attachment and optional multisampling
		VkRenderPass CreateDefaultRenderPass(VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
		// TODO support multisampling shadow maps
		// Creates a default shadow map render pass with a depth attachment
		VkRenderPass CreateDefaultShadowRenderPass(VkFormat depthFormat, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);


		VkPipeline CreatePipeline(PipelineBuilderInfo& info);
		VkDescriptorSetLayout CreateDescriptorSetLayout(const VkDescriptorSetLayoutBinding& binding);
		VkDescriptorSetLayout CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

		VkFramebuffer CreateFramebuffer(VkRenderPass renderPass, const VkImageView& attachment, uint32_t width, uint32_t height);
		VkFramebuffer CreateFramebuffer(VkRenderPass renderPass, const std::vector<VkImageView>& attachments, uint32_t width, uint32_t height);

		VkSampler CreateSampler(const VkSamplerCreateInfo& info);
	};
}
