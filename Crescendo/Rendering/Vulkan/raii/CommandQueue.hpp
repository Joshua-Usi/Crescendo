#pragma once
#include "common.hpp"
#include "vulkan/vulkan.h"
#include <functional>

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class BaseCommandQueue
	{
	protected:
		VkDevice device;
		VkCommandPool commandPool;
		mutable VkCommandBuffer commandBuffer;
		VkFence completionFence;
		VkQueue queue;
		uint32_t queueFamily;
	public:
		BaseCommandQueue();
		BaseCommandQueue(VkDevice device, VkQueue queue, uint32_t family, bool startSignalled);
		~BaseCommandQueue();
		BaseCommandQueue(const BaseCommandQueue&) = delete;
		BaseCommandQueue& operator=(const BaseCommandQueue&) = delete;
		BaseCommandQueue(BaseCommandQueue&& other) noexcept;
		BaseCommandQueue& operator=(BaseCommandQueue&& other) noexcept;
	public:
		void Begin() const;
		void End() const;
		void ResourceBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VkImageMemoryBarrier& barrier) const;
		void ResetPool() const;
		void Reset() const;
		bool IsComplete() const;
		void WaitCompletion(uint64_t timeout_ns = UINT64_MAX) const;
		void Submit(const std::vector<VkSemaphore>& waitSemaphores = {}, const std::vector<VkPipelineStageFlags>& waitStages = {}, const std::vector<VkSemaphore>& signalSemaphores = {}) const;
		void Submit(VkSemaphore waitSemaphore, VkPipelineStageFlags waitStage, VkSemaphore signalSemaphore) const;
	public:
		operator VkCommandBuffer() const;
		VkQueue GetQueue() const;
	};

	class GraphicsCommandQueue : public BaseCommandQueue
	{
	public:
		using BaseCommandQueue::BaseCommandQueue;
		GraphicsCommandQueue(GraphicsCommandQueue&& other) noexcept : BaseCommandQueue(std::move(other)) {}
		GraphicsCommandQueue& operator=(GraphicsCommandQueue&& other) noexcept { BaseCommandQueue::operator=(std::move(other)); return *this; }
	public:
		void BeginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, const VkRect2D& renderArea, const std::vector<VkClearValue>& clearValues, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;
		void BeginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, const VkRect2D& renderArea, const VkClearValue& clearValue, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;
		void EndRenderPass() const;
		void DynamicStateSetViewport(const VkViewport& viewport) const;
		void DynamicStateSetScissor(const VkRect2D& scissor) const;
		void BindVertexBuffers(const std::vector<VkBuffer>& buffers, const std::vector<VkDeviceSize>& offsets) const;
		void BindIndexBuffer(VkBuffer indexBuffer, VkIndexType indexType = VK_INDEX_TYPE_UINT32) const;
		void BindDescriptorSet(VkPipelineLayout layout, VkDescriptorSet set, uint32_t setIndex, uint32_t offset, bool isDynamic = true) const;
		void BindDescriptorSets(VkPipelineLayout layout, const std::vector<VkDescriptorSet>& sets, uint32_t firstSet, const std::vector<uint32_t>& offsets) const;
		void BindPipeline(VkPipeline pipeline) const;
		template <typename T> void PushConstants(VkPipelineLayout layout, T& data, VkShaderStageFlags stageFlags, uint32_t offset = 0) const { PushConstants(layout, &data, sizeof(T), stageFlags, offset); }
		void PushConstants(VkPipelineLayout layout, const void* data, uint32_t size, VkShaderStageFlags stageFlags, uint32_t offset = 0) const;
		void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) const;
		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) const;
		// Intantly submit commands to a frame buffer, useful for one-off commands
		// However does not detect infinite loops, so use with caution
		// If an infinite loop is possible or likely, be sure to set a timeout
		void InstantSubmit(std::function<void(const GraphicsCommandQueue& cmd)>&& function, uint64_t timeout = UINT64_MAX) const;
	};

	class TransferCommandQueue : public BaseCommandQueue
	{
	public:
		using BaseCommandQueue::BaseCommandQueue;
		// Move
		TransferCommandQueue(TransferCommandQueue&& other) noexcept : BaseCommandQueue(std::move(other)) {}
		TransferCommandQueue& operator=(TransferCommandQueue&& other) noexcept { BaseCommandQueue::operator=(std::move(other)); return *this; }
	public:
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkBufferCopy& region) const;
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const std::vector<VkBufferCopy>& regions) const;
		void CopyBufferToImage(VkBuffer buffer, VkImage image, VkImageLayout dstImageLayout, const VkBufferImageCopy& region) const;
		void BlitImage(VkImage srcImage, VkImage dstImage, const VkImageBlit& region, VkFilter filter = VK_FILTER_LINEAR) const;
		void BlitImage(VkImage srcImage, VkImage dstImage, const std::vector<VkImageBlit>& regions, VkFilter filter = VK_FILTER_LINEAR) const;
		// Intantly submit commands to a frame buffer, useful for one-off commands
		// However does not detect infinite loops, so use with caution
		// If an infinite loop is possible or likely, be sure to set a timeout
		void InstantSubmit(std::function<void(const TransferCommandQueue& cmd)>&& function, uint64_t timeout = UINT64_MAX) const;
	};
}