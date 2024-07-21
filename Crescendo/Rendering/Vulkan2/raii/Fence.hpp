#pragma once

#include "common.hpp"
#include "vulkan/vulkan.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class Fence
	{
	private:
		VkDevice device;
		VkFence fence;
	public:
		Fence();
		Fence(VkDevice device, bool startSignalled);
		~Fence();
		Fence(const Fence&) = delete;
		Fence& operator=(const Fence&) = delete;
		Fence(Fence&& other) noexcept;
		Fence& operator=(Fence&& other) noexcept;
		operator VkFence() const;
		VkFence GetFence() const;
		void Wait(uint64_t timeout = UINT64_MAX) const;
		void Reset() const;
	};
}