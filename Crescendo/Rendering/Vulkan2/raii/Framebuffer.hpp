#pragma once

#include "common.hpp"
#include "vulkan/vulkan.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class Framebuffer
	{
	private:
		VkDevice device;
		VkRenderPass renderPass;
		VkFramebuffer framebuffer;
		VkExtent2D extent;
	public:
		Framebuffer();
		Framebuffer(VkDevice device, const VkFramebufferCreateInfo& createInfo);
		~Framebuffer();
		Framebuffer(const Framebuffer&) = delete;
		Framebuffer& operator=(const Framebuffer&) = delete;
		Framebuffer(Framebuffer&& other) noexcept;
		Framebuffer& operator=(Framebuffer&& other) noexcept;
		operator VkFramebuffer() const;
		VkFramebuffer GetFramebuffer() const;
		const VkExtent2D& GetExtent() const;
		VkExtent3D GetExtent3D() const;
		VkViewport GetViewport(bool flipY = false) const;
		VkRect2D GetScissor() const;
	};
}