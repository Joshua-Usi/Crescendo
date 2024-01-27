#include "Allocator.hpp"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	Allocator::Allocator() : device(nullptr), allocator(nullptr) {}
	Allocator::Allocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device) : device(device)
	{
		VmaVulkanFunctions vma_vulkan_func{};
		vma_vulkan_func.vkAllocateMemory = vkAllocateMemory;
		vma_vulkan_func.vkBindBufferMemory = vkBindBufferMemory;
		vma_vulkan_func.vkBindImageMemory = vkBindImageMemory;
		vma_vulkan_func.vkCreateBuffer = vkCreateBuffer;
		vma_vulkan_func.vkCreateImage = vkCreateImage;
		vma_vulkan_func.vkDestroyBuffer = vkDestroyBuffer;
		vma_vulkan_func.vkDestroyImage = vkDestroyImage;
		vma_vulkan_func.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
		vma_vulkan_func.vkFreeMemory = vkFreeMemory;
		vma_vulkan_func.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
		vma_vulkan_func.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
		vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
		vma_vulkan_func.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
		vma_vulkan_func.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
		vma_vulkan_func.vkMapMemory = vkMapMemory;
		vma_vulkan_func.vkUnmapMemory = vkUnmapMemory;
		vma_vulkan_func.vkCmdCopyBuffer = vkCmdCopyBuffer;
		vma_vulkan_func.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		vma_vulkan_func.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;
		allocatorInfo.physicalDevice = physicalDevice;
		allocatorInfo.pVulkanFunctions = &vma_vulkan_func;

		vmaCreateAllocator(&allocatorInfo, &this->allocator);
	}
	Allocator::~Allocator()
	{
		if (this->device != nullptr) vmaDestroyAllocator(this->allocator);
	}
	Allocator::Allocator(Allocator&& other) noexcept : device(other.device), allocator(other.allocator)
	{
		other.device = nullptr;
		other.allocator = nullptr;
	}
	Allocator& Allocator::operator=(Allocator&& other) noexcept
	{
		if (this == &other) return *this;
		this->device = other.device; other.device = nullptr;
		this->allocator = other.allocator; other.allocator = nullptr;
		return *this;
	}
	Allocator::operator VmaAllocator() const { return allocator; }
	VmaAllocator Allocator::GetAllocator() const { return allocator; }
	VkDevice Allocator::GetDevice() const { return device; }
}