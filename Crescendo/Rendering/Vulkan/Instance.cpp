#include "Instance.hpp"
#include "Core/common.hpp"

#include "GLFW/glfw3.h"

namespace Crescendo::Vulkan
{
	bool Instance::isVolkInitialised = false;

	Instance::Instance(bool useValidationLayers, const std::string& appName, const std::string& engineName, void* windowPtr) : windowPtr(windowPtr)
	{
		if (!isVolkInitialised)
		{
			const VkResult result = volkInitialize();
			CS_ASSERT(result == VK_SUCCESS, "Failed to initialise volk!");
			isVolkInitialised = result == VK_SUCCESS;
		}

		// Create instance and debug messenger
		vkb::Instance instance = vkb::InstanceBuilder(vkGetInstanceProcAddr)
			.set_app_name(appName.c_str())
			.set_engine_name(engineName.c_str())
			.request_validation_layers(useValidationLayers)
			.require_api_version(1, 3, 0) // Using 1.3 at minimum
			.use_default_debug_messenger()
			.build().value();
		this->instance = instance.instance;
		this->debugMessenger = instance.debug_messenger;

		volkLoadInstance(this->instance);

		// Create surface
		CS_ASSERT(glfwCreateWindowSurface(this->instance, static_cast<GLFWwindow*>(windowPtr), nullptr, &this->surface) == VK_SUCCESS, "Failed to create window surface!");

		// Device features
		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.fillModeNonSolid = VK_TRUE;
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.sampleRateShading = VK_TRUE;

		// Select physical device
		this->vkbPhysicalDevice = vkb::PhysicalDeviceSelector(instance)
			.set_minimum_version(1, 3) // We use bindless, since we are using 1.3 we don't need to enable it
			.prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
			.set_surface(this->surface)
			.set_required_features(deviceFeatures)
			.select().value();

		// Push to deletion queue
		this->deletionQueue.push([&]() {
			vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
			vkb::destroy_debug_utils_messenger(this->instance, this->debugMessenger);
			vkDestroyInstance(this->instance, nullptr);
		});
	}
	Instance::~Instance()
	{
		this->deletionQueue.flush();
	}
	Instance::Instance(Instance&& other) noexcept
		: vkbPhysicalDevice(other.vkbPhysicalDevice), instance(other.instance), debugMessenger(other.debugMessenger), surface(other.surface), windowPtr(other.windowPtr)
	{
		other.instance = nullptr;
		other.debugMessenger = nullptr;
		other.surface = nullptr;
		other.windowPtr = nullptr;
		other.deletionQueue.clear();
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
			other.deletionQueue.clear();
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