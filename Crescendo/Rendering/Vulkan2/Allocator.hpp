#pragma once

#include "common.hpp"
#include "RAII.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	class Allocator
	{
	private:
		Vk::Allocator allocator;
	public:
		Allocator() = default;
		Allocator(const Vk::Instance& instance, const Vk::PhysicalDevice& physicalDevice, const Vk::Device& device);
		~Allocator() = default;
		Allocator(const Allocator&) = delete;
		Allocator& operator=(const Allocator&) = delete;
		Allocator(Allocator&& other) noexcept;
		Allocator& operator=(Allocator&& other) noexcept;
		Vk::Buffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
		Vk::Image CreateImage(VkImageCreateInfo& imageInfo, VmaMemoryUsage memoryUsage);
	};
}