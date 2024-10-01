#pragma once
#include "common.hpp"
#include "vulkan/vulkan.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class RenderPass
	{
	private:
		VkDevice device;
		VkRenderPass renderPass;
	public:
		RenderPass();
		// Creates a renderpass from createInfo
		RenderPass(VkDevice device, const VkRenderPassCreateInfo& createInfo);
		// Takes ownership of renderPass
		explicit RenderPass(VkDevice device, VkRenderPass renderPass);
		~RenderPass();
		RenderPass(const RenderPass&) = delete;
		RenderPass& operator=(const RenderPass&) = delete;
		RenderPass(RenderPass&& other) noexcept;
		RenderPass& operator=(RenderPass&& other) noexcept;
		operator VkRenderPass() const;
		VkRenderPass GetRenderPass() const;
	};
}