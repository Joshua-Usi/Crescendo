#include "RendererImpl.hpp"

namespace Crescendo
{
	void Renderer::RendererImpl::BeginFrame(const VkClearValue& clearColor)
	{
		// One second in nanoseconds
		constexpr uint64_t ONE_SECOND = 1000000000;
		const FrameData& currentFrame = this->GetCurrentFrameData();
		const internal::CommandQueue& cmd = currentFrame.commandQueue;

		cmd.WaitCompletion(ONE_SECOND);

		// Acquire image from swapchain
		VkResult imageAcquireResult = vkAcquireNextImageKHR(this->device, this->swapchain.swapchain, ONE_SECOND, currentFrame.presentSemaphore, VK_NULL_HANDLE, &this->state.swapchainImageIndex);
		if (imageAcquireResult == VK_ERROR_OUT_OF_DATE_KHR || imageAcquireResult == VK_SUBOPTIMAL_KHR)
		{
			this->RecreateSwapchain();
			imageAcquireResult = vkAcquireNextImageKHR(this->device, this->swapchain.swapchain, ONE_SECOND, currentFrame.presentSemaphore, VK_NULL_HANDLE, &this->state.swapchainImageIndex);
		}
		else CS_ASSERT(imageAcquireResult == VK_SUCCESS, "Failed to acquire swap chain image!");

		// Reset the command buffer for use again
		cmd.Reset();
		cmd.Begin();

		// Set dynamic viewport and scissor, negative height to flip the viewport
		VkViewport viewport = Create::Viewport(0.0f, static_cast<float>(this->swapchain.extent.height), static_cast<float>(this->swapchain.extent.width), -static_cast<float>(this->swapchain.extent.height), 0.0f, 1.0f);
		VkRect2D scissor = Create::Rect2D({ 0, 0 }, this->swapchain.extent);
		cmd.SetViewport(viewport);
		cmd.SetScissor(scissor);

		// Begin render pass
		VkClearValue depthClear = {};
		depthClear.depthStencil.depth = 1.0f;
		std::array<VkClearValue, 2> clearValues = { clearColor, depthClear };
		VkRenderPassBeginInfo renderPassInfo = Create::RenderPassBeginInfo(
			nullptr, this->defaultRenderPass, this->framebuffers[this->state.swapchainImageIndex], { {0, 0}, this->swapchain.extent }, clearValues.size(), clearValues.data()
		);
		cmd.BeginRenderPass(renderPassInfo);
	
		// Bind the vertex buffers
		constexpr size_t INDICES = 0, POSITION = 1, NORMALS = 2, TEXTURE_UVS = 3;
		cmd.BindVertexBuffers({
			this->vertexBuffers[POSITION].buffer,
			this->vertexBuffers[NORMALS].buffer,
			this->vertexBuffers[TEXTURE_UVS].buffer
		}, { 0, 0, 0 });
		cmd.BindIndexBuffer(this->vertexBuffers[INDICES].buffer);
	}
	void Renderer::RendererImpl::EndFrame()
	{
		const FrameData& currentFrame = this->GetCurrentFrameData();
		const internal::CommandQueue& cmd = currentFrame.commandQueue;

		cmd.EndRenderPass();
		cmd.End();
		cmd.Submit({ currentFrame.presentSemaphore }, { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }, { currentFrame.renderSemaphore });
	}
	void Renderer::RendererImpl::BindPipeline(uint32_t pipelineIndex)
	{
		CS_ASSERT(pipelineIndex < this->pipelines.size(), "Index " + std::to_string(pipelineIndex) + " is an Invalid pipeline index, pipeline count: " + std::to_string(this->pipelines.size()));

		const FrameData& currentFrame = this->GetCurrentFrameData();
		const internal::CommandQueue& cmd = currentFrame.commandQueue;

		this->state.boundPipelineIndex = pipelineIndex;

		const VkPipelineLayout currentLayout = this->pipelineLayouts[pipelineIndex];
		const VkDescriptorSet currentSet = this->descriptorSets[pipelineIndex * this->state.framesInFlight + this->GetFrameIndex()];

		const std::vector<uint32_t> dynamicOffsets(this->descriptorSetLayoutOffsets[pipelineIndex].begin(), this->descriptorSetLayoutOffsets[pipelineIndex].end() - 1);

		// Ignore last element of dynamic offsets because it shows the end of the buffer
		cmd.BindDescriptorSets(currentLayout, { currentSet }, dynamicOffsets);
		cmd.BindPipeline(this->pipelines[pipelineIndex]);
	}
	void Renderer::RendererImpl::UpdatePushConstant(ShaderStage stage, const void* data, size_t size)
	{
		const FrameData& currentFrame = this->GetCurrentFrameData();
		const internal::CommandQueue& cmd = currentFrame.commandQueue;

		VkShaderStageFlags stageFlags = 0;
		switch (stage)
		{
			case ShaderStage::Vertex: stageFlags = VK_SHADER_STAGE_VERTEX_BIT; break;
			case ShaderStage::Fragment: stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; break;
			default: { CS_ASSERT(false, "Unknown shader stage"); }
		}
		cmd.PushConstants(this->pipelineLayouts[this->state.boundPipelineIndex], data, size, stageFlags);
	}
	void Renderer::RendererImpl::Draw(uint32_t mesh)
	{
		const FrameData& currentFrame = this->GetCurrentFrameData();
		const internal::CommandQueue cmd = currentFrame.commandQueue;

		uint32_t vertexCount = (this->indiceOffsets[mesh + 1] - this->indiceOffsets[mesh]);
		cmd.DrawIndexed(vertexCount, 1, this->indiceOffsets[mesh], this->offsets[mesh], 0);
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
		uint32_t memOffset = this->descriptorSetLayoutOffsets[descriptorSetIndex][binding];
		memcpy(static_cast<char*>(this->descriptorSetBuffers[this->GetFrameIndex()].memoryLocation) + memOffset, data, size);
	}
}