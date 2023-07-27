#include "RendererImpl.hpp"

#include "./internal/Create.hpp"

namespace Crescendo
{
	namespace Create = internal::Create;
	void Renderer::RendererImpl::BeginFrame(const VkClearValue& clearColor)
	{
		// One second in nanoseconds
		constexpr uint64_t ONE_SECOND = 1000000000;

		FrameData& currentFrame = this->GetCurrentFrameData();
		VkCommandBuffer cmd = currentFrame.commandBuffer;

		// Wait for previous frame to finish
		vkWaitForFences(this->device, 1, &currentFrame.renderFence, true, ONE_SECOND);
		vkResetFences(this->device, 1, &currentFrame.renderFence);

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
		VkCommandBufferBeginInfo beginInfo = Create::CommandBufferBeginInfo(
			nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr
		);
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
	}
	void Renderer::RendererImpl::EndFrame()
	{
		FrameData& currentFrame = this->GetCurrentFrameData();
		VkCommandBuffer cmd = currentFrame.commandBuffer;

		// End render pass and command buffers
		vkCmdEndRenderPass(cmd);
		CS_ASSERT(vkEndCommandBuffer(cmd) == VK_SUCCESS, "Failed to end command buffer!");

		// Submit command buffer to queue
		VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo = Create::SubmitInfo(
			nullptr, 1, &currentFrame.presentSemaphore, &waitStages, 1, &cmd, 1, &currentFrame.renderSemaphore
		);
		CS_ASSERT(vkQueueSubmit(this->queues.universal, 1, &submitInfo, currentFrame.renderFence) == VK_SUCCESS, "Failed to submit to queue!");
	}
	void Renderer::RendererImpl::BindPipeline(uint32_t pipelineIndex)
	{
		CS_ASSERT(pipelineIndex < this->pipelines.size(), "Index " + std::to_string(pipelineIndex) + " is an Invalid pipeline index, pipeline count: " + std::to_string(this->pipelines.size()));

		FrameData& currentFrame = this->GetCurrentFrameData();
		VkCommandBuffer cmd = currentFrame.commandBuffer;

		this->state.boundPipelineIndex = pipelineIndex;
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelines[pipelineIndex]);
	}
	void Renderer::RendererImpl::UpdatePushConstant(ShaderStage stage, const void* data, size_t size)
	{
		VkShaderStageFlags stageFlags = 0;
		switch (stage)
		{
			case ShaderStage::Vertex: stageFlags = VK_SHADER_STAGE_VERTEX_BIT; break;
			case ShaderStage::Fragment: stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; break;
			default: { CS_ASSERT(false, "Unknown shader stage"); }
		}
		vkCmdPushConstants(this->GetCurrentFrameData().commandBuffer, this->pipelineLayouts[this->state.boundPipelineIndex], stageFlags, 0, size, data);
	}
	void Renderer::RendererImpl::Draw(uint32_t mesh)
	{
		constexpr uint32_t INDICES_PER_TRIANGLE = 3;
		constexpr uint32_t VERTICES_PER_INDEX = 3;

		FrameData& currentFrame = this->GetCurrentFrameData();
		VkCommandBuffer cmd = currentFrame.commandBuffer;

		std::array<VkBuffer, 3> vertexBuffers = { this->positionBuffer.buffer, this->normalBuffer.buffer, this->textureUVBuffer.buffer };
		std::array<VkDeviceSize, 3> offsets = { 0, 0, 0 };

		vkCmdBindVertexBuffers(cmd, 0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
		vkCmdBindIndexBuffer(cmd, this->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		uint32_t vertexCount = (this->offsets[mesh + 1] - this->offsets[mesh]) * INDICES_PER_TRIANGLE;

		vkCmdDrawIndexed(cmd, vertexCount, 1, this->indiceOffsets[mesh] * INDICES_PER_TRIANGLE, this->offsets[mesh] * VERTICES_PER_INDEX, 0);
	}
	void Renderer::RendererImpl::PresentFrame()
	{
		const FrameData& currentFrame = this->GetCurrentFrameData();
		const VkCommandBuffer cmd = currentFrame.commandBuffer;
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
	}
}