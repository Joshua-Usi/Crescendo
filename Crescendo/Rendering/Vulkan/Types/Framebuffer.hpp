#pragma once

#include "Volk/volk.h"
#include "Create.hpp"

namespace Crescendo::Vulkan
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
		inline Framebuffer(VkDevice device, VkFramebuffer framebuffer, VkRenderPass renderPass, VkExtent2D extent, bool hasColorAttachment, bool hasDepthAttachment) : device(device), framebuffer(framebuffer), renderPass(renderPass), extent(extent), hasColorAttachment(hasColorAttachment), hasDepthAttachment(hasDepthAttachment) {}
		// Destructors
		inline ~Framebuffer() { vkDestroyFramebuffer(this->device, this->framebuffer, nullptr); }
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
		inline VkViewport GetViewport(bool flip = false) const { return Create::Viewport(0.0f, (flip) ? static_cast<float>(this->extent.height) : 0, static_cast<float>(this->extent.width), ((flip) ? -1.0f : 1.0f) * static_cast<float>(this->extent.height), 0.0f, 1.0f); }
		inline VkRect2D GetScissor() const { return Create::Rect2D({ 0, 0 }, this->extent); }
	public:
		inline operator VkFramebuffer() const { return framebuffer; }
	};
}