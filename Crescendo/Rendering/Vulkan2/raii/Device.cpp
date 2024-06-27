#include "Device.hpp"
#include "Volk/volk.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	Device::Device() : device(nullptr), universal(nullptr, 0), transfer(nullptr, 0), compute(nullptr, 0) {}
	Device::Device(const PhysicalDevice& physicalDevice, const DeviceCreateInfo& createInfo)
	{
		vkb::Result<vkb::Device> deviceResult = vkb::DeviceBuilder(physicalDevice)
			.add_pNext(const_cast<VkPhysicalDeviceShaderDrawParametersFeatures*>(&createInfo.shaderDrawParametersFeatures))
			.add_pNext(const_cast<VkPhysicalDeviceDescriptorIndexingFeatures*>(&createInfo.descriptorIndexingFeatures))
			.build();
		if (!deviceResult) cs_std::console::error("Failed to build device: ", deviceResult.error().message());

		const vkb::Device& device = deviceResult.value();
		this->device = device;

		// Get queues
		auto universal = device.get_queue(vkb::QueueType::graphics);
		auto transfer = device.get_queue(vkb::QueueType::transfer);
		auto compute = device.get_queue(vkb::QueueType::compute);

		const bool hasDedicatedTransfer = universal.value() == transfer.value();
		const bool hasDedicatedCompute = universal.value() == transfer.value();

		if (!universal.has_value()) cs_std::console::fatal("Failed to get universal queue: ", universal.error().message());
		if (!hasDedicatedTransfer) cs_std::console::warn("Device does not have a dedicated transfer queue. Falling back to universal"); // Fallback to universal
		if (!hasDedicatedCompute) cs_std::console::warn("Device does not have a dedicated compute queue. Falling back to universal"); // Fallback to universal

		this->universal.queue = universal.value();
		this->transfer.queue = (hasDedicatedTransfer) ? transfer.value() : this->universal.queue; // Fallback to universal
		this->compute.queue = (hasDedicatedCompute) ? compute.value() : this->universal.queue; // Fallback to universal

		this->universal.family = device.get_queue_index(vkb::QueueType::graphics).value();
		this->transfer.family = (hasDedicatedTransfer) ? device.get_queue_index(vkb::QueueType::transfer).value() : this->universal.family; // Fallback to universal
		this->compute.family = (hasDedicatedCompute) ? device.get_queue_index(vkb::QueueType::compute).value() : this->universal.family; // Fallback to universal
	}
	Device::~Device()
	{
		vkDestroyDevice(this->device, nullptr);
	}
	Device::Device(Device&& other) noexcept : device(other.device), universal(other.universal), transfer(other.transfer), compute(other.compute)
	{
		other.device = nullptr;
	}
	Device& Device::operator=(Device&& other) noexcept
	{
		if (this == &other) return *this;
		this->device = other.device; other.device = nullptr;
		this->universal = other.universal;
		this->transfer = other.transfer;
		this->compute = other.compute;
		return *this;
	}
	Device::operator VkDevice() const { return device; }
	VkDevice Device::GetDevice() const { return device; }
	const Device::Queue& Device::GetUniversalQueue() const { return universal; }
	const Device::Queue& Device::GetTransferQueue() const { return transfer; }
	const Device::Queue& Device::GetComputeQueue() const { return compute; }
	void Device::WaitIdle() const { vkDeviceWaitIdle(device); }
}