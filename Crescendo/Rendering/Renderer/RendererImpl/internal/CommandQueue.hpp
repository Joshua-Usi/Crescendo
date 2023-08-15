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
	public:
		VkCommandPool commandPool;
		VkCommandBuffer commandBuffer;
		VkFence completionFence;
		VkQueue queue;
		uint32_t queueFamily;

		CommandQueue() = default;
		inline CommandQueue(VkDevice device, VkQueue queue, uint32_t queueFamily) : device(device), queue(queue), queueFamily(queueFamily), commandPool(nullptr), commandBuffer(nullptr), completionFence(nullptr) {}
		inline CommandQueue& Initialise(bool startSignalled)
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
			
			return *this;
		}
		inline void Destroy()
		{
			vkDestroyCommandPool(this->device, this->commandPool, nullptr);
			vkDestroyFence(this->device, this->completionFence, nullptr);
		}
	public:
		inline void SetViewport(const VkViewport& viewport) const
		{
			vkCmdSetViewport(this->commandBuffer, 0, 1, &viewport);
		}
		inline void SetScissor(const VkRect2D& scissor) const
		{
			vkCmdSetScissor(this->commandBuffer, 0, 1, &scissor);
		}
		inline void BindVertexBuffers(const std::vector<VkBuffer>& buffers, const std::vector<VkDeviceSize>& offsets) const
		{
			vkCmdBindVertexBuffers(this->commandBuffer, 0, buffers.size(), buffers.data(), offsets.data());
		}
		inline void BindIndexBuffer(VkBuffer indexBuffer, VkIndexType indexType = VK_INDEX_TYPE_UINT32) const
		{
			vkCmdBindIndexBuffer(this->commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		}
		inline void BindDescriptorSets(VkPipelineLayout layout, const std::vector<VkDescriptorSet>& sets, const std::vector<uint32_t>& offsets, uint32_t firstSet = 0, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const
		{
			vkCmdBindDescriptorSets(this->commandBuffer, bindPoint, layout, firstSet, sets.size(), sets.data(), offsets.size(), offsets.data());
		}
		inline void BindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const
		{
			vkCmdBindPipeline(this->commandBuffer, bindPoint, pipeline);
		}
		inline void PushConstants(VkPipelineLayout layout, const void* data, uint32_t size, VkShaderStageFlags stageFlags, uint32_t offset = 0) const
		{
			vkCmdPushConstants(this->commandBuffer, layout, stageFlags, offset, size, data);
		}
		inline void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const
		{
			vkCmdDrawIndexed(this->commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
		}
	public:
		/// <summary>
		/// Begin recording the command buffer
		/// </summary>
		inline void Begin() const
		{
			const VkCommandBufferBeginInfo beginInfo = Create::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			CS_ASSERT(vkBeginCommandBuffer(this->commandBuffer, &beginInfo) == VK_SUCCESS, "Failed to begin command buffer!");
		}
		/// <summary>
		/// End recording the command buffer
		/// </summary>
		inline void End() const
		{
			CS_ASSERT(vkEndCommandBuffer(this->commandBuffer) == VK_SUCCESS, "Failed to end command buffer!");
		}
		/// <summary>
		/// Begin the renderpass
		/// </summary>
		inline void BeginRenderPass(const VkRenderPassBeginInfo& info) const
		{
			vkCmdBeginRenderPass(this->commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
		}
		/// <summary>
		/// End the renderpass
		/// </summary>
		inline void EndRenderPass() const
		{
			vkCmdEndRenderPass(this->commandBuffer);
		}
		/// <summary>
		/// Reset the command pool
		/// </summary>
		inline void ResetPool() const
		{
			CS_ASSERT(vkResetCommandPool(this->device, this->commandPool, 0) == VK_SUCCESS, "Failed to reset command pool!");
		}
		/// <summary>
		/// Reset the command buffer
		/// </summary>
		inline void Reset() const
		{
			CS_ASSERT(vkResetCommandBuffer(this->commandBuffer, 0) == VK_SUCCESS, "Failed to reset command buffer!");
		}
		/// <summary>
		/// Wait for the command buffer to finish execution before continuing
		/// </summary>
		/// <param name="timeout_ns">Time before timeout in nanoseconds</param>
		inline void WaitCompletion(uint64_t timeout_ns) const
		{
			vkWaitForFences(this->device, 1, &this->completionFence, VK_TRUE, timeout_ns);
			vkResetFences(this->device, 1, &this->completionFence);
		}
		/// <summary>
		/// Submit a command buffer for GPU execution
		/// </summary>
		/// <param name="waitSemaphores">Semaphores to wait on</param>
		/// <param name="signalSemaphores">Semaphores to signal to</param>
		inline void Submit(const std::vector<VkSemaphore>& waitSemaphores = {}, const std::vector<VkPipelineStageFlags>& waitStages = {}, const std::vector<VkSemaphore>& signalSemaphores = {}) const
		{
			VkSubmitInfo submit = Create::SubmitInfo(
				waitSemaphores.size(), waitSemaphores.data(), waitStages.data(),
				1, &this->commandBuffer,
				signalSemaphores.size(), signalSemaphores.data()
			);
			CS_ASSERT(vkQueueSubmit(this->queue, 1, &submit, this->completionFence) == VK_SUCCESS, "Failed to submit one time submit!");
		}
	};
}