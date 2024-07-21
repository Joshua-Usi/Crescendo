#include "Allocator.hpp"
#include "Create.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	Allocator::Allocator(const Vk::Instance& instance, const Vk::PhysicalDevice& physicalDevice, const Vk::Device& device) : allocator(instance, physicalDevice, device) {}
	Allocator::Allocator(Allocator&& other) noexcept : allocator(std::move(other.allocator)) {}
	Allocator& Allocator::operator=(Allocator&& other) noexcept
	{
		if (this == &other) return *this;
		this->allocator = std::move(other.allocator);
		return *this;
	}
	Vk::Buffer Allocator::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
	{
		VkBufferCreateInfo bufferInfo = Create::BufferCreateInfo(0, size, usage, VK_SHARING_MODE_EXCLUSIVE, nullptr);
		VmaAllocationCreateInfo vmaAllocInfo{};
		vmaAllocInfo.usage = memoryUsage;
		return Vk::Buffer(this->allocator, bufferInfo, vmaAllocInfo);
	}
	Vk::Image Allocator::CreateImage(const VkImageCreateInfo& imageInfo, VmaMemoryUsage memoryUsage)
	{
		VmaAllocationCreateInfo vmaAllocInfo {};
		vmaAllocInfo.usage = memoryUsage;
		return Vk::Image(this->allocator.GetDevice(), this->allocator, imageInfo, vmaAllocInfo);
	}
	Allocator::operator const Vk::Allocator& () const { return this->allocator; }
	Allocator::operator VmaAllocator() const { return this->allocator; }
}