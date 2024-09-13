#pragma once
#include "common.hpp"
#include "vulkan/vulkan.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class Semaphore
	{
	private:
		VkDevice device;
		VkSemaphore semaphore;
	public:
		Semaphore();
		// Creates a semaphore from createInfo
		Semaphore(VkDevice device, const VkSemaphoreCreateInfo& createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0 });
		// Takes ownership of semaphore
		explicit Semaphore(VkDevice device, VkSemaphore semaphore);
		~Semaphore();
		Semaphore(const Semaphore&) = delete;
		Semaphore& operator=(const Semaphore&) = delete;
		Semaphore(Semaphore&& other) noexcept;
		Semaphore& operator=(Semaphore&& other) noexcept;
		operator VkSemaphore() const;
	};
}