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

		const Pipeline currentPipeline = this->pipelines[pipelineIndex];

		std::vector<VkDescriptorSet> descriptorSets;
		std::vector<uint32_t> offsets;
		for (uint32_t i = 0; i < currentPipeline.dataDescriptorHandles.size(); i++)
		{
			descriptorSets.push_back(this->dataDescriptorSets[currentPipeline.dataDescriptorHandles[i] * this->rendererInfo.framesInFlight + this->GetFrameIndex()]);
			// -1 because we ignore the last element because it specifies the end of the buffer
			offsets.insert(
				offsets.end(),
				this->dataDescriptorSetLayoutOffsets[currentPipeline.dataDescriptorHandles[i]].begin(),
				this->dataDescriptorSetLayoutOffsets[currentPipeline.dataDescriptorHandles[i]].end() - 1
			);
		}
		cmd.BindDescriptorSets(currentPipeline.layout, descriptorSets, offsets);
		cmd.BindPipeline(currentPipeline.pipeline);

		// Bind the vertex buffers
		std::vector<VkBuffer> buffers;
		for (uint32_t i = 0; i < currentPipeline.vertexAttributeFlags.size(); i++)
		{
			buffers.push_back(this->vertexBuffers[static_cast<size_t>(currentPipeline.vertexAttributeFlags[i]) + 1].buffer);
		}
		const std::vector<VkDeviceSize> bufferOffsets(buffers.size(), 0);

		cmd.BindVertexBuffers(buffers, bufferOffsets);
		cmd.BindIndexBuffer(this->vertexBuffers[0].buffer);
	}
	void Renderer::RendererImpl::BindTexture(uint32_t set, uint32_t textureIndex)
	{
		const FrameData& currentFrame = this->GetCurrentFrameData();
		const internal::CommandQueue& cmd = currentFrame.commandQueue;

		const Pipeline currentPipeline = this->pipelines[this->state.boundPipelineIndex];

		if (textureIndex == SHADOW_MAP_ID)
		{

		}
		else
		{
			cmd.BindDescriptorSets(currentPipeline.layout, { this->imageDescriptorSets[textureIndex] }, {}, set);
		}
	}
	void Renderer::RendererImpl::UpdatePushConstant(ShaderStage stage, const void* data, uint32_t size)
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
		cmd.PushConstants(this->pipelines[this->state.boundPipelineIndex].layout, data, size, stageFlags);
	}
	void Renderer::RendererImpl::Draw(uint32_t mesh)
	{
		const FrameData& currentFrame = this->GetCurrentFrameData();
		const internal::CommandQueue cmd = currentFrame.commandQueue;

		uint32_t vertexCount = (this->offsets[0][mesh + 1] - this->offsets[0][mesh]) * 3;

		// It's pretty safe to assume that every draw will have a vertex position
		cmd.DrawIndexed(vertexCount, 1, this->offsets[0][mesh] * 3, this->offsets[1][mesh], 0);
	}
	void Renderer::RendererImpl::PresentFrame()
	{
		const FrameData& currentFrame = this->GetCurrentFrameData();

		// Attempt to present the image, if the swapchain is out of date or suboptimal, recreate it
		// We can't present again however because the framebuffers are invalid, so we just pass
		const VkPresentInfoKHR presentInfo = Create::PresentInfoKHR(
			1, &currentFrame.renderSemaphore, 1, &this->swapchain.swapchain, &this->state.swapchainImageIndex, nullptr
		);
		const VkResult presentResult = vkQueuePresentKHR(this->queues.universal.queue, &presentInfo);
		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || this->state.didFramebufferResize)
		{
			this->state.didFramebufferResize = false;
			RecreateSwapchain();
		}
		else CS_ASSERT(presentResult == VK_SUCCESS, "Failed to present!");
		// Advance the frame index to use the next buffer
		this->state.frameIndex = (this->state.frameIndex + 1) % this->rendererInfo.framesInFlight;
	}
	void Renderer::RendererImpl::UpdateDescriptorSet(uint32_t descriptorSetIndex, uint32_t binding, const void* data, size_t size)
	{
		CS_ASSERT(size <= (this->dataDescriptorSetLayoutOffsets[descriptorSetIndex][binding + 1] - this->dataDescriptorSetLayoutOffsets[descriptorSetIndex][binding]), "Provided data is larger than the descriptor set size, This can lead to buffer overflow!");
		const uint32_t frameIndex = this->GetFrameIndex();
		const uint32_t memOffset = this->dataDescriptorSetLayoutOffsets[descriptorSetIndex][binding];
		memcpy(static_cast<char*>(this->dataDescriptorSetBuffers[frameIndex].memoryLocation) + memOffset, data, size);
	}
}