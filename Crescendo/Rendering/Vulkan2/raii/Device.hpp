#pragma once

#include "common.hpp"
#include "VkBootstrap/VkBootstrap.h"
#include "PhysicalDevice.hpp"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class Device
	{
	public:
		struct Queue { VkQueue queue;  uint32_t family; Queue() = default; Queue(VkQueue queue, uint32_t family) : queue(queue), family(family) {} };
		struct DeviceCreateInfo
		{
			VkPhysicalDeviceShaderDrawParametersFeatures shaderDrawParametersFeatures;
			VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures;
		};
	private:
		VkDevice device;
		Queue universal, transfer, compute;
	public:
		Device();
		Device(const PhysicalDevice& physicalDevice, const DeviceCreateInfo& createInfo);
		~Device();
		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;
		Device(Device&& other) noexcept;
		Device& operator=(Device&& other) noexcept;
	public:
		const Queue& GetUniversalQueue() const;
		const Queue& GetTransferQueue() const;
		const Queue& GetComputeQueue() const;
		void WaitIdle() const;
	public:
		operator VkDevice() const;
		VkDevice GetDevice() const;
	};
}