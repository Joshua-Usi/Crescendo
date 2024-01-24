#pragma once

#include "common.hpp"
#include "Volk/volk.h"

#include "VkBootstrap/VkBootstrap.h"

CS_NAMESPACE_BEGIN::Vulkan
{
	// Forward declarations
	class Instance;
	class Surface;

	class PhysicalDevice
	{
	private:
		vkb::PhysicalDevice vkbPhysicalDevice;
	public:
		enum class PreferredDeviceType : uint8_t { Discrete = 0, Integrated = 1 };
	public:
		PhysicalDevice() = default;
		PhysicalDevice(uint32_t major, uint32_t minor, const Instance& instance, const Surface& surface, const VkPhysicalDeviceFeatures& deviceFeatures);
		~PhysicalDevice() = default;
		// Copy
		PhysicalDevice(const PhysicalDevice&) = default;
		PhysicalDevice& operator=(const PhysicalDevice&) = default;

		operator VkPhysicalDevice() const { return vkbPhysicalDevice.physical_device; }
		operator const vkb::PhysicalDevice&() const { return vkbPhysicalDevice; }

		VkPhysicalDevice GetVkPhysicalDevice() const { return vkbPhysicalDevice.physical_device; }

		const VkPhysicalDeviceProperties& GetProperties() const { return vkbPhysicalDevice.properties; }
		const VkPhysicalDeviceFeatures& GetFeatures() const { return vkbPhysicalDevice.features; }
		const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const { return vkbPhysicalDevice.memory_properties; }
	};
}
