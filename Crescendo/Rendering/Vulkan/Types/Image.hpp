#pragma once

#include "common.hpp"

#include "Volk/volk.h"
#include "vma/vk_mem_alloc.h"

CS_NAMESPACE_BEGIN::Vulkan
{
	struct Image
	{
	private:
		VkDevice device;
		VmaAllocator allocator;
		VmaAllocation allocation;
	public:
		VkImage image;
		VkImageView imageView;
	public:
		// Constructors
		Image() : image(nullptr), imageView(nullptr), device(nullptr), allocator(nullptr), allocation(nullptr) {}
		Image(VkImage image, VkImageView imageView, VkDevice device, VmaAllocator allocator = nullptr, VmaAllocation allocation = nullptr) :
			image(image), imageView(imageView), device(device), allocator(allocator), allocation(allocation) {}
		// Constructor for swapchain images
		Image(VkDevice, VkImage image, VkImageView imageView) :
			image(image), imageView(imageView), device(nullptr), allocator(nullptr), allocation(nullptr) {}
		// Destructors
		~Image()
		{
			if (this->device == nullptr) return;
			vkDestroyImageView(this->device, this->imageView, nullptr);
			if (this->allocator == nullptr) return;
			vmaDestroyImage(this->allocator, this->image, this->allocation);
		}
		// No copy
		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		// Move
		Image(Image&& other) noexcept :
			image(other.image), imageView(other.imageView), device(other.device), allocator(other.allocator), allocation(other.allocation)
		{
			other.image = nullptr;
			other.imageView = nullptr;
			other.device = nullptr;
			other.allocator = nullptr;
			other.allocation = nullptr;
		}
		Image& operator=(Image&& other) noexcept
		{
			if (this != &other)
			{
				image = other.image; other.image = nullptr;
				imageView = other.imageView; other.imageView = nullptr;
				device = other.device; other.device = nullptr;
				allocator = other.allocator; other.allocator = nullptr;
				allocation = other.allocation; other.allocation = nullptr;
			}
			return *this;
		}
	public:
		operator VkImage() const { return image; }
	};
}