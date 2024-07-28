#pragma once

#include "common.hpp"
#include "RAII.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	class Device
	{
	private:
		Vk::Device device;
		Vk::Allocator allocator;
	public:
		Device();
		Device(const Vk::Instance& instance, const Vk::PhysicalDevice& physicalDevice, const Vk::Device::DeviceCreateInfo& createInfo);
		~Device() = default;
		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;
		Device(Device&& other) noexcept;
		Device& operator=(Device&& other) noexcept;
	public:
		Vk::Device::Queue GetUniversalQueue() const;
		Vk::Device::Queue GetTransferQueue() const;
		Vk::Device::Queue GetComputeQueue() const;
	public:
		void WaitIdle();
		operator const Vk::Device& () const;
		operator VkDevice() const;
		Vk::Allocator& GetAllocator();
	};
}