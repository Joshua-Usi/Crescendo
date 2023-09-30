#include "RendererImpl.hpp"

#include "cs_std/algorithms.hpp"
#include <numeric>

namespace Crescendo
{
	uint32_t sampleCountMap(VkSampleCountFlagBits samples)
	{
		switch (samples)
		{
			case VK_SAMPLE_COUNT_64_BIT: return 64;
			case VK_SAMPLE_COUNT_32_BIT: return 32;
			case VK_SAMPLE_COUNT_16_BIT: return 16;
			case VK_SAMPLE_COUNT_8_BIT:  return 8;
			case VK_SAMPLE_COUNT_4_BIT:  return 4;
			case VK_SAMPLE_COUNT_2_BIT:  return 2;
		}
		return 1;
	}
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
			glfwGetFramebufferSize(static_cast<GLFWwindow*>(this->rendererInfo.window), &width, &height);
			glfwWaitEvents();
		}

		// Destroy old buffers
		this->swapChainDeletionQueue.flush();
		// Create new info, with new data
		BuilderInfo info = {};
		info.windowExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
		info.msaaSamples = this->rendererInfo.msaaSamples;
		info.shadowMapResolution = this->rendererInfo.shadowMapResolution;
		info.framesInFlight = this->rendererInfo.framesInFlight;
		// Rebuild dynamic data
		this->InitialiseFlightFrames(info);
		this->InitialiseSwapchain(info);
		this->InitialiseRenderpasses(info);
		this->InitialiseFramebuffers(info);

		this->rendererInfo.windowExtent = info.windowExtent;
		this->rendererInfo.msaaSamples = info.msaaSamples;
		this->rendererInfo.shadowMapResolution = info.shadowMapResolution;
		this->rendererInfo.framesInFlight = info.framesInFlight;
	}
	void Renderer::RendererImpl::UploadPipeline(const std::vector<uint8_t>& vertexShader, const std::vector<uint8_t>& fragmentShader, const PipelineVariant& variant)
	{
		/* -------------------------------- 0. Initialisation -------------------------------- */

		constexpr VkPolygonMode FILL_MODE_MAP[3] = { VK_POLYGON_MODE_FILL, VK_POLYGON_MODE_LINE, VK_POLYGON_MODE_POINT };
		constexpr VkCompareOp DEPTH_COMPARE_MAP[8] = { VK_COMPARE_OP_NEVER, VK_COMPARE_OP_LESS, VK_COMPARE_OP_EQUAL, VK_COMPARE_OP_LESS_OR_EQUAL, VK_COMPARE_OP_GREATER, VK_COMPARE_OP_NOT_EQUAL, VK_COMPARE_OP_GREATER_OR_EQUAL, VK_COMPARE_OP_ALWAYS };
		constexpr VkCullModeFlags CULL_MODE_MAP[3] = { VK_CULL_MODE_NONE, VK_CULL_MODE_FRONT_BIT, VK_CULL_MODE_BACK_BIT };

		// We we always use dynamic states, there is really no performance penalty for just viewports and scissors and it means we don't need to recreate pipelines when resizing
		constexpr std::array<VkDynamicState, 2> dynamicStates { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		
		const VkDeviceSize UNIFORM_ALIGNMENT = this->physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
		const uint32_t currentLayoutIndex = this->dataDescriptorSetLayouts.size();

		PipelineData pipelineData(ReflectSpirv(vertexShader), ReflectSpirv(fragmentShader), this->device.CreateShaderModule(vertexShader), this->device.CreateShaderModule(fragmentShader));

		/* -------------------------------- 1. Data Descriptors -------------------------------- */
		
		// Collect descriptor set layouts bindings
		std::vector<std::vector<VkDescriptorSetLayoutBinding>> dataLayoutsBindings = cs_std::combine(
			pipelineData.vertexReflection.GetDescriptorSetLayoutBindings(SpirvReflection::DescriptorType::Block, VK_SHADER_STAGE_VERTEX_BIT),
			pipelineData.fragmentReflection.GetDescriptorSetLayoutBindings(SpirvReflection::DescriptorType::Block, VK_SHADER_STAGE_FRAGMENT_BIT)
		);

		std::vector<SpirvReflection::DescriptorSetLayout> dataSets = cs_std::combine(
			pipelineData.vertexReflection.GetDescriptorSetLayouts(SpirvReflection::DescriptorType::Block),
			pipelineData.fragmentReflection.GetDescriptorSetLayouts(SpirvReflection::DescriptorType::Block)
		);

		uint32_t datalayoutCount = static_cast<uint32_t>(dataLayoutsBindings.size());

		std::vector<VkDescriptorSetLayout> setLayouts(datalayoutCount);

		// Create the set layouts
		for (uint32_t i = 0; i < datalayoutCount; i++)
		{
			// Layout creation
			setLayouts[i] = this->device.CreateDescriptorSetLayout(dataLayoutsBindings[i]);
			
			// Set layout offsets
			uint32_t lastSize = (this->dataDescriptorSetLayoutOffsets.size() >= 1) ? this->dataDescriptorSetLayoutOffsets.back().back() : 0;
			std::vector<uint32_t> offsets(1, lastSize);
			for (const auto& binding : dataSets[i].bindings)
			{
				offsets.push_back(offsets.back() + PaddedSize(binding.GetSize(), UNIFORM_ALIGNMENT));
			}
			this->dataDescriptorSetLayoutOffsets.push_back(offsets);

			// Create and write descriptor sets
			for (uint32_t j = 0; j < this->rendererInfo.framesInFlight; j++)
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
			};
		}
		this->dataDescriptorSetLayouts.insert(this->dataDescriptorSetLayouts.end(), setLayouts.begin(), setLayouts.end());

		/* -------------------------------- 3. Sampler Descriptors -------------------------------- */

		// Collect sampler descriptor set layouts
		// TODO make sure texture descriptor sets support vertex shaders
		std::vector<SpirvReflection::DescriptorSetLayout> samplerSets = cs_std::combine(
			pipelineData.vertexReflection.GetDescriptorSetLayouts(SpirvReflection::DescriptorType::Sampler),
			pipelineData.fragmentReflection.GetDescriptorSetLayouts(SpirvReflection::DescriptorType::Sampler)
		);

		// I feel like vulkan should complain here, but it literally doesn't!
		for (uint32_t i = 0; i < samplerSets.size(); i++) setLayouts.push_back(this->textureDescriptorSetLayout);

		/* -------------------------------- 3. Pipeline Creation -------------------------------- */

		// Get binding and attribute descriptions
		std::vector<VkVertexInputBindingDescription> bindingDescriptions = pipelineData.vertexReflection.GetVertexBindings();
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = pipelineData.vertexReflection.GetVertexAttributes();

		// Get push constant range
		std::vector<VkPushConstantRange> pushConstantRanges = pipelineData.vertexReflection.GetPushConstantRanges(VK_SHADER_STAGE_VERTEX_BIT);

		// Create the pipeline layout
		VkPipelineLayout pipelineLayout;
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = Create::PipelineLayoutCreateInfo(
			0, setLayouts, pushConstantRanges
		);
		CS_ASSERT(vkCreatePipelineLayout(this->device, &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS, "Failed to create pipeline layout!");
		this->pipelineLayouts.push_back(pipelineLayout);

		// Create the information required to build the pipeline
		internal::Device::PipelineBuilderInfo pipelineBuilderInfo = {};
		pipelineBuilderInfo.dynamicState = Create::PipelineDynamicStateCreateInfo(dynamicStates);
		pipelineBuilderInfo.renderPass = (variant.renderPass == PipelineVariant::RenderPass::Default) ? this->defaultRenderPass : this->shadowMapRenderPass;
		pipelineBuilderInfo.shaderStagesInfo.push_back(Create::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, pipelineData.vertexShader));
		pipelineBuilderInfo.shaderStagesInfo.push_back(Create::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, pipelineData.fragmentShader));
		pipelineBuilderInfo.vertexInputInfo = Create::PipelineVertexInputStateCreateInfo(bindingDescriptions, attributeDescriptions);
		pipelineBuilderInfo.inputAssemblyInfo = Create::PipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
		pipelineBuilderInfo.rasterizerInfo = Create::PipelineRasterizationStateCreateInfo(
			VK_FALSE, VK_FALSE, FILL_MODE_MAP[static_cast<size_t>(variant.fillMode)], CULL_MODE_MAP[static_cast<size_t>(variant.cullMode)],
			VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f
		);
		pipelineBuilderInfo.multisamplingInfo = Create::PipelineMultisampleStateCreateInfo(
			this->state.msaaSamples, VK_TRUE, 1.0f, nullptr, VK_FALSE, VK_FALSE
		);
		pipelineBuilderInfo.colorBlendAttachment = Create::PipelineColorBlendAttachmentState(
			VK_TRUE, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		);
		pipelineBuilderInfo.depthStencilInfo = Create::PipelineDepthStencilStateCreateInfo(
			0, variant.depthTestEnable, variant.depthWriteEnable,
			DEPTH_COMPARE_MAP[static_cast<size_t>(variant.depthFunc)],
			VK_FALSE, VK_FALSE, {}, {}
		);
		pipelineBuilderInfo.pipelineLayout = pipelineLayout;

		// Assign the relevant data descriptor handles
		std::vector<uint32_t> dataDescriptorHandles(datalayoutCount);
		for (uint32_t i = 0; i < datalayoutCount; i++) dataDescriptorHandles[i] = currentLayoutIndex + i;
		// Assign the relevant sampler descriptor handles
		std::vector<uint32_t> samplerDescriptorHandles(samplerSets.size());
		for (uint32_t i = 0; i < samplerSets.size(); i++) samplerDescriptorHandles[i] = currentLayoutIndex + datalayoutCount + i;

		// Create the pipeline
		this->pipelines.emplace_back(pipelineLayout, this->device.CreatePipeline(pipelineBuilderInfo), dataDescriptorHandles, samplerDescriptorHandles);

		pipelineData.Destroy(this->device);
	}
	void Renderer::RendererImpl::UploadMesh(const std::vector<float>& vertices, const std::vector<float>& normals, const std::vector<float>& textureUVs, const std::vector<float>& tangents, const std::vector<uint32_t>& indices)
	{
		constexpr size_t VERTICES_PER_INDEX = 3;
		constexpr size_t NORMALS_PER_INDEX = 3;
		constexpr size_t TANGENTS_PER_INDEX = 4;
		constexpr size_t UVS_PER_INDEX = 2;

		constexpr size_t INDICES = 0, POSITION = 1, NORMALS = 2, TANGENTS = 3, TEXTURE_UVS = 4;
		
		// It should not be possible, but just in case
		CS_ASSERT(sizeof(uint32_t) == sizeof(float), "Somehow sizeof uint32_t and sizeof float don't match!");

		CS_ASSERT(indices.size()    % 3 == 0, "Invalid mesh indices! mesh has " + std::to_string(indices.size()) + " indices!");
		CS_ASSERT(vertices.size()   % 3 == 0, "Invalid mesh vertices! mesh has " + std::to_string(vertices.size()) + " vertices!");
		CS_ASSERT(normals.size()    % 3 == 0, "Invalid mesh normals! mesh has " + std::to_string(normals.size()) + " normals!");
		CS_ASSERT(tangents.size()   % 4 == 0, "Invalid mesh tangents! mesh has " + std::to_string(tangents.size()) + " tangents!");
		CS_ASSERT(textureUVs.size() % 2 == 0, "Invalid mesh texture UVs! mesh has " + std::to_string(textureUVs.size()) + " texture UVs!");
		// TODO assert for potential buffer overflow

		std::array<uint32_t, 6> offsets = {};
		offsets[0] = 0;
		offsets[1] = indices.size()					* sizeof(uint32_t);
		offsets[2] = offsets[1] + vertices.size()	* sizeof(uint32_t);
		offsets[3] = offsets[2] + normals.size()	* sizeof(uint32_t);
		offsets[4] = offsets[3] + tangents.size()	* sizeof(uint32_t);
		offsets[5] = offsets[4] + textureUVs.size() * sizeof(uint32_t);

		// Stage all data into one buffer, reduces allocations
		internal::Allocator::Buffer staging = this->allocator.CreateBuffer(sizeof(uint32_t) * offsets.back(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		staging.Fill(offsets[0], indices.data(),    indices.size()    * sizeof(uint32_t));
		staging.Fill(offsets[1], vertices.data(),   vertices.size()   * sizeof(uint32_t));
		staging.Fill(offsets[2], normals.data(),    normals.size()    * sizeof(uint32_t));
		staging.Fill(offsets[3], tangents.data(),   tangents.size()   * sizeof(uint32_t));
		staging.Fill(offsets[4], textureUVs.data(), textureUVs.size() * sizeof(uint32_t));


		this->uploadQueue.InstantSubmit([=](const internal::CommandQueue& cmd) {
			VkBufferCopy copyIndices    = Create::BufferCopy(offsets[0], this->indiceOffsets.back() * sizeof(uint32_t), indices.size() * sizeof(uint32_t));
			VkBufferCopy copyVertices   = Create::BufferCopy(offsets[1], this->offsets.back() * sizeof(float) * VERTICES_PER_INDEX, vertices.size() * sizeof(float));
			VkBufferCopy copyNormals    = Create::BufferCopy(offsets[2], this->offsets.back() * sizeof(float) * NORMALS_PER_INDEX, normals.size() * sizeof(float));
			VkBufferCopy copyTangents   = Create::BufferCopy(offsets[3], this->offsets.back() * sizeof(float) * TANGENTS_PER_INDEX, tangents.size() * sizeof(float));
			VkBufferCopy copyTextureUVs = Create::BufferCopy(offsets[4], this->offsets.back() * sizeof(float) * UVS_PER_INDEX, textureUVs.size() * sizeof(float));
		
			cmd.CopyBuffer(staging.buffer, this->vertexBuffers[INDICES].buffer,     { copyIndices });
			cmd.CopyBuffer(staging.buffer, this->vertexBuffers[POSITION].buffer,    { copyVertices });
			cmd.CopyBuffer(staging.buffer, this->vertexBuffers[NORMALS].buffer,     { copyNormals });
			cmd.CopyBuffer(staging.buffer, this->vertexBuffers[TANGENTS].buffer,    { copyTangents });
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
		const uint32_t mipLevels = 1 + generateMipmaps ? static_cast<uint32_t>(std::log2(std::max(width, height))) : 0;
		
		// Create samplers dynamically as required for mip levels
		VkSamplerCreateInfo samplerInfo = Create::SamplerCreateInfo(
			VK_FILTER_LINEAR, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
			this->rendererInfo.anistropicFiltering, 1.0f
		);
		for (uint32_t i = static_cast<uint32_t>(samplers.size()); i < mipLevels; i++)
		{
			samplerInfo.maxLod = static_cast<float>(i);
			this->samplers.push_back(this->device.CreateSampler(samplerInfo));
		}

		// Stage the image data
		internal::Allocator::Buffer staging = this->allocator.CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		staging.Fill(0, textureData, imageSize);

		// Create the image
		VkExtent3D extent = { width, height, 1 };
		internal::Allocator::Image image = this->allocator.CreateImage(Create::ImageCreateInfo(
			VK_IMAGE_TYPE_2D, DEFAULT_FORMAT, extent,
			mipLevels, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED
		), VMA_MEMORY_USAGE_GPU_ONLY);

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