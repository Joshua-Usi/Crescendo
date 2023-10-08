#pragma once

#include "Core/common.hpp"

#include "volk/volk.h"
#include "Create.hpp"

#include "Device/Device.hpp"
#include "QueueManager.hpp"

#include <functional>
#include <vector>

namespace Crescendo::internal
{
	class CommandQueue
	{
	private:
		Device device;
	public:
		VkCommandPool commandPool;
		VkCommandBuffer commandBuffer;
		VkFence completionFence;
		VkQueue queue;
		uint32_t queueFamily;

		CommandQueue() = default;
		inline CommandQueue(VkDevice device, QueueManager::Queue queue) : device(device), queue(queue.queue), queueFamily(queue.family), commandPool(nullptr), commandBuffer(nullptr), completionFence(nullptr) {}
		inline CommandQueue& Initialise(bool startSignalled)
		{
			this->commandPool = this->device.CreateCommandPool(this->queueFamily);
			this->commandBuffer = this->device.AllocateCommandBuffer(this->commandPool);
			this->completionFence = this->device.CreateFence(startSignalled);
			
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
			vkCmdBindVertexBuffers(this->commandBuffer, 0, static_cast<uint32_t>(buffers.size()), buffers.data(), offsets.data());
		}
		inline void BindIndexBuffer(VkBuffer indexBuffer, VkIndexType indexType = VK_INDEX_TYPE_UINT32) const
		{
			vkCmdBindIndexBuffer(this->commandBuffer, indexBuffer, 0, indexType);
		}
		inline void BindDescriptorSet(VkPipelineLayout layout, VkDescriptorSet set, uint32_t offset, uint32_t setIndex = 0, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const
		{
			vkCmdBindDescriptorSets(this->commandBuffer, bindPoint, layout, setIndex, 1, &set, (offset == 0) ? 0 : 1, &offset);
		}
		inline void BindDescriptorSets(VkPipelineLayout layout, const std::vector<VkDescriptorSet>& sets, const std::vector<uint32_t>& offsets, uint32_t firstSet = 0, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const
		{
			vkCmdBindDescriptorSets(this->commandBuffer, bindPoint, layout, firstSet, static_cast<uint32_t>(sets.size()), sets.data(), static_cast<uint32_t>(offsets.size()), offsets.data());
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
		inline void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkBufferCopy& region) const
		{
			vkCmdCopyBuffer(this->commandBuffer, srcBuffer, dstBuffer, 1, &region);
		}
		inline void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const std::vector<VkBufferCopy>& regions) const
		{
			vkCmdCopyBuffer(this->commandBuffer, srcBuffer, dstBuffer, static_cast<uint32_t>(regions.size()), regions.data());
		}
		inline void CopyBufferToImage(VkBuffer buffer, VkImage image, VkImageLayout dstImageLayout, const VkBufferImageCopy& region) const
		{
			vkCmdCopyBufferToImage(this->commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		}
		inline void BlitImage(VkImage srcImage, VkImage dstImage, const VkImageBlit& region, VkFilter filter = VK_FILTER_LINEAR) const
		{
			vkCmdBlitImage(this->commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, filter);
		}
		inline void BlitImage(VkImage srcImage, VkImage dstImage, const std::vector<VkImageBlit>& regions, VkFilter filter = VK_FILTER_LINEAR) const
		{
			vkCmdBlitImage(this->commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(regions.size()), regions.data(), filter);
		}
		inline void PipelineImageBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VkImageMemoryBarrier& barrier) const
		{
			vkCmdPipelineBarrier(this->commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
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
		inline void BeginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, const VkRect2D& area, const std::vector<VkClearValue>& clearValues) const
		{
			const VkRenderPassBeginInfo info = Create::RenderPassBeginInfo(
				renderPass, framebuffer, area, clearValues.size(), clearValues.data()
			);
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
			const std::vector<VkCommandBuffer> cmd = { this->commandBuffer };
			VkSubmitInfo submit = Create::SubmitInfo(waitSemaphores, waitStages, cmd, signalSemaphores);
			CS_ASSERT(vkQueueSubmit(this->queue, 1, &submit, this->completionFence) == VK_SUCCESS, "Failed to submit command buffer!");
		}
	public:
		/// <summary>
		/// Instantly submit a command buffer for GPU execution
		/// Execution on the thread is paused until the command buffer has finished executing
		/// Makes no gaurantees if the user has created an infinite loop
		/// </summary>
		/// <param name="function">Commands</param>
		/// <param name="timeout">Time to wait in nanoseconds until we continue presume timeout and continue execution, leave blank to basically gaurantee execution completion</param>
		inline void InstantSubmit(std::function<void(const CommandQueue& cmd)>&& function, uint64_t timeout = UINT64_MAX) const
		{
			this->Begin();
			function(*this);
			this->End();
			this->Submit();
			// No gaurantee that the user has not created an infinite loop
			this->WaitCompletion(timeout);
			this->ResetPool();
		}
	};
}