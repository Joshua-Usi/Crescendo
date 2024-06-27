#include "Device.hpp"
#include "Create.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	Device::Device() : device(), allocator(), descriptorManager() {}
	Device::Device(const Vk::Instance& instance, const Vk::PhysicalDevice& physicalDevice, const DeviceSpecification& spec) :
		device(physicalDevice, spec.deviceCreateInfo), allocator(instance, physicalDevice, this->device), descriptorManager(this->device, spec.descriptorManagerSpec) {}
	Device::Device(Device&& other) noexcept : device(std::move(other.device)), allocator(std::move(other.allocator)), descriptorManager(std::move(other.descriptorManager)) {}
	Device& Device::operator=(Device&& other) noexcept
	{
		if (this != &other)
		{
			this->device = std::move(other.device);
			this->allocator = std::move(other.allocator);
			this->descriptorManager = std::move(other.descriptorManager);
		}
		return *this;
	}
	Vk::Device::Queue Device::GetUniversalQueue() const { return this->device.GetUniversalQueue(); }
	Vk::Device::Queue Device::GetTransferQueue() const { return this->device.GetTransferQueue(); }
	Vk::Device::Queue Device::GetComputeQueue() const { return this->device.GetComputeQueue(); }
	Vk::Buffer Device::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
	{
		return this->allocator.CreateBuffer(size, usage, memoryUsage);
	}
	Vk::Image Device::CreateImage(const VkImageCreateInfo& createInfo, VmaMemoryUsage memoryUsage)
	{
		return this->allocator.CreateImage(createInfo, memoryUsage);
	}
	Vk::Sampler Device::CreateSampler(const VkSamplerCreateInfo& createInfo)
	{
		return Vk::Sampler(*this, createInfo);
	}
	void Device::WaitIdle() { this->device.WaitIdle(); }
	Device::operator const Vk::Device& () const { return this->device; }
	Device::operator VkDevice() const { return this->device; }
	BindlessDescriptorManager& Device::GetBindlessDescriptorManager() { return this->descriptorManager; };
}