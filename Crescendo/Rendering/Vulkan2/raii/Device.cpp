#include "Device.hpp"
#include "Volk/volk.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	Device::Device() : device(nullptr), universal(nullptr, 0), transfer(nullptr, 0), compute(nullptr, 0) {}
	Device::Device(const PhysicalDevice& physicalDevice, const VkPhysicalDeviceShaderDrawParametersFeatures& deviceFeatures)
	{
		vkb::Result<vkb::Device> deviceResult = vkb::DeviceBuilder(physicalDevice).add_pNext(const_cast<VkPhysicalDeviceShaderDrawParametersFeatures*>(&deviceFeatures)).build();
		if (!deviceResult) cs_std::console::error("Failed to build device: ", deviceResult.error().message());

		const vkb::Device& device = deviceResult.value();
		this->device = device;

		// Get queues
		auto universal = device.get_queue(vkb::QueueType::graphics);
		auto transfer = device.get_queue(vkb::QueueType::transfer);
		auto compute = device.get_queue(vkb::QueueType::compute);

		if (!universal) cs_std::console::fatal("Failed to get universal queue");
		if (!transfer) cs_std::console::warn("Failed to get transfer queue: ", transfer.error().message(), ". Falling back to universal"); // Fallback to universal
		if (!compute) cs_std::console::warn("Failed to get compute queue: ", compute.error().message(), ". Falling back to universal"); // Fallback to universal

		this->universal.queue = universal.value();
		this->transfer.queue = (transfer) ? transfer.value() : this->universal.queue; // Fallback to universal
		this->compute.queue = (compute) ? compute.value() : this->universal.queue; // Fallback to universal

		this->universal.family = device.get_queue_index(vkb::QueueType::graphics).value();
		this->transfer.family = device.get_queue_index((transfer) ? vkb::QueueType::transfer : vkb::QueueType::graphics).value(); // Fallback to universal
		this->compute.family = device.get_queue_index((compute) ? vkb::QueueType::compute : vkb::QueueType::graphics).value(); // Fallback to universal
	}
	Device::~Device()
	{
		vkDestroyDevice(this->device, nullptr);
	}
	Device::Device(Device&& other) noexcept : device(other.device) { other.device = nullptr; }
	Device& Device::operator=(Device&& other) noexcept
	{
		if (this == &other) return *this;
		this->device = other.device; other.device = nullptr;
		return *this;
	}
	Device::operator VkDevice() const { return device; }
	VkDevice Device::GetDevice() const { return device; }
	void Device::WaitIdle() const { vkDeviceWaitIdle(device); }
}