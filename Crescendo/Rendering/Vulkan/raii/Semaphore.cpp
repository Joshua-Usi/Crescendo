#include "Semaphore.hpp"
#include "Volk/volk.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	Semaphore::Semaphore() : device(nullptr), semaphore(nullptr) {}
	Semaphore::Semaphore(VkDevice device, const VkSemaphoreCreateInfo& createInfo) : device(device)
	{
		CS_ASSERT(vkCreateSemaphore(this->device, &createInfo, nullptr, &this->semaphore) == VK_SUCCESS, "Failed to create semaphore!");
	}
	Semaphore::Semaphore(VkDevice device, VkSemaphore semaphore) : device(device), semaphore(semaphore) {}
	Semaphore::~Semaphore()
	{
		if (this->device != nullptr)
			vkDestroySemaphore(this->device, this->semaphore, nullptr);
	}
	Semaphore::Semaphore(Semaphore&& other) noexcept : device(other.device), semaphore(other.semaphore)
	{
		other.device = nullptr;
		other.semaphore = nullptr;
	}
	Semaphore& Semaphore::operator=(Semaphore&& other) noexcept
	{
		if (this == &other)
			return *this;
		this->device = other.device; other.device = nullptr;
		this->semaphore = other.semaphore; other.semaphore = nullptr;
		return *this;
	}
	Semaphore::operator VkSemaphore() const { return this->semaphore; }
}