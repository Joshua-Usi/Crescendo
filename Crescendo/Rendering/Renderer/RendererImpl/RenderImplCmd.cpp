#include "RendererImpl.hpp"

namespace Crescendo
{
	void Renderer::RendererImpl::BeginFrame(const VkClearValue& clearColor)
	{
		// One second in nanoseconds
		constexpr uint64_t ONE_SECOND = 1000000000;

		FrameData& currentFrame = this->GetCurrentFrameData();
		VkCommandBuffer cmd = currentFrame.commandQueue.commandBuffer;

		// Wait for previous frame to finish
		vkWaitForFences(this->device, 1, &currentFrame.commandQueue.completionFence, true, ONE_SECOND);
		vkResetFences(this->device, 1, &currentFrame.commandQueue.completionFence);

		// Acquire image from swapchain
		VkResult imageAcquireResult = vkAcquireNextImageKHR(this->device, this->swapchain.swapchain, ONE_SECOND, currentFrame.presentSemaphore, VK_NULL_HANDLE, &this->state.swapchainImageIndex);
		if (imageAcquireResult == VK_ERROR_OUT_OF_DATE_KHR || imageAcquireResult == VK_SUBOPTIMAL_KHR)
		{
			this->RecreateSwapchain();
			imageAcquireResult = vkAcquireNextImageKHR(this->device, this->swapchain.swapchain, ONE_SECOND, currentFrame.presentSemaphore, VK_NULL_HANDLE, &this->state.swapchainImageIndex);
		}
		else CS_ASSERT(imageAcquireResult == VK_SUCCESS, "Failed to acquire swap chain image!");

		// Reset the command buffer for use again
		CS_ASSERT(vkResetCommandBuffer(cmd, 0) == VK_SUCCESS, "Failed to reset command buffer!");
		VkCommandBufferBeginInfo beginInfo = Create::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		CS_ASSERT(vkBeginCommandBuffer(cmd, &beginInfo) == VK_SUCCESS, "Failed to begin command buffer!");

		// Set dynamic viewport and scissor, negative height to flip the viewport
		VkViewport viewport = Create::Viewport(0.0f, static_cast<float>(this->swapchain.extent.height), static_cast<float>(this->swapchain.extent.width), -static_cast<float>(this->swapchain.extent.height), 0.0f, 1.0f);
		VkRect2D scissor = Create::Rect2D({ 0, 0 }, this->swapchain.extent);
		vkCmdSetViewport(cmd, 0, 1, &viewport);
		vkCmdSetScissor(cmd, 0, 1, &scissor);

		// Begin render pass
		VkClearValue depthClear;
		depthClear.depthStencil.depth = 1.0f;
		VkClearValue clearValues[2] = { clearColor, depthClear };
		VkRenderPassBeginInfo renderPassBeginInfo = Create::RenderPassBeginInfo(
			nullptr, this->defaultRenderPass, this->framebuffers[this->state.swapchainImageIndex], { {0, 0}, this->swapchain.extent }, 2, &clearValues[0]
		);
		vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
		// Bind the vertex buffers
		constexpr size_t INDICES = 0, POSITION = 1, NORMALS = 2, TEXTURE_UVS = 3;
		const std::array<VkBuffer, 3> vertexBuffers = {
			this->vertexBuffers[POSITION].buffer,
			this->vertexBuffers[NORMALS].buffer,
			this->vertexBuffers[TEXTURE_UVS].buffer
		};
		const std::array<VkDeviceSize, 3> offsets = { 0, 0, 0 };
		vkCmdBindVertexBuffers(cmd, 0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
		vkCmdBindIndexBuffer(cmd, this->vertexBuffers[INDICES].buffer, 0, VK_INDEX_TYPE_UINT32);
	}
	void Renderer::RendererImpl::EndFrame()
	{
		FrameData& currentFrame = this->GetCurrentFrameData();
		VkCommandBuffer cmd = currentFrame.commandQueue.commandBuffer;

		// End render pass and command buffers
		vkCmdEndRenderPass(cmd);
		CS_ASSERT(vkEndCommandBuffer(cmd) == VK_SUCCESS, "Failed to end command buffer!");

		// Submit command buffer to queue
		VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo = Create::SubmitInfo(
			nullptr, 1, &currentFrame.presentSemaphore, &waitStages, 1, &cmd, 1, &currentFrame.renderSemaphore
		);
		CS_ASSERT(vkQueueSubmit(this->queues.universal, 1, &submitInfo, currentFrame.commandQueue.completionFence) == VK_SUCCESS, "Failed to submit to queue!");
	}
	void Renderer::RendererImpl::BindPipeline(uint32_t pipelineIndex)
	{
		CS_ASSERT(pipelineIndex < this->pipelines.size(), "Index " + std::to_string(pipelineIndex) + " is an Invalid pipeline index, pipeline count: " + std::to_string(this->pipelines.size()));

		FrameData& currentFrame = this->GetCurrentFrameData();
		VkCommandBuffer cmd = currentFrame.commandQueue.commandBuffer;

		this->state.boundPipelineIndex = pipelineIndex;

		const VkPipelineLayout currentLayout = this->pipelineLayouts[pipelineIndex];
		const VkDescriptorSet currentSet = this->descriptorSets[pipelineIndex * this->state.framesInFlight + this->GetFrameIndex()];

		const std::vector<uint32_t>& dynamicOffsets = this->descriptorSetLayoutOffsets[pipelineIndex];

		// Ignore last element of dynamic offsets because it shows the end of the buffer
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, currentLayout, 0, 1, &currentSet, dynamicOffsets.size() - 1, dynamicOffsets.data());
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelines[pipelineIndex]);
	}
	void Renderer::RendererImpl::UpdatePushConstant(ShaderStage stage, const void* data, size_t size)
	{
		FrameData& currentFrame = this->GetCurrentFrameData();
		VkCommandBuffer cmd = currentFrame.commandQueue.commandBuffer;

		VkShaderStageFlags stageFlags = 0;
		switch (stage)
		{
			case ShaderStage::Vertex: stageFlags = VK_SHADER_STAGE_VERTEX_BIT; break;
			case ShaderStage::Fragment: stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; break;
			default: { CS_ASSERT(false, "Unknown shader stage"); }
		}
		vkCmdPushConstants(cmd, this->pipelineLayouts[this->state.boundPipelineIndex], stageFlags, 0, size, data);
	}
	void Renderer::RendererImpl::Draw(uint32_t mesh)
	{
		FrameData& currentFrame = this->GetCurrentFrameData();
		VkCommandBuffer cmd = currentFrame.commandQueue.commandBuffer;

		uint32_t vertexCount = (this->indiceOffsets[mesh + 1] - this->indiceOffsets[mesh]);

		vkCmdDrawIndexed(cmd, vertexCount, 1, this->indiceOffsets[mesh], this->offsets[mesh], 0);
	}
	void Renderer::RendererImpl::PresentFrame()
	{
		const FrameData& currentFrame = this->GetCurrentFrameData();
		const VkCommandBuffer cmd = currentFrame.commandQueue.commandBuffer;

		// Attempt to present the image, if the swapchain is out of date or suboptimal, recreate it
		// We can't present again however because the framebuffers are invalid, so we just pass
		const VkPresentInfoKHR presentInfo = Create::PresentInfoKHR(
			nullptr, 1, &currentFrame.renderSemaphore, 1, &this->swapchain.swapchain, &this->state.swapchainImageIndex, nullptr
		);
		const VkResult presentResult = vkQueuePresentKHR(this->queues.universal, &presentInfo);
		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || this->state.didFramebufferResize)
		{
			this->state.didFramebufferResize = false;
			RecreateSwapchain();
		}
		else CS_ASSERT(presentResult == VK_SUCCESS, "Failed to present!");
		// Advance the frame index to use the next buffer
		this->state.frameIndex = (this->state.frameIndex + 1) % this->state.framesInFlight;
	}
	void Renderer::RendererImpl::UpdateDescriptorSet(uint32_t descriptorSetIndex, uint32_t binding, const void* data, size_t size)
	{
		CS_ASSERT(size <= (this->descriptorSetLayoutOffsets[descriptorSetIndex][binding + 1] - this->descriptorSetLayoutOffsets[descriptorSetIndex][binding]), "Provided data is larger than the descriptor set size, This can lead to buffer overflow!");

		const FrameData& currentFrame = this->GetCurrentFrameData();
		const VkCommandBuffer cmd = currentFrame.commandQueue.commandBuffer;

		uint32_t memOffset = this->descriptorSetLayoutOffsets[descriptorSetIndex][binding];

		memcpy(static_cast<char*>(this->descriptorSetBuffers[this->GetFrameIndex()].memoryLocation) + memOffset, data, size);
	}
}