#pragma once

#include "common.hpp"
#include "RAII.hpp"
#include "Device.hpp"
#include <functional>

CS_NAMESPACE_BEGIN::Vulkan
{
	class Surface
	{
	public:
		struct SurfaceSpecification
		{
			std::function<void()> swapchainRecreationCallback = nullptr;
		};
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
		Surface(Vk::Instance& instance, void* window, const SurfaceSpecification& spec);
		~Surface();
		Surface(const Surface&) = delete;
		Surface& operator=(const Surface&) = delete;
		Surface(Surface&& other) noexcept;
		Surface& operator=(Surface&& other) noexcept;
	public:
		void* GetWindow() const;
		Vk::PhysicalDevice& GetPhysicalDevice();
		Device& GetDevice();
		Vk::Swapchain& GetSwapchain();
		Vk::Swapchain::Image& GetImage(size_t index);
	public:
		void RecreateSwapchain(VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAILBOX_KHR);
		void AcquireNextImage(VkSemaphore imageAvailableSemaphore, uint64_t timeout = std::numeric_limits<uint64_t>::max());
		void Present(VkQueue queue, VkSemaphore renderFinishSemaphore);
	};
}