#pragma once

#include "common.hpp"

#include "Volk/volk.h"
#include "Create.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	struct Framebuffer
	{
	private:
		VkDevice device;
		VkFramebuffer framebuffer;
	public:
		VkRenderPass renderPass;
		VkExtent2D extent;
		bool hasColorAttachment, hasDepthAttachment;
	public:
		// Constructors
		Framebuffer() = default;
		Framebuffer(VkDevice device, VkFramebuffer framebuffer, VkRenderPass renderPass, VkExtent2D extent, bool hasColorAttachment, bool hasDepthAttachment) : device(device), framebuffer(framebuffer), renderPass(renderPass), extent(extent), hasColorAttachment(hasColorAttachment), hasDepthAttachment(hasDepthAttachment) {}
		// Destructors
		~Framebuffer() { vkDestroyFramebuffer(this->device, this->framebuffer, nullptr); }
		// No copy
		Framebuffer(const Framebuffer&) = delete;
		Framebuffer& operator=(const Framebuffer&) = delete;
		// Move
		Framebuffer(Framebuffer&& other) noexcept : extent(other.extent), device(other.device), framebuffer(other.framebuffer), renderPass(other.renderPass), hasColorAttachment(other.hasColorAttachment), hasDepthAttachment(other.hasDepthAttachment)
		{
			other.framebuffer = nullptr;
		}
		Framebuffer& operator=(Framebuffer&& other) noexcept
		{
			if (this != &other)
			{
				extent = other.extent;
				device = other.device;
				framebuffer = other.framebuffer; other.framebuffer = nullptr;
				renderPass = other.renderPass;
				hasColorAttachment = other.hasColorAttachment;
				hasDepthAttachment = other.hasDepthAttachment;
			}
			return *this;
		}
	public:
		VkViewport GetViewport(bool flip = false) const { return Create::Viewport(0.0f, (flip) ? static_cast<float>(this->extent.height) : 0, static_cast<float>(this->extent.width), ((flip) ? -1.0f : 1.0f) * static_cast<float>(this->extent.height), 0.0f, 1.0f); }
		VkRect2D GetScissor(int32_t offsetX = 0, int32_t offsetY = 0, int32_t extentChangeX = 0, int32_t extentChangeY = 0) const { return Create::Rect2D({ offsetX, offsetY }, { this->extent.width + extentChangeX, this->extent.height + extentChangeY }); }
	public:
		operator VkFramebuffer() const { return framebuffer; }
	};
}