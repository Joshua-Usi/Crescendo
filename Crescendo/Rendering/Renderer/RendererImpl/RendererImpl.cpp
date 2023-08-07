#include "RendererImpl.hpp"

#include "internal/Create.hpp"
#include "Rendering/Reflection/Reflection.hpp"

namespace Crescendo
{
	namespace Create = internal::Create;
	FrameData& Renderer::RendererImpl::GetCurrentFrameData()
	{
		return this->state.frameData[this->state.frameIndex];
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
	void Renderer::RendererImpl::UploadPipeline(const std::vector<uint8_t>& vertexShader, const std::vector<uint8_t>& fragmentShader, const PipelineVariantBuilderInfo& info)
	{
		// Allocate space for new pipeline
		this->pipelines.resize(this->pipelines.size() + 1);
		this->pipelineLayouts.resize(this->pipelineLayouts.size() + 1);

		// Load shader modules
		VkShaderModule vertModule = this->CreateShaderModule(vertexShader);
		VkShaderModule fragModule = this->CreateShaderModule(fragmentShader);

		// Create reflection data
		SpirvReflection vertReflection = ReflectSpirv(vertexShader);
		SpirvReflection fragReflection = ReflectSpirv(fragmentShader);

		// Get binding and attribute descriptions
		std::vector<VkVertexInputBindingDescription> bindingDescriptions = vertReflection.GetVertexBindings();
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = vertReflection.GetVertexAttributes();

		// Get push constant range
		VkPushConstantRange pushConstantRange = vertReflection.GetPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT);

		// Get descriptor set create info
		// VkDescriptorSetLayoutCreateInfo setInfo = vertReflection.GetDescriptorSetLayouts(VK_SHADER_STAGE_VERTEX_BIT);

		// Create the pipeline layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = Create::PipelineLayoutCreateInfo(0, 0, nullptr, 1, &pushConstantRange);
		CS_ASSERT(vkCreatePipelineLayout(this->device, &pipelineLayoutInfo, nullptr, &this->pipelineLayouts.back()) == VK_SUCCESS, "Failed to create pipeline layout!");

		// We we always use dyanmic states, there is really no performance penalty for just viewports and scissors and it means we don't need to recreate pipelines when resizing
		constexpr std::array<VkDynamicState, 2> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		// Create the information required to build the pipeline
		PipelineBuilderInfo pipelineBuilderInfo = {};
		pipelineBuilderInfo.dynamicState = Create::PipelineDynamicStateCreateInfo(
			dynamicStates.size(), dynamicStates.data()
		);
		pipelineBuilderInfo.renderPass = this->defaultRenderPass;
		pipelineBuilderInfo.shaderStagesInfo.push_back(Create::PipelineShaderStageCreateInfo(
			nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, vertModule, "main", nullptr
		));
		pipelineBuilderInfo.shaderStagesInfo.push_back(Create::PipelineShaderStageCreateInfo(
			nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, fragModule, "main", nullptr
		));
		pipelineBuilderInfo.vertexInputInfo = Create::PipelineVertexInputStateCreateInfo(
			nullptr, static_cast<uint32_t>(bindingDescriptions.size()), bindingDescriptions.data(), static_cast<uint32_t>(attributeDescriptions.size()), attributeDescriptions.data()
		);
		pipelineBuilderInfo.inputAssemblyInfo = Create::PipelineInputAssemblyStateCreateInfo(
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE
		);
		pipelineBuilderInfo.rasterizerInfo = Create::PipelineRasterizationStateCreateInfo(
			nullptr, VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE,
			VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f
		);
		pipelineBuilderInfo.multisamplingInfo = Create::PipelineMultisampleStateCreateInfo(
			nullptr, VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 1.0f, nullptr, VK_FALSE, VK_FALSE
		);
		pipelineBuilderInfo.colorBlendAttachment = Create::PipelineColorBlendAttachmentState(
			VK_TRUE, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		);
		pipelineBuilderInfo.depthStencilInfo = Create::PipelineDepthStencilStateCreateInfo(
			0, VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS, VK_FALSE, VK_FALSE, {}, {}, 0.0f, 1.0f
		);
		pipelineBuilderInfo.pipelineLayout = this->pipelineLayouts.back();

		// Create the pipeline
		this->pipelines.back() = this->CreatePipeline(pipelineBuilderInfo);

		// Destroy shader modules, reflection will delete itself
		vkDestroyShaderModule(this->device, fragModule, nullptr);
		vkDestroyShaderModule(this->device, vertModule, nullptr);
	}
	void Renderer::RendererImpl::UploadMesh(const std::vector<float>& vertices, const std::vector<float>& normals, const std::vector<float>& textureUVs, const std::vector<uint32_t>& indices)
	{
		constexpr size_t INDICES_PER_TRIANGLE =  3;
		constexpr size_t VERTICES_PER_INDEX =    3;
		constexpr size_t NORMALS_PER_INDEX =     3;
		constexpr size_t UVS_PER_INDEX =         2;

		constexpr size_t INDICES = 0, POSITION = 1, NORMALS = 2, TEXTURE_UVS = 3;

		CS_ASSERT(indices.size() % 3 == 0, "Invalid mesh indices!");
		CS_ASSERT(vertices.size()   % 3 == 0, "Invalid mesh vertices!");
		CS_ASSERT(normals.size()    % 3 == 0, "Invalid mesh normals!");
		CS_ASSERT(textureUVs.size() % 2 == 0, "Invalid mesh texture UVs!");
		CS_ASSERT(this->offsets.back() + vertices.size() / 3 <= this->state.maxBufferSize, "Mesh would overflow buffer!");

		std::memcpy(static_cast<uint32_t*>(this->vertexBuffers[INDICES].memoryLocation)  + this->indiceOffsets.back() * INDICES_PER_TRIANGLE, indices.data(),    indices.size()    * sizeof(uint32_t));
		std::memcpy(static_cast<float*>   (this->vertexBuffers[POSITION].memoryLocation)    + this->offsets.back()       * VERTICES_PER_INDEX,   vertices.data(),   vertices.size()   * sizeof(float));
		std::memcpy(static_cast<float*>   (this->vertexBuffers[NORMALS].memoryLocation)     + this->offsets.back()       * NORMALS_PER_INDEX,    normals.data(),    normals.size()    * sizeof(float));
		std::memcpy(static_cast<float*>   (this->vertexBuffers[TEXTURE_UVS].memoryLocation) + this->offsets.back()       * UVS_PER_INDEX,        textureUVs.data(), textureUVs.size() * sizeof(float));

		uint32_t last = this->offsets.back();
		uint32_t lastIndices = this->indiceOffsets.back();
		uint32_t uniqueVerticeCount = vertices.size() / VERTICES_PER_INDEX;
		uint32_t triangleCount = indices.size() / INDICES_PER_TRIANGLE;

		this->offsets.push_back(last + uniqueVerticeCount);
		this->indiceOffsets.push_back(lastIndices + triangleCount);

	}
}