#pragma once

#include "common.hpp"
#include "VkBootstrap/VkBootstrap.h"
#include "Instance.hpp"
#include "Surface.hpp"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class PhysicalDevice
	{
	private:
		vkb::PhysicalDevice vkbPhysicalDevice;
	public:
		struct PhysicalDeviceSelectionCriteria
		{
			enum class PreferredDeviceType : uint8_t { Discrete = 0, Integrated = 1 };
			PreferredDeviceType preferredDeviceType;
			uint32_t major, minor;
			VkPhysicalDeviceFeatures requiredDeviceFeatures;
		};
	public:
		PhysicalDevice() = default;
		PhysicalDevice(const Instance& instance, const Surface& surface, const PhysicalDeviceSelectionCriteria& selectionCriteria);
		~PhysicalDevice() = default;
		PhysicalDevice(const PhysicalDevice&) = default;
		PhysicalDevice& operator=(const PhysicalDevice&) = default;
	public:
		operator VkPhysicalDevice() const;
		operator const vkb::PhysicalDevice& () const;
		VkPhysicalDevice GetVkPhysicalDevice() const;
		const VkPhysicalDeviceProperties& GetProperties() const;
		const VkPhysicalDeviceFeatures& GetFeatures() const;
		const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const;
		VkPhysicalDeviceDescriptorIndexingFeatures GetDescriptorIndexingFeatures() const;
	};
}