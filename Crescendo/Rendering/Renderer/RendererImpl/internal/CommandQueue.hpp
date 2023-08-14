#pragma once

#include "Core/common.hpp"

#include "vulkan/vulkan.h"
#include "Create.hpp"

namespace Crescendo::internal
{
	class CommandQueue
	{
	private:
		VkDevice device;
		uint32_t queueFamily;
	public:
		VkCommandPool commandPool;
		VkCommandBuffer commandBuffer;
		VkFence completionFence;

		CommandQueue() = default;
		inline CommandQueue(VkDevice device, uint32_t queueFamily) : device(device), queueFamily(queueFamily), commandPool(nullptr), commandBuffer(nullptr), completionFence(nullptr) {}
		inline void Initialise(bool startSignalled)
		{
			// Create pool
			const VkCommandPoolCreateInfo poolInfo = Create::CommandPoolCreateInfo(this->queueFamily);
			CS_ASSERT(vkCreateCommandPool(this->device, &poolInfo, nullptr, &this->commandPool) == VK_SUCCESS, "Failed to create command pool!");

			// Create buffer
			const VkCommandBufferAllocateInfo cmdBufferInfo = Create::CommandBufferAllocateInfo(this->commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
			CS_ASSERT(vkAllocateCommandBuffers(this->device, &cmdBufferInfo, &this->commandBuffer) == VK_SUCCESS, "Failed to allocate command buffers!");

			// Create fence that denotes when the command buffer has finished executing
			const VkFenceCreateInfo fenceInfo = Create::FenceCreateInfo(startSignalled);
			CS_ASSERT(vkCreateFence(this->device, &fenceInfo, nullptr, &this->completionFence) == VK_SUCCESS, "Failed to create in flight fences");
		}
		inline void Destroy()
		{
			vkDestroyCommandPool(this->device, this->commandPool, nullptr);
			vkDestroyFence(this->device, this->completionFence, nullptr);
		}
	};
}