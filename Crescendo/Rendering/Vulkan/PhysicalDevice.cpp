#include "PhysicalDevice.hpp"

#include "Instance.hpp"
#include "Surface.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	PhysicalDevice::PhysicalDevice(uint32_t major, uint32_t minor, const Instance& instance, const Surface& surface, const VkPhysicalDeviceFeatures& deviceFeatures)
	{
		// Select physical device
		// We select multiple because even if the default is to choose discrete
		// Sometimes it can choose integrated, then we need to determine which one we want
		auto physicalDeviceResult = vkb::PhysicalDeviceSelector(instance).set_minimum_version(major, minor).set_surface(surface).set_required_features(deviceFeatures).select_devices();
		if (!physicalDeviceResult) cs_std::console::fatal("Failed to select Vulkan physical device!", physicalDeviceResult.error().message());

		// Try to find discrete GPU
		bool foundDiscrete = false;
		for (const auto& device : physicalDeviceResult.value())
		{
			if (device.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				this->vkbPhysicalDevice = device;
				foundDiscrete = true;
				break;
			}
		}
		// If we didn't find a discrete GPU, select the first device by default
		if (!foundDiscrete)
		{
			this->vkbPhysicalDevice = physicalDeviceResult.value()[0];
			cs_std::console::log("Failed to find discrete GPU, falling back to first device");
		}
	}
};