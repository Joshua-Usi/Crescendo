#include "PhysicalDevice.hpp"
#include "Volk/volk.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	PhysicalDevice::PhysicalDevice(const Instance& instance, const Surface& surface, const PhysicalDeviceSelectionCriteria& selectionCriteria)
	{
		// Select physical device
		// We select multiple because even if the default is to choose discrete
		// Sometimes it can choose integrated, then we need to determine which one we want
		vkb::Result<std::vector<vkb::PhysicalDevice>> physicalDeviceResult = vkb::PhysicalDeviceSelector(instance)
			.set_minimum_version(selectionCriteria.major, selectionCriteria.minor)
			.prefer_gpu_device_type(selectionCriteria.preferredDeviceType == PhysicalDeviceSelectionCriteria::PreferredDeviceType::Discrete ? vkb::PreferredDeviceType::discrete : vkb::PreferredDeviceType::integrated)
			.set_surface(surface).set_required_features(selectionCriteria.requiredDeviceFeatures).select_devices();
		if (!physicalDeviceResult) cs_std::console::fatal("Failed to select Vulkan physical device!", physicalDeviceResult.error().message());

		// Try to find discrete GPU
		bool foundPreferred = false;
		VkPhysicalDeviceType preferredDeviceType = (selectionCriteria.preferredDeviceType == PhysicalDeviceSelectionCriteria::PreferredDeviceType::Discrete) ? VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU : VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
		for (const auto& device : physicalDeviceResult.value())
		{
			if (device.properties.deviceType == preferredDeviceType)
			{
				this->vkbPhysicalDevice = device;
				foundPreferred = true;
				break;
			}
		}

		// If we didn't find the preferred type GPU, select the first device by default
		if (!foundPreferred)
		{
			this->vkbPhysicalDevice = physicalDeviceResult.value()[0];
			cs_std::console::warn("Failed to find preferred GPU type, falling back to first device");
		}

		// Check for GPU features
		VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures = {};
		descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		descriptorIndexingFeatures.pNext = nullptr;

		VkPhysicalDeviceFeatures2 features2 = {};
		features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features2.pNext = &descriptorIndexingFeatures;

		vkGetPhysicalDeviceFeatures2(this->vkbPhysicalDevice, &features2);

		CS_ASSERT(
			descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing && descriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind &&
			descriptorIndexingFeatures.shaderUniformBufferArrayNonUniformIndexing && descriptorIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind &&
			descriptorIndexingFeatures.shaderStorageBufferArrayNonUniformIndexing && descriptorIndexingFeatures.descriptorBindingStorageBufferUpdateAfterBind,
			"Physical device does not support bindless"
		);
		cs_std::console::info("Selected physical device: ", this->vkbPhysicalDevice.properties.deviceName);
	}
	PhysicalDevice::operator VkPhysicalDevice() const { return vkbPhysicalDevice; }
	PhysicalDevice::operator const vkb::PhysicalDevice& () const { return vkbPhysicalDevice; }
	VkPhysicalDevice PhysicalDevice::GetVkPhysicalDevice() const { return vkbPhysicalDevice; }
	const VkPhysicalDeviceProperties& PhysicalDevice::GetProperties() const { return vkbPhysicalDevice.properties; }
	const VkPhysicalDeviceFeatures& PhysicalDevice::GetFeatures() const { return vkbPhysicalDevice.features; }
	const VkPhysicalDeviceMemoryProperties& PhysicalDevice::GetMemoryProperties() const { return vkbPhysicalDevice.memory_properties; }
	VkPhysicalDeviceDescriptorIndexingFeatures PhysicalDevice::GetDescriptorIndexingFeatures() const
	{
		VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures {};
		descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		descriptorIndexingFeatures.pNext = nullptr;
		VkPhysicalDeviceFeatures2 deviceFeatures {};
		deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		deviceFeatures.pNext = &descriptorIndexingFeatures;
		vkGetPhysicalDeviceFeatures2(vkbPhysicalDevice, &deviceFeatures);
		return descriptorIndexingFeatures;
	}
}