#include "RendererImpl.hpp"

#include "Rendering/Reflection/Reflection.hpp"

#include <numeric>

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
		CS_ASSERT(vkCreateShaderModule(this->device, &createInfo, nullptr, &shaderModule) == VK_SUCCESS, "Failed to create shader module!");
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
	void Renderer::RendererImpl::UploadPipeline(const std::vector<uint8_t>& vertexShader, const std::vector<uint8_t>& fragmentShader, const PipelineVariant& variant)
	{
		/* -------------------------------- 0. Initialisation -------------------------------- */

		constexpr VkPolygonMode FILL_MODE_MAP[3] = { VK_POLYGON_MODE_FILL, VK_POLYGON_MODE_LINE, VK_POLYGON_MODE_POINT };
		constexpr VkCompareOp DEPTH_COMPARE_MAP[8] = { VK_COMPARE_OP_NEVER, VK_COMPARE_OP_LESS, VK_COMPARE_OP_EQUAL, VK_COMPARE_OP_LESS_OR_EQUAL, VK_COMPARE_OP_GREATER, VK_COMPARE_OP_NOT_EQUAL, VK_COMPARE_OP_GREATER_OR_EQUAL, VK_COMPARE_OP_ALWAYS };
		constexpr VkCullModeFlags CULL_MODE_MAP[3] = { VK_CULL_MODE_NONE, VK_CULL_MODE_FRONT_BIT, VK_CULL_MODE_BACK_BIT };
		
		const VkDeviceSize UNIFORM_ALIGNMENT = this->physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;

		const uint32_t currentLayoutIndex = this->dataDescriptorSetLayouts.size();

		// Load shader modules
		VkShaderModule	vertModule = this->CreateShaderModule(vertexShader),
						fragModule = this->CreateShaderModule(fragmentShader);

		// Create reflection data
		SpirvReflection vertReflection = ReflectSpirv(vertexShader),
						fragReflection = ReflectSpirv(fragmentShader);

		/* -------------------------------- 1. Data Descriptors -------------------------------- */
		
		// Collect descriptor set layouts bindings
		std::vector<std::vector<VkDescriptorSetLayoutBinding>>	dataLayoutsBindings = vertReflection.GetDescriptorSetLayoutBindings(SpirvReflection::DescriptorType::Block, VK_SHADER_STAGE_VERTEX_BIT),
																fragDataLayoutsBindings = fragReflection.GetDescriptorSetLayoutBindings(SpirvReflection::DescriptorType::Block, VK_SHADER_STAGE_FRAGMENT_BIT);
		dataLayoutsBindings.insert(dataLayoutsBindings.end(), fragDataLayoutsBindings.begin(), fragDataLayoutsBindings.end());

		std::vector<SpirvReflection::DescriptorSetLayout>	dataSets = vertReflection.GetDescriptorSetLayouts(SpirvReflection::DescriptorType::Block),
															fragSets = fragReflection.GetDescriptorSetLayouts(SpirvReflection::DescriptorType::Block);	
		dataSets.insert(dataSets.end(), fragSets.begin(), fragSets.end());

		uint32_t datalayoutCount = static_cast<uint32_t>(dataLayoutsBindings.size());

		std::vector<VkDescriptorSetLayout> setLayouts(datalayoutCount);

		// Create the set layouts
		for (uint32_t i = 0; i < datalayoutCount; i++)
		{
			// Layout creation
			VkDescriptorSetLayout layout;
			VkDescriptorSetLayoutCreateInfo setInfo = Create::DescriptorSetLayoutCreateInfo(dataLayoutsBindings[i]);
			vkCreateDescriptorSetLayout(this->device, &setInfo, nullptr, &setLayouts[i]);
			
			// Set layout offsets
			uint32_t lastSize = (this->dataDescriptorSetLayoutOffsets.size() >= 1) ? this->dataDescriptorSetLayoutOffsets.back().back() : 0;
			std::vector<uint32_t> offsets(1, lastSize);
			for (const auto& binding : dataSets[i].bindings)
			{
				offsets.push_back(offsets.back() + PaddedSize(binding.GetSize(), UNIFORM_ALIGNMENT));
			}
			this->dataDescriptorSetLayoutOffsets.push_back(offsets);

			// Create and write descriptor sets
			for (uint32_t j = 0; j < this->state.framesInFlight; j++)
			{
				VkDescriptorSet descriptorSet = this->descriptorManager.AllocateSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, setLayouts[i]);
				this->dataDescriptorSets.push_back(descriptorSet);
				for (const auto& binding : dataSets[i].bindings)
				{
					VkDescriptorBufferInfo bufferInfo = Create::DescriptorBufferInfo(
						this->dataDescriptorSetBuffers[j].buffer, 0, binding.GetSize()
					);
					VkWriteDescriptorSet setWrite = Create::WriteDescriptorSet(
						descriptorSet, binding.binding, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, nullptr, &bufferInfo, nullptr
					);
					vkUpdateDescriptorSets(this->device, 1, &setWrite, 0, nullptr);
				}
			}
		}
		this->dataDescriptorSetLayouts.insert(this->dataDescriptorSetLayouts.end(), setLayouts.begin(), setLayouts.end());

		/* -------------------------------- 3. Sampler Descriptors -------------------------------- */

		// Collect sampler descriptor set layouts
		std::vector<SpirvReflection::DescriptorSetLayout>	samplerSets = vertReflection.GetDescriptorSetLayouts(SpirvReflection::DescriptorType::Sampler),
															fragSamplerSets = fragReflection.GetDescriptorSetLayouts(SpirvReflection::DescriptorType::Sampler);
		samplerSets.insert(samplerSets.end(), fragSamplerSets.begin(), fragSamplerSets.end());

		setLayouts.push_back(this->textureDescriptorSetLayout);

		/* -------------------------------- 3. Pipeline Creation -------------------------------- */

		// Get binding and attribute descriptions
		std::vector<VkVertexInputBindingDescription> bindingDescriptions = vertReflection.GetVertexBindings();
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = vertReflection.GetVertexAttributes();

		// Get push constant range
		VkPushConstantRange pushConstantRange = vertReflection.GetPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT);

		// Create the pipeline layout
		VkPipelineLayout pipelineLayout;
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = Create::PipelineLayoutCreateInfo(
			0, setLayouts.size(), setLayouts.data(), vertReflection.HasPushConstant() ? 1 : 0, vertReflection.HasPushConstant() ? &pushConstantRange : nullptr
		);
		CS_ASSERT(vkCreatePipelineLayout(this->device, &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS, "Failed to create pipeline layout!");
		this->pipelineLayouts.push_back(pipelineLayout);

		// We we always use dyanmic states, there is really no performance penalty for just viewports and scissors and it means we don't need to recreate pipelines when resizing
		constexpr std::array<VkDynamicState, 2> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		// Create the information required to build the pipeline
		PipelineBuilderInfo pipelineBuilderInfo = {};
		pipelineBuilderInfo.dynamicState = Create::PipelineDynamicStateCreateInfo(dynamicStates.size(), dynamicStates.data());
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
			nullptr, VK_FALSE, VK_FALSE, FILL_MODE_MAP[static_cast<size_t>(variant.fillMode)], CULL_MODE_MAP[static_cast<size_t>(variant.cullMode)],
			VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f
		);
		pipelineBuilderInfo.multisamplingInfo = Create::PipelineMultisampleStateCreateInfo(
			nullptr, this->state.msaaSamples, VK_TRUE, 1.0f, nullptr, VK_FALSE, VK_FALSE
		);
		pipelineBuilderInfo.colorBlendAttachment = Create::PipelineColorBlendAttachmentState(
			VK_TRUE, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		);
		pipelineBuilderInfo.depthStencilInfo = Create::PipelineDepthStencilStateCreateInfo(
			0,
			(variant.depthTestEnable) ? VK_TRUE : VK_FALSE,
			(variant.depthWriteEnable) ? VK_TRUE : VK_FALSE,
			DEPTH_COMPARE_MAP[static_cast<size_t>(variant.depthFunc)], VK_FALSE, VK_FALSE,
			{}, {}, 0.0f, 1.0f
		);
		pipelineBuilderInfo.pipelineLayout = pipelineLayout;

		// Assign the relevant descriptor handles
		std::vector<uint32_t> descriptorHandles(datalayoutCount);
		for (uint32_t i = 0; i < datalayoutCount; i++) descriptorHandles[i] = currentLayoutIndex + i;

		// Create the pipeline
		this->pipelines.emplace_back(pipelineLayout, this->CreatePipeline(pipelineBuilderInfo), descriptorHandles, datalayoutCount);

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
		
		// It should not be possible, but just in case
		CS_ASSERT(sizeof(uint32_t) == sizeof(float), "Somehow sizeof uint32_t and sizeof float don't match!");

		CS_ASSERT(indices.size()    % 3 == 0, "Invalid mesh indices! mesh has " + std::to_string(indices.size()) + " indices!");
		CS_ASSERT(vertices.size()   % 3 == 0, "Invalid mesh vertices! mesh has " + std::to_string(vertices.size()) + " vertices!");
		CS_ASSERT(normals.size()    % 3 == 0, "Invalid mesh normals! mesh has " + std::to_string(normals.size()) + " normals!");
		CS_ASSERT(textureUVs.size() % 2 == 0, "Invalid mesh texture UVs! mesh has " + std::to_string(textureUVs.size()) + " texture UVs!");
		// TODO assert for potential buffer overflow

		std::array<uint32_t, 5> offsets = {};
		offsets[0] = 0;
		offsets[1] = indices.size() * sizeof(uint32_t);
		offsets[2] = offsets[1] + vertices.size() * sizeof(uint32_t);
		offsets[3] = offsets[2] + normals.size() * sizeof(uint32_t);
		offsets[4] = offsets[3] + textureUVs.size() * sizeof(uint32_t);

		// Stage all data into one buffer, reduces allocations
		internal::Allocator::Buffer staging = this->allocator.CreateBuffer(sizeof(uint32_t) * offsets.back(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		staging.Fill(offsets[0], indices.data(),    indices.size()    * sizeof(uint32_t));
		staging.Fill(offsets[1], vertices.data(),   vertices.size()   * sizeof(uint32_t));
		staging.Fill(offsets[2], normals.data(),    normals.size()    * sizeof(uint32_t));
		staging.Fill(offsets[3], textureUVs.data(), textureUVs.size() * sizeof(uint32_t));


		this->uploadQueue.InstantSubmit([=](const internal::CommandQueue& cmd) {
			VkBufferCopy copyIndices    = Create::BufferCopy(offsets[0], this->indiceOffsets.back() * sizeof(uint32_t), indices.size() * sizeof(uint32_t));
			VkBufferCopy copyVertices   = Create::BufferCopy(offsets[1], this->offsets.back() * sizeof(float) * VERTICES_PER_INDEX, vertices.size() * sizeof(float));
			VkBufferCopy copyNormals    = Create::BufferCopy(offsets[2], this->offsets.back() * sizeof(float) * NORMALS_PER_INDEX, normals.size() * sizeof(float));
			VkBufferCopy copyTextureUVs = Create::BufferCopy(offsets[3], this->offsets.back() * sizeof(float) * UVS_PER_INDEX, textureUVs.size() * sizeof(float));
		
			cmd.CopyBuffer(staging.buffer, this->vertexBuffers[INDICES].buffer,     { copyIndices });
			cmd.CopyBuffer(staging.buffer, this->vertexBuffers[POSITION].buffer,    { copyVertices });
			cmd.CopyBuffer(staging.buffer, this->vertexBuffers[NORMALS].buffer,     { copyNormals });
			cmd.CopyBuffer(staging.buffer, this->vertexBuffers[TEXTURE_UVS].buffer, { copyTextureUVs });
		});

		uint32_t uniqueVerticeCount = static_cast<uint32_t>(vertices.size()) / VERTICES_PER_INDEX;
		uint32_t indicesOffset = static_cast<uint32_t>(indices.size());

		this->offsets.push_back(this->offsets.back() + uniqueVerticeCount);
		this->indiceOffsets.push_back(this->indiceOffsets.back() + indicesOffset);

		this->allocator.DestroyBuffer(staging);
	}
	void Renderer::RendererImpl::UploadTexture(const void* textureData, uint32_t width, uint32_t height, uint32_t channels, bool generateMipmaps)
	{
		constexpr VkFormat DEFAULT_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;

		const size_t imageSize = width * height * channels;
		const uint32_t mipLevels = generateMipmaps ? static_cast<uint32_t>(std::log2(std::max(width, height))) + 1 : 1;
		
		// Create samplers dynamically as required for mip levels
		for (uint32_t i = static_cast<uint32_t>(samplers.size()); i < mipLevels; i++)
		{
			VkSampler sampler;
			VkSamplerCreateInfo samplerInfo = Create::SamplerCreateInfo(
				0, VK_FILTER_LINEAR, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
				0.0f, VK_TRUE, this->physicalDeviceProperties.limits.maxSamplerAnisotropy, VK_FALSE, VK_COMPARE_OP_ALWAYS,
				0.0f, static_cast<float>(i), VK_BORDER_COLOR_INT_OPAQUE_BLACK
			);
			vkCreateSampler(this->device, &samplerInfo, nullptr, &sampler);
			this->samplers.push_back(sampler);
		}

		// Stage the image data
		internal::Allocator::Buffer staging = this->allocator.CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		staging.Fill(0, textureData, imageSize);

		// Create the image
		VkExtent3D extent = { width, height, 1 };
		VkImageViewCreateInfo imageViewInfo = Create::ImageViewCreateInfo(
			0, nullptr, VK_IMAGE_VIEW_TYPE_2D, DEFAULT_FORMAT,
			{ VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY },
			Create::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 1)
		);
		internal::Allocator::Image image = this->allocator.CreateImage(Create::ImageCreateInfo(
			nullptr, 0, VK_IMAGE_TYPE_2D, DEFAULT_FORMAT, extent,
			mipLevels, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED
		), imageViewInfo, VMA_MEMORY_USAGE_GPU_ONLY);

		// Check if format is blit compatible, if not then we cannot generate mipmaps
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(this->physicalDevice, DEFAULT_FORMAT, &formatProperties);
		CS_ASSERT(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT, "Format is not blit compatible");

		this->uploadTextureQueue.InstantSubmit([&](const internal::CommandQueue& cmd) {
			// Transition to blit compatible
			VkImageMemoryBarrier barrier = Create::ImageMemoryBarrier(
				0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image.image,
				Create::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 1)
			);
			vkCmdPipelineBarrier(cmd.commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			// Copy buffer to image
			VkBufferImageCopy region = Create::BufferImageCopy(
				0, 0, 0, Create::ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1), { 0, 0, 0 }, extent
			);
			vkCmdCopyBufferToImage(cmd.commandBuffer, staging.buffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			// Transition the image to a transfer destination
			barrier = Create::ImageMemoryBarrier(
				0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image.image,
				Create::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1)
			);

			uint32_t mipWidth = width, mipHeight = height;

			for (uint32_t i = 1; i < mipLevels; i++)
			{
				barrier.subresourceRange.baseMipLevel = i - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				vkCmdPipelineBarrier(cmd.commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

				VkImageBlit blit = Create::ImageBlit(
					Create::ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, i - 1, 0, 1),
					{ {
						{ 0, 0, 0 },
						{ static_cast<int32_t>(mipWidth), static_cast<int32_t>(mipHeight), 1 }
					} },
					Create::ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, i, 0, 1),
					{ {
						{ 0, 0, 0 },
						{ static_cast<int32_t>(std::max(mipWidth / 2, 1u)), static_cast<int32_t>(std::max(mipHeight / 2, 1u)), 1 }
					} }
				);

				vkCmdBlitImage(cmd.commandBuffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(cmd.commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

				mipWidth = std::max(mipWidth / 2, 1u);
				mipHeight = std::max(mipHeight / 2, 1u);
			}

			barrier.subresourceRange.baseMipLevel = mipLevels - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(cmd.commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		});

		VkDescriptorSet descriptorSet = this->descriptorManager.AllocateSet(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->textureDescriptorSetLayout);
		VkDescriptorImageInfo imageInfo = Create::DescriptorImageInfo(
			this->samplers[mipLevels - 1], image.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
		VkWriteDescriptorSet write = Create::WriteDescriptorSet(
			descriptorSet, 0, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &imageInfo, nullptr, nullptr
		);
		vkUpdateDescriptorSets(this->device, 1, &write, 0, nullptr);

		this->images.push_back(image);
		this->imageDescriptorSets.push_back(descriptorSet);
		this->allocator.DestroyBuffer(staging);
	}
}