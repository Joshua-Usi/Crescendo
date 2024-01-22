#include "CommandQueue.hpp"

#include "Device.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	BaseCommandQueue::BaseCommandQueue(Device& device, const Queues::Queue& queue, bool startSignalled)
		: device(device), queue(queue.queue), queueFamily(queue.family),
		commandPool(device.CreateCommandPool(queue.family)),
		commandBuffer(device.AllocateCommandBuffer(this->commandPool)),
		completionFence(device.CreateFence(startSignalled)) {}
	BaseCommandQueue::~BaseCommandQueue()
	{
		if (!this->device) return;
		vkDestroyCommandPool(this->device, this->commandPool, nullptr);
		vkDestroyFence(this->device, this->completionFence, nullptr);
	}
	BaseCommandQueue::BaseCommandQueue(BaseCommandQueue&& other) noexcept
		: device(other.device), commandPool(other.commandPool), commandBuffer(other.commandBuffer),
		completionFence(other.completionFence), queue(other.queue), queueFamily(other.queueFamily)
	{
		// Do not destroy the device
		other.commandPool = nullptr;
		other.commandBuffer = nullptr;
		other.completionFence = nullptr;
	}
	BaseCommandQueue& BaseCommandQueue::operator=(BaseCommandQueue&& other) noexcept
	{
		this->device = other.device;
		this->commandPool = other.commandPool;
		this->commandBuffer = other.commandBuffer;
		this->completionFence = other.completionFence;
		this->queue = other.queue;
		this->queueFamily = other.queueFamily;

		// Do not destroy the device
		other.commandPool = nullptr;
		other.commandBuffer = nullptr;
		other.completionFence = nullptr;

		return *this;
	}
	void BaseCommandQueue::Begin() const
	{
		const VkCommandBufferBeginInfo beginInfo = Create::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		CS_ASSERT(vkBeginCommandBuffer(this->commandBuffer, &beginInfo) == VK_SUCCESS, "Failed to begin command buffer!");
	}
	void BaseCommandQueue::End() const
	{
		CS_ASSERT(vkEndCommandBuffer(this->commandBuffer) == VK_SUCCESS, "Failed to end command buffer!");
	}
	void BaseCommandQueue::ResourceBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VkImageMemoryBarrier& barrier) const
	{
		vkCmdPipelineBarrier(this->commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}
	void BaseCommandQueue::ResetPool() const
	{
		CS_ASSERT(vkResetCommandPool(this->device, this->commandPool, 0) == VK_SUCCESS, "Failed to reset command pool!");
	}
	void BaseCommandQueue::Reset() const
	{
		CS_ASSERT(vkResetCommandBuffer(this->commandBuffer, 0) == VK_SUCCESS, "Failed to reset command buffer!");
	}
	void BaseCommandQueue::WaitCompletion(uint64_t timeout_ns) const
	{
		vkWaitForFences(this->device, 1, &this->completionFence, VK_TRUE, timeout_ns);
		vkResetFences(this->device, 1, &this->completionFence);
	}
	void BaseCommandQueue::Submit(const std::vector<VkSemaphore>& waitSemaphores, const std::vector<VkPipelineStageFlags>& waitStages, const std::vector<VkSemaphore>& signalSemaphores) const
	{
		VkSubmitInfo submit = Create::SubmitInfo(waitSemaphores, waitStages, &this->commandBuffer, signalSemaphores);
		CS_ASSERT(vkQueueSubmit(this->queue, 1, &submit, this->completionFence) == VK_SUCCESS, "Failed to submit command buffer!");
	}
	void BaseCommandQueue::Submit(VkSemaphore waitSemaphore, VkPipelineStageFlags waitStage, VkSemaphore signalSemaphore)
	{
		VkSubmitInfo submit = Create::SubmitInfo(&waitSemaphore, &waitStage, &this->commandBuffer, &signalSemaphore);
		CS_ASSERT(vkQueueSubmit(this->queue, 1, &submit, this->completionFence) == VK_SUCCESS, "Failed to submit command buffer!");
	}
	//////////////////////////////////////////////////////////////// GraphicsCommandQueue commands ////////////////////////////////////////////////////////////////
	void GraphicsCommandQueue::BeginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, const VkRect2D& renderArea, const std::vector<VkClearValue>& clearValues, VkSubpassContents contents) const
	{
		const VkRenderPassBeginInfo info = Create::RenderPassBeginInfo(
			renderPass, framebuffer, renderArea, clearValues.size(), clearValues.data()
		);
		vkCmdBeginRenderPass(this->commandBuffer, &info, contents);
	}
	void GraphicsCommandQueue::EndRenderPass() const
	{
		vkCmdEndRenderPass(this->commandBuffer);
	}
	void GraphicsCommandQueue::DynamicStateSetViewport(const VkViewport& viewport) const
	{
		vkCmdSetViewport(this->commandBuffer, 0, 1, &viewport);
	}
	void GraphicsCommandQueue::DynamicStateSetScissor(const VkRect2D& scissor) const
	{
		vkCmdSetScissor(this->commandBuffer, 0, 1, &scissor);
	}
	void GraphicsCommandQueue::BindVertexBuffers(const std::vector<VkBuffer>& buffers, const std::vector<VkDeviceSize>& offsets) const
	{
		vkCmdBindVertexBuffers(this->commandBuffer, 0, static_cast<uint32_t>(buffers.size()), buffers.data(), offsets.data());
	}
	void GraphicsCommandQueue::BindIndexBuffer(VkBuffer indexBuffer, VkIndexType indexType) const
	{
		vkCmdBindIndexBuffer(this->commandBuffer, indexBuffer, 0, indexType);
	}
	void GraphicsCommandQueue::BindDescriptorSet(VkPipelineLayout layout, VkDescriptorSet set, uint32_t setIndex, uint32_t offset, bool isDynamic, VkPipelineBindPoint bindPoint) const
	{
		vkCmdBindDescriptorSets(this->commandBuffer, bindPoint, layout, setIndex, 1, &set, static_cast<uint32_t>(isDynamic), &offset);
	}
	void GraphicsCommandQueue::BindDescriptorSets(VkPipelineLayout layout, const std::vector<VkDescriptorSet>& sets, uint32_t firstSet, const std::vector<uint32_t>& offsets, VkPipelineBindPoint bindPoint) const
	{
		vkCmdBindDescriptorSets(this->commandBuffer, bindPoint, layout, firstSet, static_cast<uint32_t>(sets.size()), sets.data(), static_cast<uint32_t>(offsets.size()), offsets.data());
	}
	void GraphicsCommandQueue::BindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint) const
	{
		vkCmdBindPipeline(this->commandBuffer, bindPoint, pipeline);
	}
	void GraphicsCommandQueue::PushConstants(VkPipelineLayout layout, const void* data, uint32_t size, VkShaderStageFlags stageFlags, uint32_t offset) const
	{
		vkCmdPushConstants(this->commandBuffer, layout, stageFlags, offset, size, data);
	}
	void GraphicsCommandQueue::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const
	{
		vkCmdDraw(this->commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}
	void GraphicsCommandQueue::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const
	{
		vkCmdDrawIndexed(this->commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}
	VkResult GraphicsCommandQueue::Present(VkSwapchainKHR swapchain, uint32_t imageIndex, VkSemaphore renderFinishSemaphore) const
	{
		const VkPresentInfoKHR presentInfo = Create::PresentInfoKHR(&renderFinishSemaphore, &swapchain, &imageIndex);
		return vkQueuePresentKHR(this->GetQueue(), &presentInfo);
	}
	void GraphicsCommandQueue::InstantSubmit(std::function<void(const GraphicsCommandQueue& cmd)>&& function, uint64_t timeout) const
	{
		this->Begin();
		function(*this);
		this->End();
		this->Submit();
		// No gaurantee that the user has not created an infinite loop
		this->WaitCompletion(timeout);
		this->ResetPool();
	}
	//////////////////////////////////////////////////////////////// TransferCommandQueue commands ////////////////////////////////////////////////////////////////
	void TransferCommandQueue::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkBufferCopy& region) const
	{
		vkCmdCopyBuffer(this->commandBuffer, srcBuffer, dstBuffer, 1, &region);
	}
	void TransferCommandQueue::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const std::vector<VkBufferCopy>& regions) const
	{
		vkCmdCopyBuffer(this->commandBuffer, srcBuffer, dstBuffer, static_cast<uint32_t>(regions.size()), regions.data());
	}
	void TransferCommandQueue::CopyBufferToImage(VkBuffer buffer, VkImage image, VkImageLayout dstImageLayout, const VkBufferImageCopy& region) const
	{
		vkCmdCopyBufferToImage(this->commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}
	void TransferCommandQueue::BlitImage(VkImage srcImage, VkImage dstImage, const VkImageBlit& region, VkFilter filter) const
	{
		vkCmdBlitImage(this->commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, filter);
	}
	void TransferCommandQueue::BlitImage(VkImage srcImage, VkImage dstImage, const std::vector<VkImageBlit>& regions, VkFilter filter) const
	{
		vkCmdBlitImage(this->commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(regions.size()), regions.data(), filter);
	}
	void TransferCommandQueue::InstantSubmit(std::function<void(const TransferCommandQueue& cmd)>&& function, uint64_t timeout) const
	{
		this->Begin();
		function(*this);
		this->End();
		this->Submit();
		// No gaurantee that the user has not created an infinite loop
		this->WaitCompletion(timeout);
		this->ResetPool();
	}
}