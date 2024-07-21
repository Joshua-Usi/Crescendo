#pragma once

#include "common.hpp"
#include "Volk/volk.h"
#include "vma/vk_mem_alloc.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class Allocator
	{
	private:
		VkDevice device;
		VmaAllocator allocator;
	public:
		Allocator();
		Allocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
		~Allocator();
		Allocator(const Allocator&) = delete;
		Allocator& operator=(const Allocator&) = delete;
		Allocator(Allocator&& other) noexcept;
		Allocator& operator=(Allocator&& other) noexcept;
		operator VmaAllocator() const;
		VmaAllocator GetAllocator() const;
		VkDevice GetDevice() const;
	};
}