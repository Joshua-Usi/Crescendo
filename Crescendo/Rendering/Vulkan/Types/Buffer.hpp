#pragma once

#include "Volk/volk.h"
#include "vma/vk_mem_alloc.h"

#include <cstring>
#include <utility>

namespace Crescendo::Vulkan
{
	struct Buffer
	{
	private:
		VmaAllocator allocator;
		VmaAllocation allocation;
	public:
		VkBuffer buffer;
		void* mPtr;
	public:
		// Constructors
		Buffer() = default;
		Buffer(VkBuffer buffer, VmaAllocator allocator, VmaAllocation allocation, void* mPtr) :
			buffer(buffer), allocator(allocator), allocation(allocation), mPtr(mPtr) {}
		// Destructors
		~Buffer()
		{
			if (this->allocator == nullptr) return;
			if (this->mPtr != nullptr) vmaUnmapMemory(this->allocator, this->allocation);
			vmaDestroyBuffer(this->allocator, this->buffer, this->allocation);
		}
		// No copy
		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;
		// Move
		Buffer(Buffer&& other) noexcept :
			buffer(other.buffer), allocator(other.allocator), allocation(other.allocation), mPtr(other.mPtr)
		{
			other.buffer = nullptr;
			other.allocator = nullptr;
			other.allocation = nullptr;
			other.mPtr = nullptr;
		}
		Buffer& operator=(Buffer&& other) noexcept
		{
			if (this != &other)
			{
				buffer = other.buffer; other.buffer = nullptr;
				allocator = other.allocator;	other.allocator = nullptr;
				allocation = other.allocation; other.allocation = nullptr;
				mPtr = other.mPtr; other.mPtr = nullptr;
			}
			return *this;
		}
	public:
		Buffer&& Fill(size_t offset, const void* data, size_t size)
		{
			memcpy(static_cast<char*>(this->mPtr) + offset, data, size);
			return std::move(*this);
		}
	public:
		operator VkBuffer() const { return buffer; }
	};
}