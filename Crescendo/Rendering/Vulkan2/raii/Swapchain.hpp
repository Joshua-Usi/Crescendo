#pragma once

#include "common.hpp"
#include "vulkan/vulkan.h"
#include "RenderPass.hpp"
#include "PhysicalDevice.hpp"
#include "Device.hpp"
#include "Surface.hpp"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class Swapchain
	{
	public:
		struct Image { VkFramebuffer framebuffer; VkImage image; VkImageView imageView; };
		struct SwapchainSpecification
		{
			VkPresentModeKHR presentMode;
			VkExtent2D windowExtent;
			VkSampleCountFlagBits samples;
		};
	private:
		VkDevice device;
		VkSwapchainKHR swapchain;
		VkFormat imageFormat;
		VkExtent2D extent;
		RenderPass renderPass;
		std::vector<Image> images;
		uint32_t acquiredImageIndex;
	public:
		Swapchain();
		// Creates a swapchain using vkb
		Swapchain(const PhysicalDevice& physicalDevice, const Device& device, const Surface& surface, const SwapchainSpecification& spec);
		// Takes ownership of swapchain
		explicit Swapchain(const Device& device, VkSwapchainKHR swapchain);
		~Swapchain();
		Swapchain(const Swapchain&) = delete;
		Swapchain& operator=(const Swapchain&) = delete;
		Swapchain(Swapchain&& other) noexcept;
		Swapchain& operator=(Swapchain&& other) noexcept;
	public:
		RenderPass CreateRenderpass(VkSampleCountFlagBits samples);
		VkResult AcquireNextImage(VkSemaphore imageAvailableSemaphore, uint64_t timeout);
		VkResult Present(VkQueue queue, VkSemaphore renderFinishSemaphore);
	public:
		operator VkSwapchainKHR() const;
		VkSwapchainKHR GetSwapchain() const;
		size_t ImageCount() const;
		VkImage GetImage(uint32_t index) const;
		VkImageView GetImageView(uint32_t index) const;
		Image& GetFramebuffer(uint32_t index);
		VkFormat GetImageFormat() const;
		const VkExtent2D& GetExtent() const;
		VkExtent3D GetExtent3D() const;
		VkViewport GetViewport(bool flipY = false) const;
		VkRect2D GetScissor() const;
		uint32_t GetAcquiredImageIndex() const;
		VkRenderPass GetRenderPass() const;
	};
}