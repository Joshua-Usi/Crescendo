#include "Surface.hpp"

#include "Instance.hpp"

#include "glfw/glfw3.h"

CS_NAMESPACE_BEGIN::Vulkan
{
	Surface::Surface() : instance(nullptr), surface(nullptr), swapchain(), window(nullptr) {}
	Surface::Surface(Instance& instance, void* window, std::function<void()> swapchainRecreationCallback) : instance(instance), window(window), swapchain(), swapchainRecreationCallback(swapchainRecreationCallback)
	{
		constexpr uint32_t DEFAULT_SETS_PER_POOL = 32;
		constexpr VkPresentModeKHR DEFAULT_PRESENT_MODE = VK_PRESENT_MODE_MAILBOX_KHR;

		if (glfwCreateWindowSurface(instance, static_cast<GLFWwindow*>(window), nullptr, &this->surface) != VK_SUCCESS) cs_std::console::fatal("Failed to create window surface!");

		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.fillModeNonSolid = VK_TRUE;
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.sampleRateShading = VK_TRUE;

		this->physicalDevice = PhysicalDevice(1, 3, instance, *this, deviceFeatures);
		this->device = Device(instance, *this, DEFAULT_SETS_PER_POOL);

		this->RecreateSwapchain(DEFAULT_PRESENT_MODE);
	}
	Surface::Surface(Surface&& other) noexcept : instance(other.instance), surface(other.surface), window(other.window), swapchain(std::move(other.swapchain)), physicalDevice(other.physicalDevice)
	{
		other.instance = nullptr;
		other.surface = nullptr;
		other.window = nullptr;
	}
	Surface& Surface::operator=(Surface&& other) noexcept
	{
		if (this == &other) return *this;
		if (surface != nullptr) this->~Surface();

		instance = other.instance; other.instance = nullptr;
		surface = other.surface; other.surface = nullptr;
		window = other.window; other.window = nullptr;
		swapchain = std::move(other.swapchain);
		physicalDevice = other.physicalDevice;

		return *this;	
	}
	void Surface::RecreateSwapchain(VkPresentModeKHR presentMode)
	{
		this->device.WaitIdle();
		
		int width = 0, height = 0;
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(static_cast<GLFWwindow*>(window), &width, &height);
			glfwWaitEvents();
		}

		this->swapchain = Swapchain(*this, this->device, presentMode, { static_cast<uint32_t>(width), static_cast<uint32_t>(height) });
		if (this->swapchainRecreationCallback != nullptr) this->swapchainRecreationCallback();
	}
	Surface::~Surface()
	{
		if (surface == nullptr) return;

		vkDestroySurfaceKHR(instance, surface, nullptr);
		surface = nullptr;
	}
}