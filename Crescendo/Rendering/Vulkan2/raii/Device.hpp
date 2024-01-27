#pragma once

#include "common.hpp"
#include "VkBootstrap/VkBootstrap.h"
#include "PhysicalDevice.hpp"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class Device
	{
	private:
		struct Queue { VkQueue queue;  uint32_t family; Queue() = default; Queue(VkQueue queue, uint32_t family) : queue(queue), family(family) {} };
	private:
		VkDevice device;
		Queue universal, transfer, compute;
	public:
		Device();
		Device(const PhysicalDevice& physicalDevice, const VkPhysicalDeviceShaderDrawParametersFeatures& deviceFeatures);
		~Device();
		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;
		Device(Device&& other) noexcept;
		Device& operator=(Device&& other) noexcept;

		operator VkDevice() const;
		VkDevice GetDevice() const;

		void WaitIdle() const;
	};
}