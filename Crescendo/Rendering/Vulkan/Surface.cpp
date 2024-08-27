#include "Surface.hpp"
#include "GLFW/glfw3.h"
#include "Create.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	Surface::Surface() : instance(nullptr), surface(), window(nullptr), physicalDevice(), device(), swapchain(), framebuffers(), swapchainRecreationCallback(nullptr), needsRecreation(false) {}
	Surface::Surface(Vk::Instance& instance, void* window, const SurfaceSpecification& spec) : instance(instance), window(window), swapchainRecreationCallback(spec.swapchainRecreationCallback), presentMode(spec.presentMode), needsRecreation(false)
	{
		this->surface = Vk::Surface(instance, window);

		VkPhysicalDeviceFeatures requiredDeviceFeatures {};
		requiredDeviceFeatures.fillModeNonSolid = true;
		requiredDeviceFeatures.samplerAnisotropy = true;
		requiredDeviceFeatures.sampleRateShading = true;

		this->physicalDevice = Vk::PhysicalDevice(instance, this->surface, {
			.preferredDeviceType = Vk::PhysicalDevice::PhysicalDeviceSelectionCriteria::PreferredDeviceType::Discrete,
			.major = 1, .minor = 3,
			.requiredDeviceFeatures = requiredDeviceFeatures
		});

		VkPhysicalDeviceShaderDrawParametersFeatures drawParametersFeatures = {};
		drawParametersFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
		drawParametersFeatures.shaderDrawParameters = VK_TRUE;

		this->device = std::move(Vk::Device(instance, this->physicalDevice, {
			.shaderDrawParametersFeatures = drawParametersFeatures,
			.descriptorIndexingFeatures = this->physicalDevice.GetDescriptorIndexingFeatures()
		}));

		this->swapchain = Vk::Swapchain(this->physicalDevice, this->device, this->surface,{
			presentMode, { 0, 0 }, VK_SAMPLE_COUNT_1_BIT
		});
	}
	Surface::~Surface()
	{
		if (this->instance == nullptr) return;
		this->device.WaitIdle();
	}
	Surface::Surface(Surface&& other) noexcept : instance(other.instance), surface(std::move(other.surface)), window(other.window), physicalDevice(other.physicalDevice), device(std::move(other.device)), swapchain(std::move(other.swapchain)), framebuffers(std::move(other.framebuffers)), swapchainRecreationCallback(other.swapchainRecreationCallback), presentMode(other.presentMode), needsRecreation(other.needsRecreation)
	{
		other.instance = nullptr;
		other.window = nullptr;
		other.swapchainRecreationCallback = nullptr;
	}
	Surface& Surface::operator=(Surface&& other) noexcept
	{
		if (this == &other) return *this;
		if (this->instance != nullptr) this->~Surface();

		this->instance = other.instance; other.instance = nullptr;
		this->surface = std::move(other.surface);
		this->window = other.window; other.window = nullptr;
		this->physicalDevice = std::move(other.physicalDevice);
		this->device = std::move(other.device);
		this->swapchain = std::move(other.swapchain);
		this->framebuffers = std::move(other.framebuffers);
		this->swapchainRecreationCallback = other.swapchainRecreationCallback; other.swapchainRecreationCallback = nullptr;
		this->presentMode = other.presentMode;
		this->needsRecreation = other.needsRecreation;

		return *this;
	}
	void* Surface::GetWindow() const { return this->window; }
	Vk::PhysicalDevice& Surface::GetPhysicalDevice() { return this->physicalDevice; }
	Vk::Device& Surface::GetDevice() { return this->device; }
	Vk::Swapchain& Surface::GetSwapchain() { return this->swapchain; }
	Vk::Swapchain::Image& Surface::GetImage(size_t index) { return this->swapchain.GetFramebuffer(index); }
	void Surface::RecreateSwapchain(VkPresentModeKHR presentMode)
	{
		this->device.WaitIdle();

		int width = 0, height = 0;
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(static_cast<GLFWwindow*>(window), &width, &height);
			glfwWaitEvents();
		}

		this->swapchain.~Swapchain();
		this->swapchain = Vk::Swapchain(this->physicalDevice, this->device, this->surface, {
			presentMode, { 0, 0 }, VK_SAMPLE_COUNT_1_BIT
		});
		this->CallRecreationCallback();
	}
	void Surface::CallRecreationCallback()
	{
		if (this->swapchainRecreationCallback != nullptr) this->swapchainRecreationCallback(*this, swapchain.GetExtent().width, swapchain.GetExtent().height, presentMode);
	}
	void Surface::AcquireNextImage(VkSemaphore imageAvailableSemaphore, uint64_t timeout)
	{
		VkResult result = this->swapchain.AcquireNextImage(imageAvailableSemaphore, timeout);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			this->needsRecreation = true;
		}
	}
	void Surface::Present(VkQueue queue, VkSemaphore renderFinishSemaphore)
	{
		VkResult result = this->swapchain.Present(queue, renderFinishSemaphore);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->needsRecreation) {
			this->RecreateSwapchain(this->presentMode);
			this->needsRecreation = false;
		}
	}
}