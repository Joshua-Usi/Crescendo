#include "Swapchain.hpp"

#include "VkBootstrap/VkBootstrap.h"

CS_NAMESPACE_BEGIN::Vulkan
{
	Swapchain::Swapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkPresentModeKHR presentMode, VkExtent2D windowExtent) : device(device), needsRecreation(false)
	{
		vkb::Swapchain vkbSwapchain = vkb::SwapchainBuilder(physicalDevice, device, surface)
			.use_default_format_selection()
			.set_desired_present_mode(presentMode)
			.set_desired_extent(windowExtent.width, windowExtent.height)
			.build().value();

		// Merge into images
		std::vector<VkImage> SCimages = vkbSwapchain.get_images().value();
		std::vector<VkImageView> SCimageViews = vkbSwapchain.get_image_views().value();
		this->images.resize(SCimages.size());
		for (size_t i = 0; i < images.size(); i++) this->images[i] = Image(SCimages[i], SCimageViews[i]);

		this->swapchain = vkbSwapchain.swapchain;
		this->imageFormat = vkbSwapchain.image_format;
		this->extent = vkbSwapchain.extent;
	}
	Swapchain::~Swapchain()
	{
		if (this->device == nullptr) return;
		for (auto& image : this->images) vkDestroyImageView(this->device, image.imageView, nullptr);
		vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);
	}
	Swapchain::Swapchain(Swapchain&& other) noexcept : device(other.device), swapchain(other.swapchain), imageFormat(other.imageFormat), extent(other.extent), images(std::move(other.images)), needsRecreation(other.needsRecreation)
	{
		other.swapchain = nullptr;
		other.images.clear();
	}
	Swapchain& Swapchain::operator=(Swapchain&& other) noexcept
	{
		if (this != &other)
		{
			this->device = other.device;
			this->swapchain = other.swapchain;
			this->imageFormat = other.imageFormat;
			this->extent = other.extent;
			this->images = std::move(other.images);
			this->needsRecreation = other.needsRecreation;

			other.swapchain = VK_NULL_HANDLE;
			other.imageFormat = VK_FORMAT_UNDEFINED;
			other.extent = { 0, 0 };
		}
		return *this;
	}
	uint32_t Swapchain::AcquireNextImage(VkSemaphore signalSemaphore, uint64_t timeout)
	{
		uint32_t index;
		VkResult imageAcquireResult = vkAcquireNextImageKHR(this->device, this->swapchain, timeout, signalSemaphore, VK_NULL_HANDLE, &index);
		if (imageAcquireResult == VK_ERROR_OUT_OF_DATE_KHR || imageAcquireResult == VK_SUBOPTIMAL_KHR)
		{
			this->needsRecreation = true;
		}
		else CS_ASSERT(imageAcquireResult == VK_SUCCESS, "Failed to acquire swap chain image!");
		return index;
	}

}