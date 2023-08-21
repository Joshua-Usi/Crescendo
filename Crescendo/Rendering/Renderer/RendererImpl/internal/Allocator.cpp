#pragma once

#define VMA_IMPLEMENTATION

#include "Core/common.hpp"
#include "Allocator.hpp"
#include "VMA/vk_mem_alloc.h"
#include "Create.hpp"

namespace Crescendo::internal
{
	Allocator& Allocator::Initialise()
	{
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = physicalDevice;
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;
		vmaCreateAllocator(&allocatorInfo, &this->allocator);

		return *this;
	}
	void Allocator::Destroy()
	{
		vmaDestroyAllocator(this->allocator);
	}
	Allocator::Buffer Allocator::CreateBuffer(size_t allocationSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
	{
		Buffer buffer;

		VkBufferCreateInfo bufferInfo = Create::BufferCreateInfo(nullptr, 0, allocationSize, usage, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr);
		VmaAllocationCreateInfo vmaAllocInfo{};
		vmaAllocInfo.usage = memoryUsage;

		CS_ASSERT(vmaCreateBuffer(this->allocator, &bufferInfo, &vmaAllocInfo, &buffer.buffer, &buffer.allocation, nullptr) == VK_SUCCESS, "Failed to create buffer!");

		// Only map if not GPU only
		if (memoryUsage != VMA_MEMORY_USAGE_GPU_ONLY) vmaMapMemory(this->allocator, buffer.allocation, &buffer.memoryLocation);
		return buffer;
	}
	void Allocator::DestroyBuffer(Allocator::Buffer& buffer)
	{
		if (buffer.memoryLocation != nullptr) vmaUnmapMemory(this->allocator, buffer.allocation);
		vmaDestroyBuffer(this->allocator, buffer.buffer, buffer.allocation);
	}
	Allocator::Image Allocator::CreateImage(const VkImageCreateInfo& imageInfo, VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags requiredFlags)
	{
		Image image;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = memoryUsage;
		allocInfo.requiredFlags = requiredFlags;

		CS_ASSERT(vmaCreateImage(this->allocator, &imageInfo, &allocInfo, &image.image, &image.allocation, nullptr) == VK_SUCCESS, "Failed to create image!");

		return image;
	}
	// Create image with image view, allocator will fill viewInfo.image attribute;
	Allocator::Image Allocator::CreateImage(const VkImageCreateInfo& imageInfo, VkImageViewCreateInfo& viewInfo, VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags requiredFlags)
	{

		Image image = this->CreateImage(imageInfo, memoryUsage, requiredFlags);
		viewInfo.image = image.image;
		CS_ASSERT(vkCreateImageView(this->device, &viewInfo, nullptr, &image.imageView) == VK_SUCCESS, "Failed to create image view!");
	
		return image;
	}
	void Allocator::DestroyImage(Allocator::Image& image)
	{
		if (image.imageView != nullptr) vkDestroyImageView(this->device, image.imageView, nullptr);
		// Some images can be from the swap chain
		if (image.allocation != nullptr) vmaDestroyImage(this->allocator, image.image, image.allocation);
	}
}