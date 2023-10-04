#pragma once

#include "volk/volk.h"

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

		inline void Destroy() { vkDestroyDevice(this->device, nullptr); }

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
		VkPipelineLayout CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, const std::vector<VkPushConstantRange>& pushConstantRanges);
		VkDescriptorSetLayout CreateDescriptorSetLayout(const VkDescriptorSetLayoutBinding& binding);
		VkDescriptorSetLayout CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

		VkFramebuffer CreateFramebuffer(VkRenderPass renderPass, const VkImageView& attachment, uint32_t width, uint32_t height);
		VkFramebuffer CreateFramebuffer(VkRenderPass renderPass, const std::vector<VkImageView>& attachments, uint32_t width, uint32_t height);

		VkSampler CreateSampler(const VkSamplerCreateInfo& info);

		void WriteDescriptorSet(const VkWriteDescriptorSet& descriptorWrite);
		void WriteDescriptorSets(const std::vector<VkWriteDescriptorSet>& descriptorWrites);

		inline void DestroyCommandPool(VkCommandPool commandPool) { vkDestroyCommandPool(this->device, commandPool, nullptr); }
		inline void DestroyFence(VkFence fence) { vkDestroyFence(this->device, fence, nullptr); }
		inline void DestroySemaphore(VkSemaphore semaphore) { vkDestroySemaphore(this->device, semaphore, nullptr); }
		inline void DestroyShaderModule(VkShaderModule shaderModule) { vkDestroyShaderModule(this->device, shaderModule, nullptr); }
		inline void DestroyRenderPass(VkRenderPass renderPass) { vkDestroyRenderPass(this->device, renderPass, nullptr); }
		inline void DestroyPipelineLayout(VkPipelineLayout pipelineLayout) { vkDestroyPipelineLayout(this->device, pipelineLayout, nullptr); }
		inline void DestroyPipeline(VkPipeline pipeline) { vkDestroyPipeline(this->device, pipeline, nullptr); }
		inline void DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) { vkDestroyDescriptorSetLayout(this->device, descriptorSetLayout, nullptr); }
		inline void DestroyFramebuffer(VkFramebuffer framebuffer) { vkDestroyFramebuffer(this->device, framebuffer, nullptr); }
		inline void DestroySampler(VkSampler sampler) { vkDestroySampler(this->device, sampler, nullptr); }

		inline void DestroySwapchain(VkSwapchainKHR swapchain) { vkDestroySwapchainKHR(this->device, swapchain, nullptr); }

		inline void DestroyCommandPools(const std::vector<VkCommandPool>& commandPools) { for (const VkCommandPool& commandPool : commandPools) this->DestroyCommandPool(commandPool); }
		inline void DestroyFences(const std::vector<VkFence>& fences) { for (const VkFence& fence : fences) this->DestroyFence(fence); }
		inline void DestroySemaphores(const std::vector<VkSemaphore>& semaphores) { for (const VkSemaphore& semaphore : semaphores) this->DestroySemaphore(semaphore); }
		inline void DestroyShaderModules(const std::vector<VkShaderModule>& shaderModules) { for (const VkShaderModule& shaderModule : shaderModules) this->DestroyShaderModule(shaderModule); }
		inline void DestroyRenderPasses(const std::vector<VkRenderPass>& renderPasses) { for (const VkRenderPass& renderPass : renderPasses) this->DestroyRenderPass(renderPass); }
		inline void DestroyPipelineLayouts(const std::vector<VkPipelineLayout>& pipelineLayouts) { for (const VkPipelineLayout& pipelineLayout : pipelineLayouts) this->DestroyPipelineLayout(pipelineLayout); }
		inline void DestroyPipelines(const std::vector<VkPipeline>& pipelines) { for (const VkPipeline& pipeline : pipelines) this->DestroyPipeline(pipeline); }
		inline void DestroyDescriptorSetLayouts(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts) { for (const VkDescriptorSetLayout& descriptorSetLayout : descriptorSetLayouts) this->DestroyDescriptorSetLayout(descriptorSetLayout); }
		inline void DestroyFramebuffers(const std::vector<VkFramebuffer>& framebuffers) { for (const VkFramebuffer& framebuffer : framebuffers) this->DestroyFramebuffer(framebuffer); }
		inline void DestroySamplers(const std::vector<VkSampler>& samplers) { for (const VkSampler& sampler : samplers) this->DestroySampler(sampler); }
	};
}
