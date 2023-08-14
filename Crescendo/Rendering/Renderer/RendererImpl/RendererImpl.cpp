#include "RendererImpl.hpp"

#include "Rendering/Reflection/Reflection.hpp"

namespace Crescendo
{
	size_t PaddedSize(size_t size, size_t alignmentRequirement)
	{
		return (alignmentRequirement > 0) ? (size + alignmentRequirement - 1) & ~(alignmentRequirement - 1) : size;
	}
	FrameData& Renderer::RendererImpl::GetCurrentFrameData()
	{
		return this->state.frameData[this->state.frameIndex];
	}
	void Renderer::RendererImpl::RecreateSwapchain()
	{
		vkDeviceWaitIdle(this->device);
		// Get glfw window size
		int width = 0, height = 0;
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(this->window, &width, &height);
			glfwWaitEvents();
		}

		// Destroy old buffers
		this->swapChainDeletionQueue.Flush();
		// Create new info, with new extent. This could be a little slow since it is a massive struct but it's not like we're recreating the swapchain every frame
		BuilderInfo info = {};
		info.windowExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
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
		// You don't need to unmap since deletion implicitly unmap
		// NEVERMIND, VMA WANTS ME TO UNMAP
		// Only map if not GPU only
		if (memoryUsage != VMA_MEMORY_USAGE_GPU_ONLY) vmaMapMemory(this->allocator, buffer.allocation, &buffer.memoryLocation);
		return buffer;
	}
	void Renderer::RendererImpl::UploadPipeline(const std::vector<uint8_t>& vertexShader, const std::vector<uint8_t>& fragmentShader, const PipelineVariant& variant)
	{
		const VkDeviceSize UNIFORM_ALIGNMENT = this->physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;

		// Allocate space for new pipeline
		this->pipelineLayouts.resize(this->pipelineLayouts.size() + 1);
		this->descriptorSetLayouts.resize(this->descriptorSetLayouts.size() + 1);

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

		// Collect descriptor set layout bindings
		const std::vector<VkDescriptorSetLayoutBinding> fraglayoutBindings = fragReflection.GetDescriptorSetBindings(VK_SHADER_STAGE_FRAGMENT_BIT);
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings(vertReflection.GetDescriptorSetBindings(VK_SHADER_STAGE_VERTEX_BIT));
		layoutBindings.insert(layoutBindings.end(), fraglayoutBindings.begin(), fraglayoutBindings.end());

		// Collect layout binding
		std::vector<DescriptorSetLayout> sets(vertReflection.descriptorSets);
		sets.insert(sets.end(), fragReflection.descriptorSets.begin(), fragReflection.descriptorSets.end());

		// Create the set layout
		VkDescriptorSetLayoutCreateInfo setInfo = Create::DescriptorSetLayoutCreateInfo(layoutBindings);
		vkCreateDescriptorSetLayout(this->device, &setInfo, nullptr, &this->descriptorSetLayouts.back());

		// Generate descriptor offsets
		{
			std::vector<uint32_t> offsets(1, 0);
			for (const auto& set : sets)
			{
				offsets.push_back(offsets.back() + PaddedSize(set.GetSize(), UNIFORM_ALIGNMENT));
			}
			this->descriptorSetLayoutOffsets.push_back(offsets);
		}

		// Create descriptor sets for each frame in flight	
		size_t lastSetSize = this->descriptorSets.size();
		this->descriptorSets.resize(lastSetSize + this->state.framesInFlight);
		for (uint32_t i = 0; i < this->state.framesInFlight; i++)
		{
			// Allocate descriptor set
			VkDescriptorSetAllocateInfo allocInfo = Create::DescriptorSetAllocateInfo(this->descriptorPool, this->descriptorSetLayouts.back());
			vkAllocateDescriptorSets(this->device, &allocInfo, &this->descriptorSets[lastSetSize + i]);
			
			for (uint32_t j = 0; j < sets.size(); j++)
			{
				// Show where it points in the buffer
				VkDescriptorBufferInfo bufferInfo = Create::DescriptorBufferInfo(
					this->descriptorSetBuffers[i].buffer, 0, sets[j].GetSize()
				);
				// Write to set
				VkWriteDescriptorSet setWrite = Create::WriteDescriptorSet(
					this->descriptorSets[lastSetSize + i], sets[j].binding, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, nullptr, &bufferInfo, nullptr
				);
				// Update set
				vkUpdateDescriptorSets(this->device, 1, &setWrite, 0, nullptr);
			}
		}

		// Create the pipeline layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = Create::PipelineLayoutCreateInfo(
			0, 1, &this->descriptorSetLayouts.back(), 1, &pushConstantRange
		);
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
			nullptr, VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT,
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
		this->pipelines.push_back(this->CreatePipeline(pipelineBuilderInfo));

		// Destroy shader modules, reflection will delete itself
		vkDestroyShaderModule(this->device, fragModule, nullptr);
		vkDestroyShaderModule(this->device, vertModule, nullptr);
	}
	void Renderer::RendererImpl::UploadMesh(const std::vector<float>& vertices, const std::vector<float>& normals, const std::vector<float>& textureUVs, const std::vector<uint32_t>& indices)
	{
		constexpr size_t VERTICES_PER_INDEX = 3;
		constexpr size_t NORMALS_PER_INDEX = 3;
		constexpr size_t UVS_PER_INDEX = 2;

		constexpr size_t INDICES = 0, POSITION = 1, NORMALS = 2, TEXTURE_UVS = 3;
		
		CS_ASSERT(indices.size()    % 3 == 0, "Invalid mesh indices!");
		CS_ASSERT(vertices.size()   % 3 == 0, "Invalid mesh vertices!");
		CS_ASSERT(normals.size()    % 3 == 0, "Invalid mesh normals!");
		CS_ASSERT(textureUVs.size() % 2 == 0, "Invalid mesh texture UVs!");
		// TODO assert for potential buffer overflow

		Buffer stagingIndices    = this->CreateBuffer(indices.size()    * sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		Buffer stagingVertices   = this->CreateBuffer(vertices.size()   * sizeof(float),    VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		Buffer stagingNormals    = this->CreateBuffer(normals.size()    * sizeof(float),    VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		Buffer stagingTextureUVs = this->CreateBuffer(textureUVs.size() * sizeof(float),    VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		memcpy(stagingIndices.memoryLocation,    indices.data(),    indices.size()    * sizeof(uint32_t));
		memcpy(stagingVertices.memoryLocation,   vertices.data(),   vertices.size()   * sizeof(float));
		memcpy(stagingNormals.memoryLocation,    normals.data(),    normals.size()    * sizeof(float));
		memcpy(stagingTextureUVs.memoryLocation, textureUVs.data(), textureUVs.size() * sizeof(float));

		this->OneTimeSubmit([=](VkCommandBuffer cmd) {
			VkBufferCopy copyIndices    = Create::BufferCopy(0, this->indiceOffsets.back() * sizeof(uint32_t),                   indices.size()    * sizeof(uint32_t));
			VkBufferCopy copyVertices   = Create::BufferCopy(0, this->offsets.back()       * sizeof(float) * VERTICES_PER_INDEX, vertices.size()   * sizeof(float));
			VkBufferCopy copyNormals    = Create::BufferCopy(0, this->offsets.back()       * sizeof(float) * NORMALS_PER_INDEX,  normals.size()    * sizeof(float));
			VkBufferCopy copyTextureUVs = Create::BufferCopy(0, this->offsets.back()       * sizeof(float) * UVS_PER_INDEX,      textureUVs.size() * sizeof(float));
		
			vkCmdCopyBuffer(cmd, stagingIndices.buffer,    this->vertexBuffers[INDICES].buffer,     1, &copyIndices);
			vkCmdCopyBuffer(cmd, stagingVertices.buffer,   this->vertexBuffers[POSITION].buffer,    1, &copyVertices);
			vkCmdCopyBuffer(cmd, stagingNormals.buffer,    this->vertexBuffers[NORMALS].buffer,     1, &copyNormals);
			vkCmdCopyBuffer(cmd, stagingTextureUVs.buffer, this->vertexBuffers[TEXTURE_UVS].buffer, 1, &copyTextureUVs);
		});

		uint32_t uniqueVerticeCount = vertices.size() / VERTICES_PER_INDEX;
		uint32_t indicesOffset = indices.size();

		this->offsets.push_back(this->offsets.back() + uniqueVerticeCount);
		this->indiceOffsets.push_back(this->indiceOffsets.back() + indicesOffset);

		vmaUnmapMemory(this->allocator, stagingIndices.allocation);
		vmaUnmapMemory(this->allocator, stagingVertices.allocation);
		vmaUnmapMemory(this->allocator, stagingNormals.allocation);
		vmaUnmapMemory(this->allocator, stagingTextureUVs.allocation);

		vmaDestroyBuffer(this->allocator, stagingIndices.buffer,    stagingIndices.allocation);
		vmaDestroyBuffer(this->allocator, stagingVertices.buffer,   stagingVertices.allocation);
		vmaDestroyBuffer(this->allocator, stagingNormals.buffer,    stagingNormals.allocation);
		vmaDestroyBuffer(this->allocator, stagingTextureUVs.buffer, stagingTextureUVs.allocation);
	}
	void Renderer::RendererImpl::OneTimeSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
	{
		const VkCommandBuffer cmd = this->uploadQueue.commandBuffer;

		// Begin the command buffer
		VkCommandBufferBeginInfo beginInfo = Create::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		CS_ASSERT(vkBeginCommandBuffer(cmd, &beginInfo) == VK_SUCCESS, "Failed to begin one time submit!");
		function(cmd);
		CS_ASSERT(vkEndCommandBuffer(cmd) == VK_SUCCESS, "Failed to end one time submit!");

		VkSubmitInfo submit = Create::SubmitInfo(nullptr, 0, nullptr, nullptr, 1, &cmd, 0, nullptr);
		CS_ASSERT(vkQueueSubmit(this->queues.universal, 1, &submit, this->uploadQueue.completionFence) == VK_SUCCESS, "Failed to submit one time submit!");

		vkWaitForFences(this->device, 1, &this->uploadQueue.completionFence, VK_TRUE, UINT64_MAX);
		vkResetFences(this->device, 1, &this->uploadQueue.completionFence);

		CS_ASSERT(vkResetCommandPool(this->device, this->uploadQueue.commandPool, 0) == VK_SUCCESS, "Failed to reset upload command pool!");
	}
}