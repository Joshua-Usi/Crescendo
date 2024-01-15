#pragma once

#include "common.hpp"

#include "volk/volk.h"

#include "Types/Image.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	class Swapchain
	{
	public:
		struct Framebuffer { VkFramebuffer framebuffer; VkImage image; VkImageView imageView; };
	private:
		VkDevice device;
		VkSwapchainKHR swapchain;
		VkFormat imageFormat;
		VkRenderPass renderPass;
		std::vector<Swapchain::Framebuffer> framebuffers;
		VkExtent2D extent;
		bool needsRecreation;
	public:
		Swapchain() : device(nullptr), swapchain(nullptr), imageFormat(VK_FORMAT_UNDEFINED), renderPass(nullptr), framebuffers(0), extent({ 0, 0 }), needsRecreation(false) {}
		Swapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkPresentModeKHR presentMode, VkExtent2D windowExtent);
		~Swapchain();
		// No copy
		Swapchain(const Swapchain&) = delete;
		Swapchain& operator=(const Swapchain&) = delete;
		// Move is okay
		Swapchain(Swapchain&& other) noexcept;
		Swapchain& operator=(Swapchain&& other) noexcept;

		operator VkSwapchainKHR() const { return swapchain; }

		uint32_t AcquireNextImage(VkSemaphore signalSemaphore, uint64_t timeout = std::numeric_limits<uint64_t>::max());
		bool NeedsRecreation() const { return needsRecreation; }
		uint32_t GetImageCount() const { return static_cast<uint32_t>(framebuffers.size()); }
		VkFormat GetImageFormat() const { return imageFormat; }
		VkFramebuffer GetFramebuffer(uint32_t index) const { return framebuffers[index].framebuffer; }
		VkRenderPass GetRenderPass() const { return renderPass; }
		const VkExtent2D& GetExtent() const { return extent; }
		VkExtent3D GetExtent3D() const { return { extent.width, extent.height, 1 }; }
		VkSwapchainKHR GetSwapchain() const { return swapchain; }

		VkRenderPass CreateRenderpass(VkSampleCountFlagBits samples);
	};
}