#include "Buffer.hpp"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	Buffer::Buffer() : allocator(nullptr), allocation(nullptr), buffer(nullptr), size(0), pointer(nullptr) {}
	Buffer::Buffer(VmaAllocator allocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo) : allocator(allocator), size(createInfo.size)
	{
		CS_ASSERT(vmaCreateBuffer(this->allocator, &createInfo, &allocationInfo, &this->buffer, &this->allocation, nullptr) == VK_SUCCESS, "Failed to create buffer!");
		// Only map memeory if it is not on the GPU
		if (allocationInfo.usage != VMA_MEMORY_USAGE_GPU_ONLY)
			vmaMapMemory(this->allocator, allocation, &pointer);
		else
			pointer = nullptr;
	}
	Buffer::~Buffer()
	{
		if (this->allocator == nullptr)
			return;
		if (IsMapped())
			vmaUnmapMemory(this->allocator, allocation);
		vmaDestroyBuffer(this->allocator, this->buffer, this->allocation);
	}
	Buffer::Buffer(Buffer&& other) noexcept : allocator(other.allocator), allocation(other.allocation), buffer(other.buffer), size(other.size), pointer(other.pointer)
	{
		other.allocator = nullptr;
		other.allocation = nullptr;
		other.buffer = nullptr;
		other.size = 0;
		other.pointer = nullptr;
	}
	Buffer& Buffer::operator=(Buffer&& other) noexcept
	{
		if (this == &other)
			return *this;
		this->allocator = other.allocator; other.allocator = nullptr;
		this->allocation = other.allocation; other.allocation = nullptr;
		this->buffer = other.buffer; other.buffer = nullptr;
		this->size = other.size; other.size = 0;
		this->pointer = other.pointer; other.pointer = nullptr;
		return *this;
	}
	bool Buffer::IsMapped() const { return pointer != nullptr; }
	void Buffer::memcpy(const void* data, VkDeviceSize size, VkDeviceSize offset)
	{
		CS_ASSERT(offset + size <= this->size, "memcpy out of bounds!");
		if (IsMapped())
			std::memcpy(reinterpret_cast<uint8_t*>(pointer) + offset, data, size);
		else
			cs_std::console::fatal("Cannot memcpy to unmapped memory! Memory lives on GPU!");
	}
	Buffer::operator VkBuffer() const { return buffer; }
	VkDeviceSize Buffer::GetSize() const { return size; }
}