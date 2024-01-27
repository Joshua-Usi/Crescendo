#include "RenderPass.hpp"
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
		if (this->device != nullptr) vkDestroyRenderPass(this->device, this->renderPass, nullptr);
	}
	RenderPass::RenderPass(RenderPass&& other) noexcept : device(other.device), renderPass(other.renderPass)
	{
		other.device = nullptr;
		other.renderPass = nullptr;
	}
	RenderPass& RenderPass::operator=(RenderPass&& other) noexcept
	{
		if (this == &other) return *this;
		this->device = other.device; other.device = nullptr;
		this->renderPass = other.renderPass; other.renderPass = nullptr;
		return *this;
	}
	RenderPass::operator VkRenderPass() const { return renderPass; }
	VkRenderPass RenderPass::GetRenderPass() const { return renderPass; }
}