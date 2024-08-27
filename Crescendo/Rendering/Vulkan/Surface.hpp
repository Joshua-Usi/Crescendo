#pragma once

#include "common.hpp"
#include "RAII.hpp"
#include <functional>

CS_NAMESPACE_BEGIN::Vulkan
{
	class Surface
	{
	public:
		struct SurfaceSpecification
		{
			std::function<void(Vulkan::Surface& surface, uint32_t width, uint32_t height, VkPresentModeKHR presentMode)> swapchainRecreationCallback = nullptr;
			VkPresentModeKHR presentMode;
		};
	private:
		VkInstance instance;
		Vk::Surface surface;
		Vk::PhysicalDevice physicalDevice;
		Vk::Device device;
		Vk::Swapchain swapchain;
		void* window;
		std::vector<Vk::Framebuffer> framebuffers;
		std::function<void(Vulkan::Surface& surface, uint32_t width, uint32_t height, VkPresentModeKHR presentMode)> swapchainRecreationCallback;
		VkPresentModeKHR presentMode;
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
		Vk::Device& GetDevice();
		Vk::Swapchain& GetSwapchain();
		Vk::Swapchain::Image& GetImage(size_t index);
	public:
		void RecreateSwapchain(VkPresentModeKHR presentMode);
		void CallRecreationCallback();
		void AcquireNextImage(VkSemaphore imageAvailableSemaphore, uint64_t timeout = std::numeric_limits<uint64_t>::max());
		void Present(VkQueue queue, VkSemaphore renderFinishSemaphore);
	};
}