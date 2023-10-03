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
		this->InitialiseFramebuffers(info);

		this->rendererInfo.windowExtent = info.windowExtent;
		this->rendererInfo.msaaSamples = info.msaaSamples;
		this->rendererInfo.shadowMapResolution = info.shadowMapResolution;
		this->rendererInfo.framesInFlight = info.framesInFlight;
	}
	void Renderer::RendererImpl::UploadPipeline(const std::vector<uint8_t>& vertexShader, const std::vector<uint8_t>& fragmentShader, const std::vector<PipelineVariant>& variants)
	{
		constexpr VkPolygonMode FILL_MODE_MAP[3] = { VK_POLYGON_MODE_FILL, VK_POLYGON_MODE_LINE, VK_POLYGON_MODE_POINT };
		constexpr VkCompareOp DEPTH_COMPARE_MAP[8] = { VK_COMPARE_OP_NEVER, VK_COMPARE_OP_LESS, VK_COMPARE_OP_EQUAL, VK_COMPARE_OP_LESS_OR_EQUAL, VK_COMPARE_OP_GREATER, VK_COMPARE_OP_NOT_EQUAL, VK_COMPARE_OP_GREATER_OR_EQUAL, VK_COMPARE_OP_ALWAYS };
		constexpr VkCullModeFlags CULL_MODE_MAP[3] = { VK_CULL_MODE_NONE, VK_CULL_MODE_FRONT_BIT, VK_CULL_MODE_BACK_BIT };
		constexpr std::array<std::pair<const char*, ShaderAttributeFlag>, 12> ATTRIBUTE_MAP = {
			std::make_pair("iPosition", ShaderAttributeFlag::Position),
			std::make_pair("iNormal", ShaderAttributeFlag::Normal),
			std::make_pair("iTangent", ShaderAttributeFlag::Tangent),
			std::make_pair("iTexCoord", ShaderAttributeFlag::TexCoord_0), // Alternative name
			std::make_pair("iTexCoord0", ShaderAttributeFlag::TexCoord_0),
			std::make_pair("iTexCoord1", ShaderAttributeFlag::TexCoord_1),
			std::make_pair("iColor", ShaderAttributeFlag::Color_0), // Alternative name
			std::make_pair("iColor0", ShaderAttributeFlag::Color_0),
			std::make_pair("iJoints", ShaderAttributeFlag::Color_0), // Alternative name
			std::make_pair("iJoints0", ShaderAttributeFlag::Joints_0),
			std::make_pair("iWeights", ShaderAttributeFlag::Color_0), // Alternative name
			std::make_pair("iWeights0", ShaderAttributeFlag::Weights_0)
		};

		// We we always use dynamic states, there is really no performance penalty for just viewports and scissors and it means we don't need to recreate pipelines when resizing
		constexpr std::array<VkDynamicState, 2> dynamicStates { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		const VkDeviceSize UNIFORM_ALIGNMENT = this->physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
		const uint32_t currentLayoutIndex = this->dataDescriptorSetLayouts.size();

		/* -------------------------------- 0. Initialisation -------------------------------- */

		PipelineData pipelineData(ReflectSpirv(vertexShader), ReflectSpirv(fragmentShader), this->device.CreateShaderModule(vertexShader), this->device.CreateShaderModule(fragmentShader));

		std::vector<ShaderAttributeFlag> attributeFlags;
		// O(n^2) algo, not the fastest, but not the end of the world, since n is small
		for (const auto& inputVariable : pipelineData.vertexReflection.inputVariables)
		{
			for (const auto& attribute : ATTRIBUTE_MAP)
			{
				if (inputVariable.name == attribute.first)
				{
					attributeFlags.push_back(attribute.second);
					break;
				}
			}
		}

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
					this->device.WriteDescriptorSet(setWrite);
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
		VkPipelineLayout pipelineLayout = this->device.CreatePipelineLayout(setLayouts, pushConstantRanges);
		this->pipelineLayouts.push_back(pipelineLayout);

		for (const auto& variant : variants)
		{
			// Create the information required to build the pipeline
			internal::Device::PipelineBuilderInfo pipelineBuilderInfo = {};
			pipelineBuilderInfo.dynamicState = Create::PipelineDynamicStateCreateInfo(dynamicStates);
			pipelineBuilderInfo.renderPass = (variant.renderPass == PipelineVariant::RenderPass::Default) ? this->renderPasses[DEFAULT_RENDER_PASS] : this->renderPasses[SHADOW_RENDER_PASS];
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
			this->pipelines.emplace_back(pipelineLayout, this->device.CreatePipeline(pipelineBuilderInfo), dataDescriptorHandles, samplerDescriptorHandles, attributeFlags);
		}

		pipelineData.Destroy(this->device);
	}
	void Renderer::RendererImpl::UploadMesh(const std::vector<ShaderAttribute>& attributes, const std::vector<uint32_t>& indices)
	{
		// Elements per attribute
		constexpr size_t ELEMENTS_PER_ATTRIBUTE[static_cast<size_t>(ShaderAttributeFlag::SHADER_ATTRIBUTE_FLAG_COUNT)]{
			3, // POSITION
			3, // NORMAL
			4, // TANGENT
			2, // TEXCOORD0
			2, // TEXCOORD1
			4, // COLOR0
			4, // JOINTS0
			4  // WEIGHTS0
		};
		
		// It should not be possible, but just in case
		CS_ASSERT(sizeof(uint32_t) == sizeof(float), "Somehow sizeof uint32_t and sizeof float don't match!");

		for (const auto& attribute : attributes)
		{
			CS_ASSERT(attribute.data.size() % ELEMENTS_PER_ATTRIBUTE[static_cast<size_t>(attribute.attribute)] == 0, "Invalid mesh data! mesh has " + std::to_string(attribute.data.size()) + " elements! but expected a multiple of " + std::to_string(ELEMENTS_PER_ATTRIBUTE[static_cast<size_t>(attribute.attribute)]) + "!");
		}
		CS_ASSERT(indices.size() % 3 == 0, "Invalid mesh data! mesh has " + std::to_string(indices.size()) + " indices! but expected a multiple of 3!");
		// TODO assert for potential buffer overflow
		
		// Determine the buffer offsets
		std::vector<uint32_t> bufferOffsets(1, 0);
		bufferOffsets.push_back(bufferOffsets.back() + indices.size() * sizeof(uint32_t));
		for (const auto& attribute : attributes)
		{
			bufferOffsets.push_back(bufferOffsets.back() + attribute.data.size() * sizeof(float));
		}

		// Stage all data into one buffer, reduces allocations
		internal::Allocator::Buffer staging = this->allocator.CreateBuffer(bufferOffsets.back(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		staging.Fill(0, indices.data(), indices.size() * sizeof(uint32_t));
		for (size_t i = 0; i < attributes.size(); i++)
		{
			staging.Fill(bufferOffsets[i + 1], attributes[i].data.data(), attributes[i].data.size() * sizeof(float));
		}

		// Upload data to the GPU
		this->uploadQueue.InstantSubmit([&](const internal::CommandQueue& cmd) {
			// Copy index data
			VkBufferCopy copy = Create::BufferCopy(0, this->offsets[0].back() * sizeof(uint32_t) * 3, indices.size() * sizeof(uint32_t));
			cmd.CopyBuffer(staging.buffer, this->vertexBuffers[0].buffer, { copy });
			
			// Copy other vertex attributes
			for (size_t i = 0; i < attributes.size(); i++)
			{
				const size_t attributeIndex = static_cast<size_t>(attributes[i].attribute) + 1;
				VkBufferCopy copy = Create::BufferCopy(bufferOffsets[i + 1], this->offsets[attributeIndex].back() * sizeof(float) * ELEMENTS_PER_ATTRIBUTE[attributeIndex - 1], attributes[i].data.size() * sizeof(float));
				cmd.CopyBuffer(staging.buffer, this->vertexBuffers[attributeIndex].buffer, { copy });
			}
		});

		// Indice offsets
		this->offsets[0].push_back(this->offsets[0].back() + indices.size() / 3);

		// Other vertex attribute offsets
		for (size_t i = 0; i < attributes.size(); i++)
		{
			const uint32_t attributeIndex = static_cast<uint32_t>(attributes[i].attribute) + 1;
			this->offsets[attributeIndex].push_back(this->offsets[attributeIndex].back() + static_cast<uint32_t>(attributes[i].data.size()) / ELEMENTS_PER_ATTRIBUTE[attributeIndex - 1]);
		}
		// Even though some attributes may not be present, we still need to add the offsets, but it'll be 0, only need to add to attributes that haven't been pushed to
		for (size_t i = 1; i < this->offsets.size(); i++)
		{
			bool found = false;
			for (const auto& attribute : attributes)
			{
				if (static_cast<size_t>(attribute.attribute) + 1 == i)
				{
					found = true;
					break;
				}
			}
			if (!found) this->offsets[i].push_back(this->offsets[i].back());
		}

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
			cmd.CopyBufferToImage(staging.buffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, region);

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
		this->device.WriteDescriptorSet(write);

		this->images.push_back(image);
		this->imageDescriptorSets.push_back(descriptorSet);
		this->allocator.DestroyBuffer(staging);
	}
}