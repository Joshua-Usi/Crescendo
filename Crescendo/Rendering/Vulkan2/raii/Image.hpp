#pragma once

#include "common.hpp"
#include "vulkan/vulkan.h"
#include "vma/vk_mem_alloc.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class Image
	{
	private:
		VkDevice device;
		VmaAllocator allocator;
		VmaAllocation allocation;
		VkImage image;
		VkImageView imageView;
	public:
		Image();
		Image(VkDevice device, VmaAllocator allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo);
		~Image();
		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		Image(Image&& other) noexcept;
		Image& operator=(Image&& other) noexcept;
	public:
		operator VkImage() const;
		operator VkImageView() const;
	};
}