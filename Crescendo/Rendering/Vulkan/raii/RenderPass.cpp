#include "RenderPass.hpp"
#include "../Create.hpp"
#include "Volk/volk.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	RenderPass::RenderPass() : device(nullptr), renderPass(nullptr) {}
	RenderPass::RenderPass(VkDevice device, const VkRenderPassCreateInfo& createInfo) : device(device)
	{
		CS_ASSERT(vkCreateRenderPass(device, &createInfo, nullptr, &this->renderPass) == VK_SUCCESS, "Failed to create renderpass!");
	}
	RenderPass::RenderPass(VkDevice device, VkRenderPass renderPass) : device(device), renderPass(renderPass) {}
	RenderPass::~RenderPass()
	{
		if (this->device != nullptr)
			vkDestroyRenderPass(this->device, this->renderPass, nullptr);
	}
	RenderPass::RenderPass(RenderPass&& other) noexcept : device(other.device), renderPass(other.renderPass)
	{
		other.device = nullptr;
		other.renderPass = nullptr;
	}
	RenderPass& RenderPass::operator=(RenderPass&& other) noexcept
	{
		if (this == &other)
			return *this;
		this->device = other.device; other.device = nullptr;
		this->renderPass = other.renderPass; other.renderPass = nullptr;
		return *this;
	}
	RenderPass::operator VkRenderPass() const { return renderPass; }
	VkRenderPass RenderPass::GetRenderPass() const { return renderPass; }
	RenderPass RenderPass::CreateReversedZDepthRenderPass(VkDevice device, VkFormat depthFormat, VkSampleCountFlagBits samples)
	{
		VkAttachmentDescription depthAttachment = Vulkan::Create::AttachmentDescription(
			depthFormat, samples, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);
		VkAttachmentReference depthAttachmentRef = Vulkan::Create::AttachmentReference(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		VkSubpassDescription depthSubpass = Vulkan::Create::SubpassDescription(VK_PIPELINE_BIND_POINT_GRAPHICS, nullptr, nullptr, nullptr, &depthAttachmentRef, nullptr);
		const std::array<VkSubpassDependency, 2> depthDependencies = {
			Vulkan::Create::SubpassDependency(
				VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_SHADER_READ_BIT,
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT
			),
			Vulkan::Create::SubpassDependency(
				0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT
			)
		};
		return Vulkan::Vk::RenderPass(device, Vulkan::Create::RenderPassCreateInfo(&depthAttachment, &depthSubpass, depthDependencies));
	}
	RenderPass RenderPass::CreateMainRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits samples)
	{
		VkAttachmentDescription colorAttachment = Vulkan::Create::AttachmentDescription(
			colorFormat, samples,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
		VkAttachmentReference colorAttachmentRef = Vulkan::Create::AttachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkAttachmentDescription depthAttachment = Vulkan::Create::AttachmentDescription(
			depthFormat, samples,
			VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);
		VkAttachmentReference depthAttachmentRef = Vulkan::Create::AttachmentReference(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		const VkSubpassDescription subpass = Vulkan::Create::SubpassDescription(
			VK_PIPELINE_BIND_POINT_GRAPHICS, nullptr, &colorAttachmentRef, nullptr, &depthAttachmentRef, nullptr
		);
		VkSubpassDependency colorDependency = Vulkan::Create::SubpassDependency(
			VK_SUBPASS_EXTERNAL, 0,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT
		);
		VkSubpassDependency colorDependency2 = Vulkan::Create::SubpassDependency(
			0, VK_SUBPASS_EXTERNAL,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT
		);
		VkSubpassDependency depthDependency = Vulkan::Create::SubpassDependency(
			VK_SUBPASS_EXTERNAL, 0,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT
		);
		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		std::array<VkSubpassDependency, 3> dependencies = { colorDependency, colorDependency2, depthDependency };

		return Vulkan::Vk::RenderPass(device, Vulkan::Create::RenderPassCreateInfo(attachments, &subpass, dependencies));
	}
	RenderPass RenderPass::CreateBloomRenderPass(VkDevice device, VkFormat colorFormat)
	{
		VkAttachmentDescription colorAttachment = Vulkan::Create::AttachmentDescription(
			colorFormat, VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
		VkAttachmentReference colorAttachmentRef = Vulkan::Create::AttachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		const VkSubpassDescription subpass = Vulkan::Create::SubpassDescription(
			VK_PIPELINE_BIND_POINT_GRAPHICS, nullptr, &colorAttachmentRef, nullptr, nullptr, nullptr
		);
		VkSubpassDependency dependency = Vulkan::Create::SubpassDependency(
			VK_SUBPASS_EXTERNAL, 0,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT
		);
		return Vulkan::Vk::RenderPass(device, Vulkan::Create::RenderPassCreateInfo(&colorAttachment, &subpass, &dependency));
	}
}