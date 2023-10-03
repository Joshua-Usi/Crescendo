#pragma once

#define VMA_IMPLEMENTATION

#include "Core/common.hpp"
#include "Allocator.hpp"
#include "VMA/vk_mem_alloc.h"
#include "../Create.hpp"

namespace Crescendo::internal
{
	Allocator& Allocator::Initialise(VkInstance instance, VkPhysicalDevice physicalDevice)
	{
		VmaAllocatorCreateInfo allocatorInfo {};
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;
		allocatorInfo.physicalDevice = physicalDevice;

		vmaCreateAllocator(&allocatorInfo, &this->allocator);

		return *this;
	}
	void Allocator::Destroy()
	{
		vmaDestroyAllocator(this->allocator);
	}
	Allocator::Buffer Allocator::CreateBuffer(size_t allocationSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
	{
		Buffer buffer {};

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
	void Allocator::DestroyBuffers(std::vector<Buffer>& buffers)
	{
		for (Buffer& buffer : buffers) this->DestroyBuffer(buffer);
	}
	// Create image with image view, allocator will fill viewInfo.image attribute;
	Allocator::Image Allocator::CreateImage(const VkImageCreateInfo& imageInfo, VmaMemoryUsage memoryUsage)
	{
		// Maps VmaMemoryUsage to VkMemoryPropertyFlags
		constexpr VkMemoryPropertyFlags FLAG_MAP[5]{
			0,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};
		// Maps VkImageType to VkImageViewType
		constexpr VkImageViewType FORMAT_MAP[7] {
			VK_IMAGE_VIEW_TYPE_1D,
			VK_IMAGE_VIEW_TYPE_2D,
			VK_IMAGE_VIEW_TYPE_3D,
		};

		Image image{};
		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = memoryUsage;
		allocInfo.requiredFlags = FLAG_MAP[memoryUsage];
		CS_ASSERT(vmaCreateImage(this->allocator, &imageInfo, &allocInfo, &image.image, &image.allocation, nullptr) == VK_SUCCESS, "Failed to create image!");
		
		// Automatically create image view
		const VkImageViewCreateInfo viewInfo = Create::ImageViewCreateInfo(
			image.image, FORMAT_MAP[imageInfo.imageType], imageInfo.format, {},
			Create::ImageSubresourceRange((imageInfo.format == VK_FORMAT_D32_SFLOAT) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT, 0, imageInfo.mipLevels, 0, 1)
		);
		CS_ASSERT(vkCreateImageView(this->device, &viewInfo, nullptr, &image.imageView) == VK_SUCCESS, "Failed to create image view!");
	
		return image;
	}
	void Allocator::DestroyImage(Allocator::Image& image)
	{
		// Some images can have no image views
		if (image.imageView != nullptr) vkDestroyImageView(this->device, image.imageView, nullptr);
		// Some images can be from the swap chain, as such they have no allocation
		if (image.allocation != nullptr) vmaDestroyImage(this->allocator, image.image, image.allocation);
	}
	void Allocator::DestroyImages(std::vector<Image>& images)
	{
		for (Image& image : images) this->DestroyImage(image);
	}
}