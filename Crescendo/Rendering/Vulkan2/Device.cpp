#include "Device.hpp"
#include "Create.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	Device::Device() : device(), allocator() {}
	Device::Device(const Vk::Instance& instance, const Vk::PhysicalDevice& physicalDevice, const Vk::Device::DeviceCreateInfo& createInfo) :
		device(physicalDevice, createInfo), allocator(instance, physicalDevice, this->device) {}
	Device::Device(Device&& other) noexcept : device(std::move(other.device)), allocator(std::move(other.allocator)) {}
	Device& Device::operator=(Device&& other) noexcept
	{
		if (this != &other)
		{
			this->device = std::move(other.device);
			this->allocator = std::move(other.allocator);
		}
		return *this;
	}
	Vk::Device::Queue Device::GetUniversalQueue() const { return this->device.GetUniversalQueue(); }
	Vk::Device::Queue Device::GetTransferQueue() const { return this->device.GetTransferQueue(); }
	Vk::Device::Queue Device::GetComputeQueue() const { return this->device.GetComputeQueue(); }
	void Device::WaitIdle() { this->device.WaitIdle(); }
	Device::operator const Vk::Device& () const { return this->device; }
	Device::operator VkDevice() const { return this->device; }
	Vk::Allocator& Device::GetAllocator() { return this->allocator; };
}