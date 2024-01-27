#pragma once

#include "common.hpp"
#include "RAII.hpp"
#include "Device.hpp"
#include <functional>

CS_NAMESPACE_BEGIN::Vulkan
{
	class Surface
	{
	private:
		VkInstance instance;
		Vk::Surface surface;
		Vk::PhysicalDevice physicalDevice;
		Device device;
		Vk::Swapchain swapchain;
		void* window;
		std::vector<Vk::Framebuffer> framebuffers;
		std::function<void()> swapchainRecreationCallback;
		bool needsRecreation;
	public:
		Surface();
		Surface(Vk::Instance& instance, void* window, std::function<void()> swapchainRecreationCallback = nullptr);
		~Surface();
		Surface(const Surface&) = delete;
		Surface& operator=(const Surface&) = delete;
		Surface(Surface&& other) noexcept;
		Surface& operator=(Surface&& other) noexcept;
		void* GetWindow() const;
		Vk::PhysicalDevice& GetPhysicalDevice();
		Vk::Device& GetDevice();
		Vk::Swapchain& GetSwapchain();
		Vk::Swapchain::Image& GetImage(size_t index);
		void RecreateSwapchain(VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAILBOX_KHR);
		uint32_t AcquireNextImage(VkSemaphore signalSemaphore, uint64_t timeout = std::numeric_limits<uint64_t>::max());
	};
}