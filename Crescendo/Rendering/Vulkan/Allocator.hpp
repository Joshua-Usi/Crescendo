#pragma once

#include "volk/volk.h"
#include "vma/vk_mem_alloc.h"

#include "Types/Buffer.hpp"
#include "Types/Image.hpp"

#include <vector>

namespace Crescendo::Vulkan
{
	class Allocator
	{
	private:
		VmaAllocator allocator;
		VkDevice device;
	public:
		Allocator() = default;
		Allocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
		~Allocator();
		// No copy
		Allocator(const Allocator&) = delete;
		Allocator& operator=(const Allocator&) = delete;
		// Move
		Allocator(Allocator&& other) noexcept;
		Allocator& operator=(Allocator&& other) noexcept;

		void Destroy();

		Buffer CreateBuffer(size_t allocationSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
		Image CreateImage(const VkImageCreateInfo& imageInfo, VmaMemoryUsage memoryUsage);
	};
}