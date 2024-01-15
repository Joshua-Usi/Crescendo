#pragma once

#include "common.hpp"

#include "volk/volk.h"

#include "Types/Create.hpp"
#include "Types/Queues.hpp"

#include <functional>

CS_NAMESPACE_BEGIN::Vulkan
{
	class Device;

	class BaseCommandQueue
	{
	protected:
		VkDevice device;
		VkCommandPool commandPool;
		VkCommandBuffer commandBuffer;
		VkFence completionFence;
		VkQueue queue;
		uint32_t queueFamily;
	public:
		BaseCommandQueue() = default;
		BaseCommandQueue(Device& device, const Queues::Queue&, bool startSignalled);
		~BaseCommandQueue();
		// No copy
		BaseCommandQueue(const BaseCommandQueue&) = delete;
		BaseCommandQueue& operator=(const BaseCommandQueue&) = delete;
		// Move
		BaseCommandQueue(BaseCommandQueue&& other) noexcept;
		BaseCommandQueue& operator=(BaseCommandQueue&& other) noexcept;
		/// <summary>
		/// Begin recording commands
		/// </summary>
		void Begin() const;
		/// <summary>
		/// End recording commands
		/// </summary>
		void End() const;
		/// <summary>
		/// Insert a resource barrier into the command buffer
		/// </summary>
		/// <param name="srcStageMask"></param>
		/// <param name="dstStageMask"></param>
		/// <param name="barrier"></param>
		void ResourceBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VkImageMemoryBarrier& barrier) const;
		/// <summary>
		/// Reset the command pool
		/// </summary>
		void ResetPool() const;
		/// <summary>
		/// Reset the command buffer
		/// </summary>
		void Reset() const;
		/// <summary>
		/// Wait for the command buffer to finish execution before continuing
		/// </summary>
		/// <param name="timeout_ns">Time before timeout in nanoseconds</param>
		void WaitCompletion(uint64_t timeout_ns = UINT64_MAX) const;
		/// <summary>
		/// Submit a command buffer for GPU execution
		/// </summary>
		/// <param name="waitSemaphores">Semaphores to wait on</param>
		/// <param name="signalSemaphores">Semaphores to signal to</param>
		void Submit(const std::vector<VkSemaphore>& waitSemaphores = {}, const std::vector<VkPipelineStageFlags>& waitStages = {}, const std::vector<VkSemaphore>& signalSemaphores = {}) const;
		void Submit(VkSemaphore waitSemaphore, VkPipelineStageFlags waitStage, VkSemaphore signalSemaphore);
	public:
		VkDevice GetDevice() const { return device; }
		VkQueue GetQueue() const { return queue; }
		uint32_t GetQueueFamily() const { return queueFamily; }
	};

	class GraphicsCommandQueue : public BaseCommandQueue
	{
	public:
		using BaseCommandQueue::BaseCommandQueue;
		// Move
		GraphicsCommandQueue(GraphicsCommandQueue&& other) noexcept : BaseCommandQueue(std::move(other)) {}
		GraphicsCommandQueue& operator=(GraphicsCommandQueue&& other) noexcept { BaseCommandQueue::operator=(std::move(other)); return *this; }
		/// <summary>
		/// Begin the render pass
		/// </summary>
		/// <param name="renderPass"></param>
		/// <param name="framebuffer"></param>
		/// <param name="renderArea"></param>
		/// <param name="clearValues"></param>
		/// <param name="contents"></param>
		void BeginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, const VkRect2D& renderArea, const std::vector<VkClearValue>& clearValues, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;
		// End the render pass
		void EndRenderPass() const;
		/// <summary>
		/// Set the dynamic state for the viewport
		/// </summary>
		/// <param name="viewport"></param>
		void DynamicStateSetViewport(const VkViewport& viewport) const;
		/// <summary>
		/// Set the dynamic state for the scissor
		/// </summary>
		/// <param name="scissor"></param>
		void DynamicStateSetScissor(const VkRect2D& scissor) const;
		/// <summary>
		/// Bind a set of vertex buffers
		/// </summary>
		/// <param name="A set of buffers"></param>
		/// <param name="The buffer offsets"></param>
		void BindVertexBuffers(const std::vector<VkBuffer>& buffers, const std::vector<VkDeviceSize>& offsets) const;
		/// <summary>
		/// Bind an index buffer, defaults to 32 bit indices
		/// </summary>
		/// <param name="indexBuffer">Buffer</param>
		/// <param name="indexType">Type (default 32 bit)</param>
		void BindIndexBuffer(VkBuffer indexBuffer, VkIndexType indexType = VK_INDEX_TYPE_UINT32) const;
		/// <summary>
		/// Bind a descriptor set
		/// </summary>
		/// <param name="layout">The layout</param>
		/// <param name="set">The set</param>
		/// <param name="offset">Offset of the set in it's buffer</param>
		/// <param name="setIndex">The index of the set</param>
		/// <param name="bindPoint">Automatically presumes GRAPHICS</param>
		void BindDescriptorSet(VkPipelineLayout layout, VkDescriptorSet set, uint32_t setIndex, uint32_t offset, bool isDynamic = true, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const;
		/// <summary>
		/// Bind a bunch of descriptor sets at once
		/// </summary>
		/// <param name="layout">The set layout</param>
		/// <param name="sets">Sets to bind, in order</param>
		/// <param name="offsets">Offsets of each set</param>
		/// <param name="firstSet">The index of the first set</param>
		/// <param name="bindPoint">Automatically presumes GRAPHICS</param>
		void BindDescriptorSets(VkPipelineLayout layout, const std::vector<VkDescriptorSet>& sets, uint32_t firstSet, const std::vector<uint32_t>& offsets, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const;
		/// <summary>
		/// Bind a pipeline to the command buffer
		/// </summary>
		/// <param name="pipeline">Pipeline</param>
		/// <param name="bindPoint">Automatically presumes GRAPHICS</param>
		void BindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const;
		template <typename T> void PushConstants(VkPipelineLayout layout, T& data, VkShaderStageFlags stageFlags, uint32_t offset = 0) const
		{
			PushConstants(layout, &data, sizeof(T), stageFlags, offset);
		}
		/// <summary>
		/// Push a constant to the shader
		/// </summary>
		/// <param name="layout">Pipeline layout</param>
		/// <param name="data">Data to send</param>
		/// <param name="size">Size of the data</param>
		/// <param name="stageFlags">Which stages it should be exposed to</param>
		/// <param name="offset">Offset into the range, assumes 0</param>
		void PushConstants(VkPipelineLayout layout, const void* data, uint32_t size, VkShaderStageFlags stageFlags, uint32_t offset = 0) const;
		/// <summary>
		/// Draw vertices
		/// </summary>
		/// <param name="vertexCount"></param>
		/// <param name="instanceCount"></param>
		/// <param name="firstVertex"></param>
		/// <param name="firstInstance"></param>
		void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) const;
		/// <summary>
		/// Draw vertices using an index buffer
		/// </summary>
		/// <param name="indexCount">Number of indices to draw</param>
		/// <param name="instanceCount">Number of instances to draw, defaults to 1</param>
		/// <param name="firstIndex">Offset of the first index in the bound index buffer</param>
		/// <param name="vertexOffset">Offset of the first vertex in the bound buffers</param>
		/// <param name="firstInstance"starting InstanceID of the first instance drawn></param>
		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) const;
		/// <summary>
		/// Present the swapchain image to the screen, Only graphics queues can present
		/// Isn't really a command-per-se, but makes sense to be here
		/// </summary>
		/// <param name="swapchain">Swapchain to get images from</param>
		/// <param name="imageIndex">index of the image to present</param>
		/// <param name="waitSemaphore">Semaphore to wait on</param>
		VkResult  Present(VkSwapchainKHR swapchain, uint32_t imageIndex, VkSemaphore renderFinishSemaphore) const;
		/// <summary>
		/// Instantly submit a command buffer for GPU execution
		/// Execution on the thread is paused until the command buffer has finished executing
		/// Makes no gaurantees if the user has created an infinite loop
		/// </summary>
		/// <param name="function">Commands</param>
		/// <param name="timeout">Time to wait in nanoseconds until we continue execution, leave blank to basically gaurantee execution completion, If the command queue is not completed in the timeout, then it continues</param>
		void InstantSubmit(std::function<void(const GraphicsCommandQueue& cmd)>&& function, uint64_t timeout = UINT64_MAX) const;
	};

	class TransferCommandQueue : public BaseCommandQueue
	{
	public:
		using BaseCommandQueue::BaseCommandQueue;
		// Move
		TransferCommandQueue(TransferCommandQueue&& other) noexcept : BaseCommandQueue(std::move(other)) {}
		TransferCommandQueue& operator=(TransferCommandQueue&& other) noexcept { BaseCommandQueue::operator=(std::move(other)); return *this; }
		/// <summary>
		/// Copies a region of a buffer to another buffer
		/// </summary>
		/// <param name="srcBuffer"></param>
		/// <param name="dstBuffer"></param>
		/// <param name="region"></param>
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkBufferCopy& region) const;
		/// <summary>
		/// Copies a vector of regions of a buffer to another buffer
		/// </summary>
		/// <param name="srcBuffer"></param>
		/// <param name="dstBuffer"></param>
		/// <param name="regions"></param>
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const std::vector<VkBufferCopy>& regions) const;
		/// <summary>
		/// Copies a buffer of data to an image
		/// </summary>
		/// <param name="buffer"></param>
		/// <param name="image"></param>
		/// <param name="dstImageLayout"></param>
		/// <param name="region"></param>
		void CopyBufferToImage(VkBuffer buffer, VkImage image, VkImageLayout dstImageLayout, const VkBufferImageCopy& region) const;
		/// <summary>
		/// Blit an image region
		/// </summary>
		/// <param name="srcImage"></param>
		/// <param name="dstImage"></param>
		/// <param name="region"></param>
		/// <param name="filter">Defaults to linear for mipmaps</param>
		void BlitImage(VkImage srcImage, VkImage dstImage, const VkImageBlit& region, VkFilter filter = VK_FILTER_LINEAR) const;
		/// <summary>
		/// Blit several image regions
		/// </summary>
		/// <param name="srcImage"></param>
		/// <param name="dstImage"></param>
		/// <param name="regions"></param>
		/// <param name="filter">Defaults to linear for mipmaps</param>
		void BlitImage(VkImage srcImage, VkImage dstImage, const std::vector<VkImageBlit>& regions, VkFilter filter = VK_FILTER_LINEAR) const;
		/// <summary>
		/// Instantly submit a command buffer for GPU execution
		/// Execution on the thread is paused until the command buffer has finished executing
		/// Makes no gaurantees if the user has created an infinite loop
		/// </summary>
		/// <param name="function">Commands</param>
		/// <param name="timeout">Time to wait in nanoseconds until we continue execution, leave blank to basically gaurantee execution completion, If the command queue is not completed in the timeout, then it continues</param>
		void InstantSubmit(std::function<void(const TransferCommandQueue& cmd)>&& function, uint64_t timeout = UINT64_MAX) const;
	};
}