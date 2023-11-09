#pragma once

#include "volk/volk.h"

#include "Types/Image.hpp"

#include <vector>

namespace Crescendo::Vulkan
{
	class Swapchain
	{
	public:
		struct Image { VkImage image; VkImageView imageView; };
	private:
		VkDevice device;
		VkSwapchainKHR swapchain;
		VkFormat imageFormat;
		VkExtent2D extent;
		std::vector<Image> images;
		bool needsRecreation;
	public:
		Swapchain() : device(nullptr), swapchain(nullptr), imageFormat(VK_FORMAT_UNDEFINED), extent({ 0, 0 }), images(), needsRecreation(false) {}
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
		uint32_t GetImageCount() const { return static_cast<uint32_t>(images.size()); }
		VkFormat GetImageFormat() const { return imageFormat; }
			const VkExtent2D& GetExtent() const { return extent; }
		VkExtent3D GetExtent3D() const { return { extent.width, extent.height, 1 }; }
		VkSwapchainKHR GetSwapchain() const { return swapchain; }

		// Iterator over images
		std::vector<Image>::iterator begin() { return images.begin(); }
		std::vector<Image>::iterator end() { return images.end(); }
		std::vector<Image>::const_iterator begin() const { return images.begin(); }
		std::vector<Image>::const_iterator end() const { return images.end(); }
	};
}