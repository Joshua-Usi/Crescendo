#pragma once

#include "volk/volk.h"
#include "vma/vk_mem_alloc.h"

#include <vector>

namespace Crescendo::internal
{
	class Allocator
	{
	private:
		VmaAllocator allocator;
		VkDevice device;
	public:
		struct Buffer
		{
			VkBuffer buffer;
			VmaAllocation allocation;
			void* memoryLocation;

			Buffer() = default;

			operator VkBuffer() const { return buffer; }

			// Offset in bytes, only valid for host visible mapped memory
			inline Buffer& Fill(size_t offset, const void* data, size_t size)
			{
				memcpy(static_cast<char*>(this->memoryLocation) + offset, data, size);
				return *this;
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
		inline Allocator(VkDevice device) : allocator(nullptr), device(device) {}
		~Allocator() = default;
		/// <summary>
		/// Initialise the allocator
		/// </summary>
		/// <returns>Reference to allocator if you want one line creation and initialisation</returns>
		Allocator& Initialise(VkInstance instance, VkPhysicalDevice physicalDevice);
		/// <summary>
		/// Destroy the allocator, should be called before destroying the device
		/// </summary>
		void Destroy();
	public:
		Buffer CreateBuffer(size_t allocationSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
		void DestroyBuffer(Buffer& buffer);
		void DestroyBuffers(std::vector<Buffer>& buffers);
		Image CreateImage(const VkImageCreateInfo& imageInfo, VmaMemoryUsage memoryUsage);
		void DestroyImage(Allocator::Image& image);
		void DestroyImages(std::vector<Image>& images);
	};
};