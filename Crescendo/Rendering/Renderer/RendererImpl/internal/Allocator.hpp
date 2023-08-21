#pragma once

#include "vulkan/vulkan.h"
#include "VMA/vk_mem_alloc.h"

namespace Crescendo::internal
{
	class Allocator
	{
	private:
		VmaAllocator allocator;
		VkInstance instance;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
	public:
		struct Buffer
		{
			VkBuffer buffer;
			VmaAllocation allocation;
			void* memoryLocation;

			inline Buffer() : buffer(nullptr), allocation(nullptr), memoryLocation(nullptr) {}

			// Offset in bytes
			inline void Fill(size_t offset, const void* data, size_t size)
			{
				memcpy(static_cast<char*>(this->memoryLocation) + offset, data, size);
			}
		};

		struct Image
		{
			VkImage image;
			VkImageView imageView;
			VmaAllocation allocation;

			inline Image() : image(nullptr), imageView(nullptr), allocation(nullptr) {}
			// vkb can allocate it's own image without vma, so we need a way to allocator without vma
			inline Image(VkImage image, VkImageView view) : image(image), imageView(view), allocation(nullptr) {}
		};
	public:
		Allocator() = default;
		inline Allocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device) : allocator(nullptr), instance(instance), physicalDevice(physicalDevice), device(device) {}
		~Allocator() = default;
		/// <summary>
		/// Initialise the allocator
		/// </summary>
		/// <returns>Reference to allocator if you want one line creation and initialisation</returns>
		Allocator& Initialise();
		/// <summary>
		/// Destroy the allocator, should be called before destroying the device
		/// </summary>
		void Destroy();
	public:
		Buffer CreateBuffer(size_t allocationSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
		void DestroyBuffer(Buffer& buffer);
		Image CreateImage(const VkImageCreateInfo& imageInfo, VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags requiredFlags = 0);
		// Create image with image view
		Image CreateImage(const VkImageCreateInfo& imageInfo, VkImageViewCreateInfo& viewInfo, VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags requiredFlags = 0);
		void DestroyImage(Allocator::Image& image);
	};
};