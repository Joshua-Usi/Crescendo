#include "Framebuffer.hpp"
#include "Volk/volk.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	Framebuffer::Framebuffer() : device(nullptr), extent({ 0, 0 }) {}
	Framebuffer::Framebuffer(VkDevice device, const VkFramebufferCreateInfo& createInfo) : device(device), extent({ createInfo.width, createInfo.height })
	{
		CS_ASSERT(vkCreateFramebuffer(device, &createInfo, nullptr, &framebuffer) == VK_SUCCESS, "Failed to create framebuffer");
	}
	Framebuffer::~Framebuffer()
	{
		if (this->device != nullptr)
			vkDestroyFramebuffer(this->device, this->framebuffer, nullptr);
	}
	Framebuffer::Framebuffer(Framebuffer&& other) noexcept : device(other.device), extent(other.extent), framebuffer(other.framebuffer)
	{
		other.device = nullptr;
		other.framebuffer = nullptr;
	}
	Framebuffer& Framebuffer::Framebuffer::operator=(Framebuffer&& other) noexcept
	{
		if (this->device != nullptr)
			vkDestroyFramebuffer(this->device, this->framebuffer, nullptr);
		this->device = other.device;
		this->extent = other.extent;
		this->framebuffer = other.framebuffer;
		other.device = nullptr;
		other.framebuffer = nullptr;
		return *this;	
	}
	Framebuffer::operator VkFramebuffer() const { return framebuffer; }
	VkFramebuffer Framebuffer::GetFramebuffer() const { return framebuffer; }
	const VkExtent2D& Framebuffer::GetExtent() const { return extent; }
	VkExtent3D Framebuffer::GetExtent3D() const { return { extent.width, extent.height, 1 }; }
	VkViewport Framebuffer::GetViewport(bool flipY) const { return { 0.0f, flipY ? static_cast<float>(extent.height) : 0.0f, static_cast<float>(extent.width), flipY ? -static_cast<float>(extent.height) : static_cast<float>(extent.height), 0.0f, 1.0f }; }
	VkRect2D Framebuffer::GetScissor() const { return { { 0, 0 }, extent }; }
}