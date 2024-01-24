#pragma once

#include "common.hpp"
#include "Volk/volk.h"

CS_NAMESPACE_BEGIN::Vulkan
{
	class Swapchain
	{
	private:
		struct Framebuffer { VkFramebuffer framebuffer; VkImage image; VkImageView imageView; };
		VkSwapchainKHR swapchain;
		VkFormat imageFormat;
		VkRenderPass renderPass;
		std::vector<Framebuffer> framebuffers;
		VkExtent2D extent;
		bool needsRecreation;
	public:
	};

	class Instance
	{
	private:
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkPhysicalDevice physicalDevice;
		Surface surface;
	public:
	};
}