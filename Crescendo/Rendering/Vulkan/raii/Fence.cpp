#include "Fence.hpp"
#include "Volk/volk.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	Fence::Fence() : device(nullptr), fence(nullptr) {}
	Fence::Fence(VkDevice device, bool startSignalled) : device(device)
	{
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = startSignalled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
		CS_ASSERT(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence) == VK_SUCCESS, "Failed to create fence!");
	}
	Fence::~Fence()
	{
		if (this->device != nullptr)
			vkDestroyFence(device, fence, nullptr);
	}
	Fence::Fence(Fence&& other) noexcept : device(other.device), fence(other.fence)
	{
		other.device = nullptr;
		other.fence = nullptr;
	}
	Fence& Fence::operator=(Fence&& other) noexcept
	{
		if (this == &other)
			return *this;
		this->device = other.device;
		this->fence = other.fence;
		other.device = nullptr;
		other.fence = nullptr;
		return *this;
	}
	Fence::operator VkFence() const { return fence; }
	VkFence Fence::GetFence() const { return fence; }
	void Fence::Wait(uint64_t timeout) const { CS_ASSERT(vkWaitForFences(device, 1, &fence, VK_TRUE, timeout) == VK_SUCCESS, "Failed to wait for fence!"); }
	void Fence::Reset() const { CS_ASSERT(vkResetFences(device, 1, &fence) == VK_SUCCESS, "Failed to reset fence!"); }
}