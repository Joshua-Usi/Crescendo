#pragma once

#include "common.hpp"
#include "Volk/volk.h"

#include "Swapchain.hpp"
#include "PhysicalDevice.hpp"
#include "Device.hpp"

#include <functional>

CS_NAMESPACE_BEGIN::Vulkan
{
	// Forward declarations
	class Instance;

	class Surface
	{
	private:
		VkInstance instance;
		VkSurfaceKHR surface;
		void* window;
		PhysicalDevice physicalDevice;
		Device device;
		Swapchain swapchain;
		std::function<void()> swapchainRecreationCallback;
	public:
		Surface();
		Surface(Instance& instance, void* window, std::function<void()> swapchainRecreationCallback = nullptr);
		~Surface();
		// No copy
		Surface(const Surface&) = delete;
		Surface& operator=(const Surface&) = delete;
		// Move
		Surface(Surface&& other) noexcept;
		Surface& operator=(Surface&& other) noexcept;

		operator VkSurfaceKHR() const { return surface; }

		void* GetWindow() const { return window; }
		VkPhysicalDevice GetVkPhysicalDevice() const { return physicalDevice; }
		PhysicalDevice& GetPhysicalDevice() { return physicalDevice; }

		VkDevice GetVkDevice() const { return device; }
		Device& GetDevice() { return device; }

		VkSwapchainKHR GetVkSwapchain() const { return swapchain; }
		Swapchain& GetSwapchain() { return swapchain; }

		void RecreateSwapchain(VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAILBOX_KHR);
		bool SwapchainNeedsRecreation() const { return swapchain.NeedsRecreation(); }
		void SetSwapchainRecreationCallback(std::function<void()> callback) { swapchainRecreationCallback = callback; }
		void SwapchainRecreationCallback() { if (swapchainRecreationCallback != nullptr) swapchainRecreationCallback(); }
	};
}