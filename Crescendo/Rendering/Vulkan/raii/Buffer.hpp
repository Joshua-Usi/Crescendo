#pragma once
#include "common.hpp"
#include "vulkan/vulkan.h"
#include "vma/vk_mem_alloc.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class Buffer
	{
	private:
		VmaAllocator allocator;
		VmaAllocation allocation;
		VkBuffer buffer;
		VkDeviceSize size;
		void* pointer;
	public:
		Buffer();
		Buffer(VmaAllocator allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo);
		~Buffer();
		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;
		Buffer(Buffer&& other) noexcept;
		Buffer& operator=(Buffer&& other) noexcept;
		bool IsMapped() const;
		void memcpy(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
		operator VkBuffer() const;
		VkBuffer GetBuffer() const;
		VkDeviceSize GetSize() const;
		template <typename T>
		size_t GetElementCount() const { return static_cast<uint32_t>(size / sizeof(T)); }
	};
};