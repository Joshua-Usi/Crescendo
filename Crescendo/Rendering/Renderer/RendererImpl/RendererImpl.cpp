#include "RendererImpl.hpp"

#include "./internal/Create.hpp"

namespace Crescendo
{
	namespace Create = internal::Create;
	FrameData& Renderer::RendererImpl::GetCurrentFrameData()
	{
		return this->state.frameData;
	}
	void Renderer::RendererImpl::RecreateSwapchain()
	{
		if (this->state.temp.sizeToResize.width == 0 || this->state.temp.sizeToResize.height == 0) return;
		vkDeviceWaitIdle(this->device);
		// Destroy old buffers
		this->swapChainDeletionQueue.Flush();
		// Create new info, with new extent. This could be a little slow since it is a massive struct but it's not like we're recreating the swapchain every frame
		BuilderInfo info = {};
		info.windowExtent = this->state.temp.sizeToResize;
		// Create swapchain and framebuffers again
		this->InitialiseSwapchain(info);
		this->InitialiseFramebuffers(info);
	}
	VkShaderModule Renderer::RendererImpl::CreateShaderModule(const std::vector<uint8_t>& code)
	{
		VkShaderModule shaderModule;
		const VkShaderModuleCreateInfo createInfo = Create::ShaderModuleCreateInfo(code);
		CS_ASSERT(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) == VK_SUCCESS, "Failed to create shader module!");
		return shaderModule;
	}
	VkRenderPass Renderer::RendererImpl::CreateRenderPass(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& subpassDependencies)
	{
		const VkRenderPassCreateInfo renderPassInfo = Create::RenderPassCreateInfo(
			attachments, subpasses, subpassDependencies
		);
		VkRenderPass renderPass;
		vkCreateRenderPass(this->device, &renderPassInfo, nullptr, &renderPass);
		return renderPass;
	}
	VkPipeline Renderer::RendererImpl::CreatePipeline(PipelineBuilderInfo& info)
	{
		VkPipelineViewportStateCreateInfo viewportState = Create::PipelineViewportStateCreateInfo(nullptr, 1, nullptr, 1, nullptr);
		VkPipelineColorBlendStateCreateInfo colorBlending = Create::PipelineColorBlendStateCreateInfo(
			nullptr, 0, VK_FALSE, VK_LOGIC_OP_COPY, 1, &info.colorBlendAttachment, { 0.0f, 0.0f, 0.0f, 0.0f }
		);
		// Fill pipeline info
		VkGraphicsPipelineCreateInfo pipelineInfo = Create::GraphicsPipelineCreateInfo(
			nullptr, 0,
			static_cast<uint32_t>(info.shaderStagesInfo.size()), info.shaderStagesInfo.data(),
			&info.vertexInputInfo, &info.inputAssemblyInfo, &info.tessellationInfo,
			&viewportState, &info.rasterizerInfo, &info.multisamplingInfo,
			&info.depthStencilInfo, &colorBlending, &info.dynamicState,
			info.pipelineLayout, info.renderPass, 0, VK_NULL_HANDLE, 0
		);
		// Sometimes pipelines can fail to generate, but it's not a critical error
		// So we just return a null handle and let the user handle it
		VkPipeline pipeline = VK_NULL_HANDLE;
		if (vkCreateGraphicsPipelines(this->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
		{
			std::cout << "Failed to create graphics pipeline!" << std::endl;
		}
		return pipeline;
	}
	Buffer Renderer::RendererImpl::CreateBuffer(size_t allocationSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
	{
		const VkBufferCreateInfo bufferInfo = Create::BufferCreateInfo(nullptr, 0, allocationSize, usage, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr);
		VmaAllocationCreateInfo vmaAllocInfo{};
		vmaAllocInfo.usage = memoryUsage;
		Buffer buffer = { nullptr, nullptr };
		CS_ASSERT(vmaCreateBuffer(this->allocator, &bufferInfo, &vmaAllocInfo, &buffer.buffer, &buffer.allocation, nullptr) == VK_SUCCESS, "Failed to create buffer!");
		// You'll also need to explicitly unmap
		vmaMapMemory(this->allocator, buffer.allocation, &buffer.memoryLocation);
		return buffer;
	}
	void Renderer::RendererImpl::UploadMesh(const Mesh& mesh)
	{
		constexpr uint32_t INDICES_PER_TRIANGLE = 3;
		constexpr uint32_t VERTICES_PER_INDEX = 3 * INDICES_PER_TRIANGLE;
		constexpr uint32_t NORMALS_PER_INDEX = 3 * INDICES_PER_TRIANGLE;
		constexpr uint32_t TEXTURE_UVS_PER_INDEX = 2 * INDICES_PER_TRIANGLE;

		CS_ASSERT(mesh.vertices.size()   % 3 == 0, "Invalid mesh vertices");
		CS_ASSERT(mesh.normals.size()    % 3 == 0, "Invalid mesh normals");
		CS_ASSERT(mesh.textureUVs.size() % 2 == 0, "Invalid mesh texture UVs");
		CS_ASSERT(mesh.indices.size()    % 3 == 0, "Invalid mesh indices");
		
		std::cout << "Copying into offset: " << this->offsets.back() << std::endl;
		std::cout << "Copying into indice offset: " << this->indiceOffsets.back() << std::endl;

		std::memcpy(static_cast<char*>(this->positionBuffer.memoryLocation) +  this->offsets.back() *       sizeof(float) *    VERTICES_PER_INDEX,       mesh.vertices.data(),   mesh.vertices.size() *   sizeof(float));
		std::memcpy(static_cast<char*>(this->normalBuffer.memoryLocation) +    this->offsets.back() *       sizeof(float) *    NORMALS_PER_INDEX,        mesh.normals.data(),    mesh.normals.size() *    sizeof(float));
		std::memcpy(static_cast<char*>(this->textureUVBuffer.memoryLocation) + this->offsets.back() *       sizeof(float) *    TEXTURE_UVS_PER_INDEX,    mesh.textureUVs.data(), mesh.textureUVs.size() * sizeof(float));
		std::memcpy(static_cast<char*>(this->indexBuffer.memoryLocation) +     this->indiceOffsets.back() * sizeof(uint32_t) * INDICES_PER_TRIANGLE,     mesh.indices.data(),    mesh.indices.size() *    sizeof(uint32_t));
		
		uint32_t last = this->offsets.back();
		uint32_t lastIndices = this->indiceOffsets.back();
		uint32_t uniqueVerticeCount = mesh.vertices.size() / 3;
		uint32_t triangleCount = mesh.indices.size() / INDICES_PER_TRIANGLE;

		this->offsets.push_back(last + uniqueVerticeCount);
		this->indiceOffsets.push_back(lastIndices + triangleCount);

		std::cout << "Buffer is now: " << this->offsets.back() << std::endl;
		std::cout << "Indice buffer is now: " << this->indiceOffsets.back() << std::endl;
	}
}