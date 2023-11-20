#include "Instance.hpp"
#include "Core/common.hpp"

#include "GLFW/glfw3.h"

namespace Crescendo::Vulkan
{
	bool Instance::isVolkInitialised = false;

	Instance::Instance(bool useValidationLayers, const std::string& appName, const std::string& engineName, void* windowPtr) : windowPtr(windowPtr)
	{
		constexpr uint32_t CS_VK_MAJOR = 1, CS_VK_MINOR = 3, CS_VK_PATCH = 0;

		if (!isVolkInitialised)
		{
			const VkResult result = volkInitialize();
			CS_ASSERT(result == VK_SUCCESS, "Failed to initialise volk!");
			isVolkInitialised = result == VK_SUCCESS;
		}

		// Create instance and debug messenger
		const vkb::Result<vkb::Instance> instanceResult = vkb::InstanceBuilder(vkGetInstanceProcAddr)
			.set_app_name(appName.c_str()).set_engine_name(engineName.c_str())
			.request_validation_layers(useValidationLayers).require_api_version(CS_VK_MAJOR, CS_VK_MINOR, CS_VK_PATCH)
			.use_default_debug_messenger().build();
		if (!instanceResult) cs_std::console::fatal("Failed to create Vulkan instance!", instanceResult.error().message());
		
		const vkb::Instance instance = instanceResult.value();
		this->instance = instance.instance;
		this->debugMessenger = instance.debug_messenger;
		volkLoadInstance(this->instance);

		// Create surface
		if (glfwCreateWindowSurface(this->instance, static_cast<GLFWwindow*>(windowPtr), nullptr, &this->surface) != VK_SUCCESS) cs_std::console::fatal("Failed to create window surface!");

		// Device features
		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.fillModeNonSolid = VK_TRUE;
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.sampleRateShading = VK_TRUE;

		// Select physical device
		const vkb::Result<vkb::PhysicalDevice> physicalDeviceResult = vkb::PhysicalDeviceSelector(instance).set_minimum_version(CS_VK_MAJOR, CS_VK_MINOR).set_surface(this->surface).set_required_features(deviceFeatures).select();
		if (!physicalDeviceResult) cs_std::console::fatal("Failed to select Vulkan physical device!", physicalDeviceResult.error().message());
		this->vkbPhysicalDevice = physicalDeviceResult.value();
	}
	Instance::~Instance()
	{
		if (this->instance == nullptr) return;
		vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
		vkb::destroy_debug_utils_messenger(this->instance, this->debugMessenger);
		vkDestroyInstance(this->instance, nullptr);
	}
	Instance::Instance(Instance&& other) noexcept
		: vkbPhysicalDevice(other.vkbPhysicalDevice), instance(other.instance), debugMessenger(other.debugMessenger), surface(other.surface), windowPtr(other.windowPtr)
	{
		other.instance = nullptr;
		other.debugMessenger = nullptr;
		other.surface = nullptr;
		other.windowPtr = nullptr;
	}
	Instance& Instance::operator=(Instance&& other) noexcept
	{
		if (this != &other)
		{
			this->vkbPhysicalDevice = std::move(other.vkbPhysicalDevice);
			this->instance = other.instance;
			this->debugMessenger = other.debugMessenger;
			this->surface = other.surface;
			this->windowPtr = other.windowPtr;

			other.instance = nullptr;
			other.debugMessenger = nullptr;
			other.surface = nullptr;
			other.windowPtr = nullptr;
		}
		return *this;
	}
	Device Instance::CreateDevice(uint32_t descriptorSetsPerPool)
	{
		VkPhysicalDeviceShaderDrawParametersFeatures drawParametersFeatures = {};
		drawParametersFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
		drawParametersFeatures.shaderDrawParameters = VK_TRUE;

		const vkb::Device deviceResult = vkb::DeviceBuilder(this->vkbPhysicalDevice).add_pNext(&drawParametersFeatures).build().value();
		volkLoadDevice(deviceResult.device);
		return Device(deviceResult, this->instance, descriptorSetsPerPool, this->GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment);
	}
	Swapchain Instance::CreateSwapchain(VkDevice device, VkPresentModeKHR presentMode, VkExtent2D extent)
	{
		return Swapchain(this->vkbPhysicalDevice, device, this->surface, presentMode, extent);
	}
}