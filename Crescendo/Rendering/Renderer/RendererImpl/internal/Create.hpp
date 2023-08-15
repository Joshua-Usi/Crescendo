#pragma once

#include "vulkan/vulkan.h"

#include <array>
#include <vector>

/// <summary>
/// Argument order is defined exactly as in the Vulkan API.
/// </summary>
namespace Crescendo::internal::Create
{
	inline constexpr VkSubmitInfo SubmitInfo(uint32_t waitSemaphoreCount, const VkSemaphore* pWaitSemaphores, const VkPipelineStageFlags* pWaitDstStageMask, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers, uint32_t signalSemaphoreCount, const VkSemaphore* pSignalSemaphores)
	{
		VkSubmitInfo submitInfo;

		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = waitSemaphoreCount;
		submitInfo.pWaitSemaphores = pWaitSemaphores;
		submitInfo.pWaitDstStageMask = pWaitDstStageMask;
		submitInfo.commandBufferCount = commandBufferCount;
		submitInfo.pCommandBuffers = pCommandBuffers;
		submitInfo.signalSemaphoreCount = signalSemaphoreCount;
		submitInfo.pSignalSemaphores = pSignalSemaphores;

		return submitInfo;
	}
	inline constexpr VkPresentInfoKHR PresentInfoKHR(const void* pNext, uint32_t waitSemaphoreCount, const VkSemaphore* pWaitSemaphores, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const uint32_t* pImageIndices, VkResult* pResults)
	{
		VkPresentInfoKHR presentInfo;

		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = pNext;
		presentInfo.waitSemaphoreCount = waitSemaphoreCount;
		presentInfo.pWaitSemaphores = pWaitSemaphores;
		presentInfo.swapchainCount = swapchainCount;
		presentInfo.pSwapchains = pSwapchains;
		presentInfo.pImageIndices = pImageIndices;
		presentInfo.pResults = pResults;

		return presentInfo;
	}
	inline constexpr VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(const void* pNext, VkPipelineShaderStageCreateFlags flags, VkShaderStageFlagBits stage, VkShaderModule module, const char* pName, const VkSpecializationInfo* pSpecializationInfo)
	{
		VkPipelineShaderStageCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		createInfo.pNext = pNext;
		createInfo.flags = flags;
		createInfo.stage = stage;
		createInfo.module = module;
		createInfo.pName = pName;
		createInfo.pSpecializationInfo = pSpecializationInfo;

		return createInfo;
	}
	inline constexpr VkFramebufferCreateInfo FramebufferCreateInfo(const void* pNext, VkFramebufferCreateFlags flags, VkRenderPass renderPass, uint32_t attachmentCount, const VkImageView* pAttachments, VkExtent2D extent, uint32_t layers)
	{
		VkFramebufferCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.pNext = pNext;
		createInfo.flags = flags;
		createInfo.renderPass = renderPass;
		createInfo.attachmentCount = attachmentCount;
		createInfo.pAttachments = pAttachments;
		createInfo.width = extent.width;
		createInfo.height = extent.height;
		createInfo.layers = layers;

		return createInfo;
	}
	inline constexpr VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo(const void* pNext, uint32_t vertexBindingDescriptionCount, const VkVertexInputBindingDescription* pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount, const VkVertexInputAttributeDescription* pVertexAttributeDescriptions)
	{
		VkPipelineVertexInputStateCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		createInfo.pNext = pNext;
		createInfo.flags = 0;
		createInfo.vertexBindingDescriptionCount = vertexBindingDescriptionCount;
		createInfo.pVertexBindingDescriptions = pVertexBindingDescriptions;
		createInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptionCount;
		createInfo.pVertexAttributeDescriptions = pVertexAttributeDescriptions;

		return createInfo;
	}
	inline constexpr VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology topology, VkBool32 primitiveRestartEnable)
	{
		VkPipelineInputAssemblyStateCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.topology = topology;
		createInfo.primitiveRestartEnable = primitiveRestartEnable;

		return createInfo;
	}
	inline constexpr VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo(const void* pNext, uint32_t viewportCount, const VkViewport* pViewports, uint32_t scissorCount, const VkRect2D* pScissors)
	{
		VkPipelineViewportStateCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		createInfo.pNext = pNext;
		createInfo.flags = 0;
		createInfo.viewportCount = viewportCount;
		createInfo.pViewports = pViewports;
		createInfo.scissorCount = scissorCount;
		createInfo.pScissors = pScissors;

		return createInfo;
	}
	inline constexpr VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo(const void* pNext, VkBool32 depthClampEnable, VkBool32 rasterizerDiscardEnable, VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontFace, VkBool32 depthBiasEnable, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor, float lineWidth)
	{
		VkPipelineRasterizationStateCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		createInfo.pNext = pNext;
		createInfo.flags = 0;
		createInfo.depthClampEnable = depthClampEnable;
		createInfo.rasterizerDiscardEnable = rasterizerDiscardEnable;
		createInfo.polygonMode = polygonMode;
		createInfo.cullMode = cullMode;
		createInfo.frontFace = frontFace;
		createInfo.depthBiasEnable = depthBiasEnable;
		createInfo.depthBiasConstantFactor = depthBiasConstantFactor;
		createInfo.depthBiasClamp = depthBiasClamp;
		createInfo.depthBiasSlopeFactor = depthBiasSlopeFactor;
		createInfo.lineWidth = lineWidth;

		return createInfo;
	}
	inline constexpr VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo(const void* pNext, VkSampleCountFlagBits rasterizationSamples, VkBool32 sampleShadingEnable, float minSampleShading, const VkSampleMask* pSampleMask, VkBool32 alphaToCoverageEnable, VkBool32 alphaToOneEnable)
	{
		VkPipelineMultisampleStateCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		createInfo.pNext = pNext;
		createInfo.flags = 0;
		createInfo.rasterizationSamples = rasterizationSamples;
		createInfo.sampleShadingEnable = sampleShadingEnable;
		createInfo.minSampleShading = minSampleShading;
		createInfo.pSampleMask = pSampleMask;
		createInfo.alphaToCoverageEnable = alphaToCoverageEnable;
		createInfo.alphaToOneEnable = alphaToOneEnable;

		return createInfo;
	}
	inline constexpr VkPipelineColorBlendAttachmentState PipelineColorBlendAttachmentState(VkBool32 blendEnable, VkBlendFactor srcColorBlendFactor, VkBlendFactor dstColorBlendFactor, VkBlendOp colorBlendOp, VkBlendFactor srcAlphaBlendFactor, VkBlendFactor dstAlphaBlendFactor, VkBlendOp alphaBlendOp, VkColorComponentFlags colorWriteMask)
	{
		VkPipelineColorBlendAttachmentState attachmentState;

		attachmentState.blendEnable = blendEnable;
		attachmentState.srcColorBlendFactor = srcColorBlendFactor;
		attachmentState.dstColorBlendFactor = dstColorBlendFactor;
		attachmentState.colorBlendOp = colorBlendOp;
		attachmentState.srcAlphaBlendFactor = srcAlphaBlendFactor;
		attachmentState.dstAlphaBlendFactor = dstAlphaBlendFactor;
		attachmentState.alphaBlendOp = alphaBlendOp;
		attachmentState.colorWriteMask = colorWriteMask;

		return attachmentState;
	}
	inline VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo(const void* pNext, VkPipelineColorBlendStateCreateFlags flags, VkBool32 logicOpEnable, VkLogicOp logicOp, uint32_t attachmentCount, const VkPipelineColorBlendAttachmentState* pAttachments, const std::array<float, 4>& blendConstants)
	{
		VkPipelineColorBlendStateCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		createInfo.pNext = pNext;
		createInfo.flags = flags;
		createInfo.logicOpEnable = logicOpEnable;
		createInfo.logicOp = logicOp;
		createInfo.attachmentCount = attachmentCount;
		createInfo.pAttachments = pAttachments;
		memcpy(createInfo.blendConstants, blendConstants.data(), sizeof(float) * blendConstants.size());

		return createInfo;
	}
	inline constexpr VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo(uint32_t dynamicStateCount, const VkDynamicState* pDynamicStates)
	{
		VkPipelineDynamicStateCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.dynamicStateCount = dynamicStateCount;
		createInfo.pDynamicStates = pDynamicStates;

		return createInfo;
	}
	inline constexpr VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo(VkPipelineLayoutCreateFlags flags, uint32_t setLayoutCount, const VkDescriptorSetLayout* pSetLayouts, uint32_t pushConstantRangeCount, const VkPushConstantRange* pPushConstantRanges)
	{
		VkPipelineLayoutCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = flags;
		createInfo.setLayoutCount = setLayoutCount;
		createInfo.pSetLayouts = pSetLayouts;
		createInfo.pushConstantRangeCount = pushConstantRangeCount;
		createInfo.pPushConstantRanges = pPushConstantRanges;

		return createInfo;
	}
	inline constexpr VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo(const void* pNext, VkPipelineCreateFlags flags, uint32_t stageCount, const VkPipelineShaderStageCreateInfo* pStages, const VkPipelineVertexInputStateCreateInfo* pVertexInputState, const VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyState, const VkPipelineTessellationStateCreateInfo* pTessellationState, const VkPipelineViewportStateCreateInfo* pViewportState, const VkPipelineRasterizationStateCreateInfo* pRasterizationState, const VkPipelineMultisampleStateCreateInfo* pMultisampleState, const VkPipelineDepthStencilStateCreateInfo* pDepthStencilState, const VkPipelineColorBlendStateCreateInfo* pColorBlendState, const VkPipelineDynamicStateCreateInfo* pDynamicState, VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpass, VkPipeline basePipelineHandle, int32_t basePipelineIndex)
	{
		VkGraphicsPipelineCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		createInfo.pNext = pNext;
		createInfo.flags = flags;
		createInfo.stageCount = stageCount;
		createInfo.pStages = pStages;
		createInfo.pVertexInputState = pVertexInputState;
		createInfo.pInputAssemblyState = pInputAssemblyState;
		createInfo.pTessellationState = pTessellationState;
		createInfo.pViewportState = pViewportState;
		createInfo.pRasterizationState = pRasterizationState;
		createInfo.pMultisampleState = pMultisampleState;
		createInfo.pDepthStencilState = pDepthStencilState;
		createInfo.pColorBlendState = pColorBlendState;
		createInfo.pDynamicState = pDynamicState;
		createInfo.layout = layout;
		createInfo.renderPass = renderPass;
		createInfo.subpass = subpass;
		createInfo.basePipelineHandle = basePipelineHandle;
		createInfo.basePipelineIndex = basePipelineIndex;

		return createInfo;
	}
	inline VkShaderModuleCreateInfo ShaderModuleCreateInfo(const std::vector<uint8_t>& code)
	{
		VkShaderModuleCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		return createInfo;
	}
	inline constexpr VkAttachmentDescription AttachmentDescription(VkAttachmentDescriptionFlags flags, VkFormat format, VkSampleCountFlagBits samples, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp, VkImageLayout initialLayout, VkImageLayout finalLayout)
	{
		VkAttachmentDescription description;

		description.flags = flags;
		description.format = format;
		description.samples = samples;
		description.loadOp = loadOp;
		description.storeOp = storeOp;
		description.stencilLoadOp = stencilLoadOp;
		description.stencilStoreOp = stencilStoreOp;
		description.initialLayout = initialLayout;
		description.finalLayout = finalLayout;

		return description;
	}
	inline constexpr VkAttachmentReference AttachmentReference(uint32_t attachment, VkImageLayout layout)
	{
		VkAttachmentReference reference;

		reference.attachment = attachment;
		reference.layout = layout;

		return reference;
	}
	inline constexpr VkSubpassDescription SubpassDescription(VkSubpassDescriptionFlags flags, VkPipelineBindPoint pipelineBindPoint, uint32_t inputAttachmentCount, const VkAttachmentReference* pInputAttachments, uint32_t colorAttachmentCount, const VkAttachmentReference* pColorAttachments, const VkAttachmentReference* pResolveAttachments, const VkAttachmentReference* pDepthStencilAttachment, uint32_t preserveAttachmentCount, const uint32_t* pPreserveAttachments)
	{
		VkSubpassDescription description;

		description.flags = flags;
		description.pipelineBindPoint = pipelineBindPoint;
		description.inputAttachmentCount = inputAttachmentCount;
		description.pInputAttachments = pInputAttachments;
		description.colorAttachmentCount = colorAttachmentCount;
		description.pColorAttachments = pColorAttachments;
		description.pResolveAttachments = pResolveAttachments;
		description.pDepthStencilAttachment = pDepthStencilAttachment;
		description.preserveAttachmentCount = preserveAttachmentCount;
		description.pPreserveAttachments = pPreserveAttachments;

		return description;
	}
	inline constexpr VkRenderPassCreateInfo RenderPassCreateInfo(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& subpassDependencies)
	{
		VkRenderPassCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		createInfo.pAttachments = attachments.data();
		createInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
		createInfo.pSubpasses = subpasses.data();
		createInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
		createInfo.pDependencies = subpassDependencies.data();

		return createInfo;
	}
	inline constexpr VkSubpassDependency SubpassDependency(uint32_t srcSubpass, uint32_t dstSubpass, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkDependencyFlags dependencyFlags
	) {
		VkSubpassDependency dependency;

		dependency.srcSubpass = srcSubpass;
		dependency.dstSubpass = dstSubpass;
		dependency.srcStageMask = srcStageMask;
		dependency.dstStageMask = dstStageMask;
		dependency.srcAccessMask = srcAccessMask;
		dependency.dstAccessMask = dstAccessMask;
		dependency.dependencyFlags = dependencyFlags;

		return dependency;
	}
	inline constexpr VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex)
	{
		VkCommandPoolCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		createInfo.queueFamilyIndex = queueFamilyIndex;

		return createInfo;
	}
	inline constexpr VkBufferCreateInfo BufferCreateInfo(const void* pNext, VkBufferCreateFlags flags, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t* pQueueFamilyIndices)
	{
		VkBufferCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.pNext = pNext;
		createInfo.flags = flags;
		createInfo.size = size;
		createInfo.usage = usage;
		createInfo.sharingMode = sharingMode;
		createInfo.queueFamilyIndexCount = queueFamilyIndexCount;
		createInfo.pQueueFamilyIndices = pQueueFamilyIndices;

		return createInfo;
	}
	inline constexpr VkMemoryAllocateInfo MemoryAllocateInfo(const void* pNext, VkDeviceSize allocationSize, uint32_t memoryTypeIndex)
	{
		VkMemoryAllocateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		createInfo.pNext = pNext;
		createInfo.allocationSize = allocationSize;
		createInfo.memoryTypeIndex = memoryTypeIndex;

		return createInfo;
	}
	inline constexpr VkImageCreateInfo ImageCreateInfo(const void* pNext, VkImageCreateFlags flags, VkImageType imageType, VkFormat format, VkExtent3D extent, uint32_t mipLevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t* pQueueFamilyIndices, VkImageLayout initialLayout)
	{
		VkImageCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.pNext = pNext;
		createInfo.flags = flags;
		createInfo.imageType = imageType;
		createInfo.format = format;
		createInfo.extent = extent;
		createInfo.mipLevels = mipLevels;
		createInfo.arrayLayers = arrayLayers;
		createInfo.samples = samples;
		createInfo.tiling = tiling;
		createInfo.usage = usage;
		createInfo.sharingMode = sharingMode;
		createInfo.queueFamilyIndexCount = queueFamilyIndexCount;
		createInfo.pQueueFamilyIndices = pQueueFamilyIndices;
		createInfo.initialLayout = initialLayout;

		return createInfo;
	}
	inline constexpr VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool commandPool, VkCommandBufferLevel level, uint32_t commandBufferCount)
	{
		VkCommandBufferAllocateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.commandPool = commandPool;
		createInfo.level = level;
		createInfo.commandBufferCount = commandBufferCount;

		return createInfo;
	}
	inline constexpr VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags)
	{
		VkCommandBufferBeginInfo createInfo;
		
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = flags;
		createInfo.pInheritanceInfo = nullptr;

		return createInfo;
	}
	inline constexpr VkImageMemoryBarrier ImageMemoryBarrier(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex, VkImage image, VkImageSubresourceRange subresourceRange)
	{
		VkImageMemoryBarrier barrier;

		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;

		barrier.srcAccessMask = srcAccessMask;
		barrier.dstAccessMask = dstAccessMask;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
		barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
		barrier.image = image;
		barrier.subresourceRange = subresourceRange;

		return barrier;
	}
	inline constexpr VkBufferImageCopy BufferImageCopy(VkDeviceSize bufferOffset, uint32_t bufferRowLength, uint32_t bufferImageHeight, VkImageSubresourceLayers imageSubresource, VkOffset3D imageOffset, VkExtent3D imageExtent)
	{
		VkBufferImageCopy copyRegion;

		copyRegion.bufferOffset = bufferOffset;
		copyRegion.bufferRowLength = bufferRowLength;
		copyRegion.bufferImageHeight = bufferImageHeight;
		copyRegion.imageSubresource = imageSubresource;
		copyRegion.imageOffset = imageOffset;
		copyRegion.imageExtent = imageExtent;

		return copyRegion;
	}
	inline constexpr VkImageViewCreateInfo ImageViewCreateInfo(const void* pNext, VkImageViewCreateFlags flags, VkImage image, VkImageViewType viewType, VkFormat format, VkComponentMapping components, VkImageSubresourceRange subresourceRange)
	{
		VkImageViewCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.pNext = pNext;
		createInfo.flags = flags;
		createInfo.image = image;
		createInfo.viewType = viewType;
		createInfo.format = format;
		createInfo.components = components;
		createInfo.subresourceRange = subresourceRange;

		return createInfo;
	}
	inline constexpr VkSamplerCreateInfo SamplerCreateInfo(const void* pNext, VkSamplerCreateFlags flags, VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV, VkSamplerAddressMode addressModeW, float mipLodBias, VkBool32 anisotropyEnable, float maxAnisotropy, VkBool32 compareEnable, VkCompareOp compareOp, float minLod, float maxLod, VkBorderColor borderColor, VkBool32 unnormalizedCoordinates)
	{
		VkSamplerCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		createInfo.pNext = pNext;
		createInfo.flags = flags;
		createInfo.magFilter = magFilter;
		createInfo.minFilter = minFilter;
		createInfo.mipmapMode = mipmapMode;
		createInfo.addressModeU = addressModeU;
		createInfo.addressModeV = addressModeV;
		createInfo.addressModeW = addressModeW;
		createInfo.mipLodBias = mipLodBias;
		createInfo.anisotropyEnable = anisotropyEnable;
		createInfo.maxAnisotropy = maxAnisotropy;
		createInfo.compareEnable = compareEnable;
		createInfo.compareOp = compareOp;
		createInfo.minLod = minLod;
		createInfo.maxLod = maxLod;
		createInfo.borderColor = borderColor;
		createInfo.unnormalizedCoordinates = unnormalizedCoordinates;

		return createInfo;
	}
	inline constexpr VkSemaphoreCreateInfo SemaphoreCreateInfo(const void* pNext = nullptr)
	{
		VkSemaphoreCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		createInfo.pNext = pNext;
		createInfo.flags = 0;

		return createInfo;
	}
	inline constexpr VkFenceCreateInfo FenceCreateInfo(bool signaled)
	{
		VkFenceCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = (signaled) ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

		return createInfo;
	}
	inline constexpr VkRenderPassBeginInfo RenderPassBeginInfo(const void* pNext, VkRenderPass renderPass, VkFramebuffer framebuffer, VkRect2D renderArea, uint32_t clearValueCount, const VkClearValue* pClearValues)
	{
		VkRenderPassBeginInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		createInfo.pNext = pNext;
		createInfo.renderPass = renderPass;
		createInfo.framebuffer = framebuffer;
		createInfo.renderArea = renderArea;
		createInfo.clearValueCount = clearValueCount;
		createInfo.pClearValues = pClearValues;

		return createInfo;
	}
	inline constexpr VkViewport Viewport(float x, float y, float width, float height, float minDepth, float maxDepth)
	{
		VkViewport viewport;

		viewport.x = x;
		viewport.y = y;
		viewport.width = width;
		viewport.height = height;
		viewport.minDepth = minDepth;
		viewport.maxDepth = maxDepth;

		return viewport;
	}
	inline constexpr VkRect2D Rect2D(VkOffset2D offset, VkExtent2D extent)
	{
		VkRect2D rect2D;

		rect2D.offset = offset;
		rect2D.extent = extent;

		return rect2D;
	}
	inline constexpr VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount, VkDescriptorType descriptorType, const VkDescriptorImageInfo* pImageInfo, const VkDescriptorBufferInfo* pBufferInfo, const VkBufferView* pTexelBufferView)
	{
		VkWriteDescriptorSet descriptorSet;

		descriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorSet.pNext = nullptr;
		descriptorSet.dstSet = dstSet;
		descriptorSet.dstBinding = dstBinding;
		descriptorSet.dstArrayElement = dstArrayElement;
		descriptorSet.descriptorCount = descriptorCount;
		descriptorSet.descriptorType = descriptorType;
		descriptorSet.pImageInfo = pImageInfo;
		descriptorSet.pBufferInfo = pBufferInfo;
		descriptorSet.pTexelBufferView = pTexelBufferView;

		return descriptorSet;
	}
	inline constexpr VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags, const VkSampler* pImmutableSamplers)
	{
		VkDescriptorSetLayoutBinding layoutBinding;

		layoutBinding.binding = binding;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = descriptorCount;
		layoutBinding.stageFlags = stageFlags;
		layoutBinding.pImmutableSamplers = pImmutableSamplers;

		return layoutBinding;
	}
	inline constexpr VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
	{
		VkDescriptorSetLayoutCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		createInfo.pBindings = bindings.data();

		return createInfo;
	}
	inline constexpr VkDescriptorPoolSize DescriptorPoolSize(VkDescriptorType type, uint32_t descriptorCount)
	{
		VkDescriptorPoolSize descriptorPoolSize;

		descriptorPoolSize.type = type;
		descriptorPoolSize.descriptorCount = descriptorCount;

		return descriptorPoolSize;
	}
	inline constexpr VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo(
		VkDescriptorPoolCreateFlags flags,
		uint32_t maxSets,
		const std::vector<VkDescriptorPoolSize>& poolSizes
	) {
		VkDescriptorPoolCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = flags;
		createInfo.maxSets = maxSets;
		createInfo.poolSizeCount = poolSizes.size();
		createInfo.pPoolSizes = poolSizes.data();

		return createInfo;
	}
	inline constexpr VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo(VkDescriptorPool descriptorPool, const std::vector<VkDescriptorSetLayout>& setLayouts) {
		VkDescriptorSetAllocateInfo allocateInfo;

		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.pNext = nullptr;
		allocateInfo.descriptorPool = descriptorPool;
		allocateInfo.descriptorSetCount = setLayouts.size();
		allocateInfo.pSetLayouts = setLayouts.data();

		return allocateInfo;
	}
	inline constexpr VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo(VkDescriptorPool descriptorPool, const VkDescriptorSetLayout& setLayout) {
		VkDescriptorSetAllocateInfo allocateInfo;

		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.pNext = nullptr;
		allocateInfo.descriptorPool = descriptorPool;
		allocateInfo.descriptorSetCount = 1;
		allocateInfo.pSetLayouts = &setLayout;

		return allocateInfo;
	}
	inline constexpr VkDescriptorBufferInfo DescriptorBufferInfo(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range)
	{
		VkDescriptorBufferInfo bufferInfo;

		bufferInfo.buffer = buffer;
		bufferInfo.offset = offset;
		bufferInfo.range = range;

		return bufferInfo;
	}
	inline constexpr VkDescriptorImageInfo DescriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout)
	{
		VkDescriptorImageInfo imageInfo;

		imageInfo.sampler = sampler;
		imageInfo.imageView = imageView;
		imageInfo.imageLayout = imageLayout;

		return imageInfo;
	}
	inline constexpr VkVertexInputBindingDescription VertexInputBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate)
	{
		VkVertexInputBindingDescription description;

		description.binding = binding;
		description.stride = stride;
		description.inputRate = inputRate;

		return description;
	}
	inline constexpr VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo(VkPipelineDepthStencilStateCreateFlags flags, VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp depthCompareOp, VkBool32 depthBoundsTestEnable, VkBool32 stencilTestEnable, VkStencilOpState front, VkStencilOpState back, float minDepthBounds, float maxDepthBounds)
	{
		VkPipelineDepthStencilStateCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = flags;
		createInfo.depthTestEnable = depthTestEnable;
		createInfo.depthWriteEnable = depthWriteEnable;
		createInfo.depthCompareOp = depthCompareOp;
		createInfo.depthBoundsTestEnable = depthBoundsTestEnable;
		createInfo.stencilTestEnable = stencilTestEnable;
		createInfo.front = front;
		createInfo.back = back;
		createInfo.minDepthBounds = minDepthBounds;
		createInfo.maxDepthBounds = maxDepthBounds;

		return createInfo;
	}
	inline constexpr VkVertexInputAttributeDescription VertexInputAttributeDescription(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset)
	{
		VkVertexInputAttributeDescription description;

		description.location = location;
		description.binding = binding;
		description.format = format;
		description.offset = offset;

		return description;
	}
	inline constexpr VkPushConstantRange PushConstantRange(VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size)
	{
		VkPushConstantRange pushConstantRange;

		pushConstantRange.stageFlags = stageFlags;
		pushConstantRange.offset = offset;
		pushConstantRange.size = size;
		
		return pushConstantRange;
	}
	inline constexpr VkBufferCopy BufferCopy(VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize size)
	{
		VkBufferCopy bufferCopy;
		
		bufferCopy.srcOffset = srcOffset;
		bufferCopy.dstOffset = dstOffset;
		bufferCopy.size = size;

		return bufferCopy;
	}
	inline constexpr VkImageSubresourceRange ImageSubresourceRange(VkImageAspectFlags aspectMask, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount)
	{
		VkImageSubresourceRange subresourceRange;

		subresourceRange.aspectMask = aspectMask;
		subresourceRange.baseMipLevel = baseMipLevel;
		subresourceRange.levelCount = levelCount;
		subresourceRange.baseArrayLayer = baseArrayLayer;
		subresourceRange.layerCount = layerCount;

		return subresourceRange;
	}
}