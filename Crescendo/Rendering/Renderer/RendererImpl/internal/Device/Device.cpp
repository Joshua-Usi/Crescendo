#include "Device.hpp"

#include "Core/common.hpp"
#include "../Create.hpp"

#include <iostream>

namespace Crescendo::internal
{
	VkCommandPool Device::CreateCommandPool(uint32_t queueFamilyIndex)
	{
		VkCommandPool pool = nullptr;
		const VkCommandPoolCreateInfo poolInfo = Create::CommandPoolCreateInfo(queueFamilyIndex);
		CS_ASSERT(vkCreateCommandPool(this->device, &poolInfo, nullptr, &pool) == VK_SUCCESS, "Failed to create command pool!");
		return pool;
	}
	VkCommandBuffer Device::AllocateCommandBuffer(VkCommandPool commandPool, VkCommandBufferLevel level)
	{
		VkCommandBuffer cmd = nullptr;
		const VkCommandBufferAllocateInfo cmdBufferInfo = Create::CommandBufferAllocateInfo(commandPool, level, 1);
		CS_ASSERT(vkAllocateCommandBuffers(this->device, &cmdBufferInfo, &cmd) == VK_SUCCESS, "Failed to allocate command buffers!");
		return cmd;
	}
	VkFence Device::CreateFence(bool signaled)
	{
		VkFence fence = nullptr;
		const VkFenceCreateInfo fenceInfo = Create::FenceCreateInfo(signaled);
		CS_ASSERT(vkCreateFence(this->device, &fenceInfo, nullptr, &fence) == VK_SUCCESS, "Failed to create fence");
		return fence;
	}
	VkSemaphore Device::CreateSemaphore()
	{
		VkSemaphore semaphore = nullptr;
		const VkSemaphoreCreateInfo semaphoreInfo = Create::SemaphoreCreateInfo();
		CS_ASSERT(vkCreateSemaphore(this->device, &semaphoreInfo, nullptr, &semaphore) == VK_SUCCESS, "Failed to create semaphore");
		return semaphore;
	}
	VkShaderModule Device::CreateShaderModule(const std::vector<uint8_t>& code)
	{
		VkShaderModule shaderModule = nullptr;
		const VkShaderModuleCreateInfo createInfo = Create::ShaderModuleCreateInfo(code);
		CS_ASSERT(vkCreateShaderModule(this->device, &createInfo, nullptr, &shaderModule) == VK_SUCCESS, "Failed to create shader module!");
		return shaderModule;
	}
	VkRenderPass Device::CreateRenderPass(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& subpassDependencies)
	{
		VkRenderPass renderPass = nullptr;
		const VkRenderPassCreateInfo renderPassInfo = Create::RenderPassCreateInfo(attachments, subpasses, subpassDependencies);
		CS_ASSERT(vkCreateRenderPass(this->device, &renderPassInfo, nullptr, &renderPass) == VK_SUCCESS, "Failed to create render pass");
		return renderPass;
	}
	VkRenderPass Device::CreateDefaultRenderPass(VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits samples)
	{
		bool isMultiSampling = samples != VK_SAMPLE_COUNT_1_BIT;

		VkAttachmentDescription colorAttachment = Create::AttachmentDescription(
			colorFormat, samples,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, (!isMultiSampling) ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);
		VkAttachmentReference colorAttachmentRef = Create::AttachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		VkAttachmentDescription depthAttachment = Create::AttachmentDescription(
			depthFormat, samples,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);
		VkAttachmentReference depthAttachmentRef = Create::AttachmentReference(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		VkAttachmentDescription colorAttachmentResolve = Create::AttachmentDescription(
			colorFormat, VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		);
		VkAttachmentReference colorAttachmentResolveRef = Create::AttachmentReference((isMultiSampling) ? 2 : VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		const VkSubpassDescription subpass = Create::SubpassDescription(
			VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1,
			&colorAttachmentRef,
			&colorAttachmentResolveRef,
			&depthAttachmentRef,
			0, nullptr
		);

		VkSubpassDependency colorDependency = Create::SubpassDependency(
			VK_SUBPASS_EXTERNAL, 0,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0
		);
		VkSubpassDependency depthDependency = Create::SubpassDependency(
			VK_SUBPASS_EXTERNAL, 0,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, 0
		);

		std::vector<VkAttachmentDescription> attachments{ colorAttachment, depthAttachment };
		if (isMultiSampling) attachments.push_back(colorAttachmentResolve);
		std::vector<VkSubpassDependency> dependencies{ colorDependency, depthDependency };

		return this->CreateRenderPass(attachments, { subpass }, dependencies);
	}
	VkRenderPass Device::CreateDefaultShadowRenderPass(VkFormat depthFormat, VkSampleCountFlagBits samples)
	{
		VkAttachmentDescription shadowMapAttachment = Create::AttachmentDescription(
			depthFormat, samples,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
		);
		VkAttachmentReference shadowMapAttachmentRef = Create::AttachmentReference(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		VkSubpassDescription shadowMapSubpass = Create::SubpassDescription(
			VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, &shadowMapAttachmentRef, 0, nullptr
		);

		VkSubpassDependency shadowMapDependency = Create::SubpassDependency(
			VK_SUBPASS_EXTERNAL, 0,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		);
		VkSubpassDependency shadowMapDependency2 = Create::SubpassDependency(
			0, VK_SUBPASS_EXTERNAL,
			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		);

		return this->CreateRenderPass({ shadowMapAttachment }, { shadowMapSubpass }, { shadowMapDependency, shadowMapDependency2 });
	}
	VkPipeline Device::CreatePipeline(PipelineBuilderInfo& info)
	{
		VkPipeline pipeline = nullptr;
		// We use always use dynamic states for viewports and scissors
		const VkPipelineViewportStateCreateInfo viewportState = Create::PipelineViewportStateCreateInfo(nullptr, 1, nullptr, 1, nullptr);
		const VkPipelineColorBlendStateCreateInfo colorBlending = Create::PipelineColorBlendStateCreateInfo(
			VK_FALSE, VK_LOGIC_OP_COPY, 1, &info.colorBlendAttachment, { 0.0f, 0.0f, 0.0f, 0.0f }
		);
		// Fill pipeline info
		const VkGraphicsPipelineCreateInfo pipelineInfo = Create::GraphicsPipelineCreateInfo(
			info.shaderStagesInfo,
			&info.vertexInputInfo, &info.inputAssemblyInfo, &info.tessellationInfo,
			&viewportState, &info.rasterizerInfo, &info.multisamplingInfo,
			&info.depthStencilInfo, &colorBlending, &info.dynamicState,
			info.pipelineLayout, info.renderPass, 0, VK_NULL_HANDLE, 0
		);
		// Sometimes pipelines can fail to generate, for now we'll treat it as a critical error
		CS_ASSERT(vkCreateGraphicsPipelines(this->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) == VK_SUCCESS, "Failed to create pipeline!");
		return pipeline;
	}
	VkPipelineLayout Device::CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, const std::vector<VkPushConstantRange>& pushConstantRanges)
	{
		VkPipelineLayout pipelineLayout = nullptr;
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = Create::PipelineLayoutCreateInfo(
			descriptorSetLayouts, pushConstantRanges
		);
		CS_ASSERT(vkCreatePipelineLayout(this->device, &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS, "Failed to create pipeline layout!");
		return pipelineLayout;
	}
	VkDescriptorSetLayout Device::CreateDescriptorSetLayout(const VkDescriptorSetLayoutBinding& binding)
	{
		VkDescriptorSetLayout layout = nullptr;
		const VkDescriptorSetLayoutCreateInfo setInfo = Create::DescriptorSetLayoutCreateInfo(binding);
		vkCreateDescriptorSetLayout(this->device, &setInfo, nullptr, &layout);
		return layout;
	}
	VkDescriptorSetLayout Device::CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
	{
		VkDescriptorSetLayout layout = nullptr;
		const VkDescriptorSetLayoutCreateInfo setInfo = Create::DescriptorSetLayoutCreateInfo(bindings);
		vkCreateDescriptorSetLayout(this->device, &setInfo, nullptr, &layout);
		return layout;
	}
	VkFramebuffer Device::CreateFramebuffer(VkRenderPass renderPass, const VkImageView& attachment, uint32_t width, uint32_t height)
	{
		VkFramebuffer fb = nullptr;
		VkFramebufferCreateInfo framebufferInfo = Create::FramebufferCreateInfo(renderPass, attachment, { width, height }, 1);
		CS_ASSERT(vkCreateFramebuffer(this->device, &framebufferInfo, nullptr, &fb) == VK_SUCCESS, "Failed to create framebuffer!");
		return fb;
	}
	VkFramebuffer Device::CreateFramebuffer(VkRenderPass renderPass, const std::vector<VkImageView>& attachments, uint32_t width, uint32_t height)
	{
		VkFramebuffer fb = nullptr;
		VkFramebufferCreateInfo framebufferInfo = Create::FramebufferCreateInfo(renderPass, attachments, { width, height }, 1);
		CS_ASSERT(vkCreateFramebuffer(this->device, &framebufferInfo, nullptr, &fb) == VK_SUCCESS, "Failed to create framebuffer!");
		return fb;
	}
	VkSampler Device::CreateSampler(const VkSamplerCreateInfo& info)
	{
		VkSampler sampler = nullptr;
		vkCreateSampler(this->device, &info, nullptr, &sampler);
		return sampler;
	}
	void Device::WriteDescriptorSet(const VkWriteDescriptorSet& descriptorWrite)
	{
		vkUpdateDescriptorSets(this->device, 1, &descriptorWrite, 0, nullptr);
	}
	void Device::WriteDescriptorSets(const std::vector<VkWriteDescriptorSet>& descriptorWrites)
	{
		vkUpdateDescriptorSets(this->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}