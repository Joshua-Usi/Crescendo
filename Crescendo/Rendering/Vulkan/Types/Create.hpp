#pragma once

#include "common.hpp"

#include "Volk/volk.h"

#include <concepts>
#include <cstddef>
#include <type_traits>

#define CS_CREATE_SET_CONCEPT_FIELD(field, concept_t, type, on_container, on_value)\
if constexpr (ContainerType<concept_t, type>) {\
	field = on_container;\
} else if constexpr (PointerType<concept_t, type*>) {\
	field = on_value;\
}

/// <summary>
/// Argument order is defined exactly as in the Vulkan API.
/// </summary>
CS_NAMESPACE_BEGIN::Vulkan::Create
{
	// Any container that has a data() function that returns a pointer to the first element and
	// a size() function that returns the number of elements
	// Preferably a container that has contiguous storage
	template<typename T, typename ValueType>
	concept ContainerType = requires(T a) {
		typename T::value_type;
		requires std::same_as<typename T::value_type, ValueType>;
		{ a.data() } -> std::same_as<ValueType*>;
		{ a.size() } -> std::same_as<std::size_t>;
	};
	template<typename T, typename ValueType>
	concept PointerType = requires(T a) {
		requires std::same_as<T, ValueType> || std::same_as<T, std::nullptr_t>;
	};
	template<typename T, typename ValueType>
	concept ValidType = requires(T a) {
		requires PointerType<T, ValueType*> || ContainerType<T, ValueType>;
	};

	template<ValidType<VkSemaphore> WaitSemaphores, ValidType<VkPipelineStageFlags> WaitDstStageMask, ValidType<VkCommandBuffer> CommandBuffers, ValidType<VkSemaphore> SignalSemaphores>
	inline constexpr VkSubmitInfo SubmitInfo(const WaitSemaphores& waitSemaphores, const WaitDstStageMask& waitDstStageMask, const CommandBuffers& commandBuffers, const SignalSemaphores& signalSemaphores) {
		VkSubmitInfo submitInfo = {};

		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		CS_CREATE_SET_CONCEPT_FIELD(submitInfo.waitSemaphoreCount, WaitSemaphores, VkSemaphore, static_cast<uint32_t>(waitSemaphores.size()), waitSemaphores != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(submitInfo.pWaitSemaphores, WaitSemaphores, VkSemaphore, waitSemaphores.data(), waitSemaphores);
		CS_CREATE_SET_CONCEPT_FIELD(submitInfo.pWaitDstStageMask, WaitDstStageMask, VkPipelineStageFlags, waitDstStageMask.data(), waitDstStageMask);
		CS_CREATE_SET_CONCEPT_FIELD(submitInfo.commandBufferCount, CommandBuffers, VkCommandBuffer, static_cast<uint32_t>(commandBuffers.size()), commandBuffers != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(submitInfo.pCommandBuffers, CommandBuffers, VkCommandBuffer, commandBuffers.data(), commandBuffers);
		CS_CREATE_SET_CONCEPT_FIELD(submitInfo.signalSemaphoreCount, SignalSemaphores, VkSemaphore, static_cast<uint32_t>(signalSemaphores.size()), signalSemaphores != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(submitInfo.pSignalSemaphores, SignalSemaphores, VkSemaphore, signalSemaphores.data(), signalSemaphores);

		return submitInfo;
	}
	template<ValidType<VkSemaphore> WaitSemaphores, ValidType<VkSwapchainKHR> Swapchains, ValidType<uint32_t> ImageIndices>
	inline constexpr VkPresentInfoKHR PresentInfoKHR(const WaitSemaphores& waitSemaphores, const Swapchains& swapchains, const ImageIndices& imageIndices) {
		VkPresentInfoKHR presentInfo = {};

		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		CS_CREATE_SET_CONCEPT_FIELD(presentInfo.waitSemaphoreCount, WaitSemaphores, VkSemaphore, static_cast<uint32_t>(waitSemaphores.size()), waitSemaphores != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(presentInfo.pWaitSemaphores, WaitSemaphores, VkSemaphore, waitSemaphores.data(), waitSemaphores);
		CS_CREATE_SET_CONCEPT_FIELD(presentInfo.swapchainCount, Swapchains, VkSwapchainKHR, static_cast<uint32_t>(swapchains.size()), swapchains != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(presentInfo.pSwapchains, Swapchains, VkSwapchainKHR, swapchains.data(), swapchains);
		CS_CREATE_SET_CONCEPT_FIELD(presentInfo.pImageIndices, ImageIndices, uint32_t, imageIndices.data(), imageIndices);
		presentInfo.pResults = nullptr;

		return presentInfo;
	}
	inline constexpr VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule module, const char* pName = "main", const VkSpecializationInfo* pSpecializationInfo = nullptr) {
		VkPipelineShaderStageCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.stage = stage;
		createInfo.module = module;
		createInfo.pName = pName;
		createInfo.pSpecializationInfo = pSpecializationInfo;

		return createInfo;
	}
	template<ValidType<VkImageView> Attachments>
	inline constexpr VkFramebufferCreateInfo FramebufferCreateInfo(VkRenderPass renderPass, const Attachments& pAttachments, VkExtent2D extent, uint32_t layers)
	{
		VkFramebufferCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.renderPass = renderPass;
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.attachmentCount, Attachments, VkImageView, static_cast<uint32_t>(pAttachments.size()), pAttachments != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.pAttachments, Attachments, VkImageView, pAttachments.data(), pAttachments);
		createInfo.width = extent.width;
		createInfo.height = extent.height;
		createInfo.layers = layers;

		return createInfo;
	}
	template<ValidType<VkVertexInputBindingDescription> VertexBindingDescriptions, ValidType<VkVertexInputAttributeDescription> VertexAttributeDescriptions>
	inline constexpr VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo(const VertexBindingDescriptions& pVertexBindingDescriptions, const VertexAttributeDescriptions& pVertexAttributeDescriptions)
	{
		VkPipelineVertexInputStateCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.vertexBindingDescriptionCount, VertexBindingDescriptions, VkVertexInputBindingDescription, static_cast<uint32_t>(pVertexBindingDescriptions.size()), pVertexBindingDescriptions != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.pVertexBindingDescriptions, VertexBindingDescriptions, VkVertexInputBindingDescription, pVertexBindingDescriptions.data(), pVertexBindingDescriptions);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.vertexAttributeDescriptionCount, VertexAttributeDescriptions, VkVertexInputAttributeDescription, static_cast<uint32_t>(pVertexAttributeDescriptions.size()), pVertexAttributeDescriptions != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.pVertexAttributeDescriptions, VertexAttributeDescriptions, VkVertexInputAttributeDescription, pVertexAttributeDescriptions.data(), pVertexAttributeDescriptions);

		return createInfo;
	}
	inline constexpr VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology topology, VkBool32 primitiveRestartEnable)
	{
		VkPipelineInputAssemblyStateCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.topology = topology;
		createInfo.primitiveRestartEnable = primitiveRestartEnable;

		return createInfo;
	}
	inline constexpr VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo()
	{
		VkPipelineViewportStateCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.viewportCount = 1;
		createInfo.pViewports = nullptr;
		createInfo.scissorCount = 1;
		createInfo.pScissors = nullptr;

		return createInfo;
	}
	template<ValidType<VkViewport> Viewports, ValidType<VkRect2D> Scissors>
	inline constexpr VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo(const Viewports& pViewports, const Scissors& pScissors)
	{
		VkPipelineViewportStateCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.viewportCount, Viewports, VkViewport, static_cast<uint32_t>(pViewports.size()), pViewports != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.pViewports, Viewports, VkViewport, pViewports.data(), pViewports);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.scissorCount, Scissors, VkRect2D, static_cast<uint32_t>(pScissors.size()), pScissors != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.pScissors, Scissors, VkRect2D, pScissors.data(), pScissors);

		return createInfo;
	}
	inline constexpr VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo(VkBool32 depthClampEnable, VkBool32 rasterizerDiscardEnable, VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontFace, VkBool32 depthBiasEnable, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor, float lineWidth)
	{
		VkPipelineRasterizationStateCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		createInfo.pNext = nullptr;
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
	inline constexpr VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo(VkSampleCountFlagBits rasterizationSamples, VkBool32 sampleShadingEnable, float minSampleShading, const VkSampleMask* pSampleMask, VkBool32 alphaToCoverageEnable, VkBool32 alphaToOneEnable)
	{
		VkPipelineMultisampleStateCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		createInfo.pNext = nullptr;
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
		VkPipelineColorBlendAttachmentState attachmentState = {};

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
	template<ValidType<VkPipelineColorBlendAttachmentState> Attachments>
	inline VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo(VkBool32 logicOpEnable, VkLogicOp logicOp, const Attachments& attachments, const std::array<float, 4>& blendConstants)
	{
		VkPipelineColorBlendStateCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.logicOpEnable = logicOpEnable;
		createInfo.logicOp = logicOp;
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.attachmentCount, Attachments, VkPipelineColorBlendAttachmentState, static_cast<uint32_t>(attachments.size()), attachments != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.pAttachments, Attachments, VkPipelineColorBlendAttachmentState, attachments.data(), attachments);
		//memcpy(createInfo.blendConstants, blendConstants.data(), sizeof(float) * blendConstants.size());
		createInfo.blendConstants[0] = blendConstants[0];
		createInfo.blendConstants[1] = blendConstants[1];
		createInfo.blendConstants[2] = blendConstants[2];
		createInfo.blendConstants[3] = blendConstants[3];

		return createInfo;
	}
	template<ValidType<VkDynamicState> DynamicStates>
	inline constexpr VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo(const DynamicStates& dynamicStates)
	{
		VkPipelineDynamicStateCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.dynamicStateCount, DynamicStates, VkDynamicState, static_cast<uint32_t>(dynamicStates.size()), dynamicStates != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.pDynamicStates, DynamicStates, VkDynamicState, dynamicStates.data(), dynamicStates);

		return createInfo;
	}
	template<ValidType<VkDescriptorSetLayout> SetLayouts, ValidType<VkPushConstantRange> PushConstantRanges>
	inline constexpr VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo(const SetLayouts& pSetLayouts, const PushConstantRanges& pPushConstantRanges)
	{
		VkPipelineLayoutCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.setLayoutCount, SetLayouts, VkDescriptorSetLayout, static_cast<uint32_t>(pSetLayouts.size()), pSetLayouts != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.pSetLayouts, SetLayouts, VkDescriptorSetLayout, pSetLayouts.data(), pSetLayouts);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.pushConstantRangeCount, PushConstantRanges, VkPushConstantRange, static_cast<uint32_t>(pPushConstantRanges.size()), pPushConstantRanges != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.pPushConstantRanges, PushConstantRanges, VkPushConstantRange, pPushConstantRanges.data(), pPushConstantRanges);

		return createInfo;
	}
	template<ValidType<VkPipelineShaderStageCreateInfo> PipelineShaderStageCreateInfo>
	inline constexpr VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo(const PipelineShaderStageCreateInfo& stages, const VkPipelineVertexInputStateCreateInfo* pVertexInputState, const VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyState, const VkPipelineTessellationStateCreateInfo* pTessellationState, const VkPipelineViewportStateCreateInfo* pViewportState, const VkPipelineRasterizationStateCreateInfo* pRasterizationState, const VkPipelineMultisampleStateCreateInfo* pMultisampleState, const VkPipelineDepthStencilStateCreateInfo* pDepthStencilState, const VkPipelineColorBlendStateCreateInfo* pColorBlendState, const VkPipelineDynamicStateCreateInfo* pDynamicState, VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpass, VkPipeline basePipelineHandle, int32_t basePipelineIndex)
	{
		VkGraphicsPipelineCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.stageCount, PipelineShaderStageCreateInfo, VkPipelineShaderStageCreateInfo, static_cast<uint32_t>(stages.size()), stages != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.pStages, PipelineShaderStageCreateInfo, VkPipelineShaderStageCreateInfo, stages.data(), stages);
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
		VkShaderModuleCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		return createInfo;
	}
	inline constexpr VkAttachmentDescription AttachmentDescription(VkFormat format, VkSampleCountFlagBits samples, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp, VkImageLayout initialLayout, VkImageLayout finalLayout)
	{
		VkAttachmentDescription description = {};

		description.flags = 0;
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
		VkAttachmentReference reference = {};

		reference.attachment = attachment;
		reference.layout = layout;

		return reference;
	}
	template<ValidType<VkAttachmentReference> InputAttachments, ValidType<VkAttachmentReference> ColorAttachments, ValidType<VkAttachmentReference> ResolveAttachments, ValidType<VkAttachmentReference> DepthStencilAttachment, ValidType<uint32_t> PreserveAttachments>
	inline constexpr VkSubpassDescription SubpassDescription(VkPipelineBindPoint pipelineBindPoint, const InputAttachments& pInputAttachments, const ColorAttachments& pColorAttachments, const ResolveAttachments& pResolveAttachments, const DepthStencilAttachment& pDepthStencilAttachment, const PreserveAttachments& pPreserveAttachments)
	{
		VkSubpassDescription description = {};

		description.flags = 0;
		description.pipelineBindPoint = pipelineBindPoint;
		CS_CREATE_SET_CONCEPT_FIELD(description.inputAttachmentCount, InputAttachments, VkAttachmentReference, static_cast<uint32_t>(pInputAttachments.size()), pInputAttachments != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(description.pInputAttachments, InputAttachments, VkAttachmentReference, pInputAttachments.data(), pInputAttachments);
		CS_CREATE_SET_CONCEPT_FIELD(description.colorAttachmentCount, ColorAttachments, VkAttachmentReference, static_cast<uint32_t>(pColorAttachments.size()), pColorAttachments != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(description.pColorAttachments, ColorAttachments, VkAttachmentReference, pColorAttachments.data(), pColorAttachments);
		CS_CREATE_SET_CONCEPT_FIELD(description.pResolveAttachments, ResolveAttachments, VkAttachmentReference, pResolveAttachments.data(), pResolveAttachments);
		CS_CREATE_SET_CONCEPT_FIELD(description.pDepthStencilAttachment, DepthStencilAttachment, VkAttachmentReference, pDepthStencilAttachment.data(), pDepthStencilAttachment);
		CS_CREATE_SET_CONCEPT_FIELD(description.preserveAttachmentCount, PreserveAttachments, uint32_t, static_cast<uint32_t>(pPreserveAttachments.size()), pPreserveAttachments != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(description.pPreserveAttachments, PreserveAttachments, uint32_t, pPreserveAttachments.data(),	pPreserveAttachments);

		return description;
	}
	template<ValidType<VkSubpassDescription> Subpasses, ValidType<VkAttachmentDescription> Attachments, ValidType<VkSubpassDependency> Dependencies>
	inline constexpr VkRenderPassCreateInfo RenderPassCreateInfo(const Attachments& pAttachments, const Subpasses& pSubpasses, const Dependencies& pDependencies)
	{
		VkRenderPassCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.attachmentCount, Attachments, VkAttachmentDescription, static_cast<uint32_t>(pAttachments.size()), pAttachments != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.pAttachments, Attachments, VkAttachmentDescription, pAttachments.data(), pAttachments);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.subpassCount, Subpasses, VkSubpassDescription, static_cast<uint32_t>(pSubpasses.size()), pSubpasses != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.pSubpasses, Subpasses, VkSubpassDescription, pSubpasses.data(), pSubpasses);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.dependencyCount, Dependencies, VkSubpassDependency, static_cast<uint32_t>(pDependencies.size()), pDependencies != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.pDependencies, Dependencies, VkSubpassDependency, pDependencies.data(), pDependencies);

		return createInfo;
	}
	inline constexpr VkSubpassDependency SubpassDependency(uint32_t srcSubpass, uint32_t dstSubpass, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkDependencyFlags dependencyFlags
	) {
		VkSubpassDependency dependency = {};

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
		VkCommandPoolCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		createInfo.queueFamilyIndex = queueFamilyIndex;

		return createInfo;
	}
	inline constexpr VkBufferCreateInfo BufferCreateInfo(const void* pNext, VkBufferCreateFlags flags, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t* pQueueFamilyIndices)
	{
		VkBufferCreateInfo createInfo = {};

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
		VkMemoryAllocateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		createInfo.pNext = pNext;
		createInfo.allocationSize = allocationSize;
		createInfo.memoryTypeIndex = memoryTypeIndex;

		return createInfo;
	}
	inline constexpr VkImageCreateInfo ImageCreateInfo(VkImageType imageType, VkFormat format, VkExtent3D extent, uint32_t mipLevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE	, uint32_t queueFamilyIndexCount = 0, const uint32_t* pQueueFamilyIndices = nullptr, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED)
	{
		VkImageCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
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
		VkCommandBufferAllocateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.commandPool = commandPool;
		createInfo.level = level;
		createInfo.commandBufferCount = commandBufferCount;

		return createInfo;
	}
	inline constexpr VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags)
	{
		VkCommandBufferBeginInfo createInfo = {};
		
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = flags;
		createInfo.pInheritanceInfo = nullptr;

		return createInfo;
	}
	inline constexpr VkImageMemoryBarrier ImageMemoryBarrier(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex, VkImage image, VkImageSubresourceRange subresourceRange)
	{
		VkImageMemoryBarrier barrier = {};

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
		VkBufferImageCopy copyRegion = {};

		copyRegion.bufferOffset = bufferOffset;
		copyRegion.bufferRowLength = bufferRowLength;
		copyRegion.bufferImageHeight = bufferImageHeight;
		copyRegion.imageSubresource = imageSubresource;
		copyRegion.imageOffset = imageOffset;
		copyRegion.imageExtent = imageExtent;

		return copyRegion;
	}
	inline constexpr VkImageViewCreateInfo ImageViewCreateInfo(VkImage image, VkImageViewType viewType, VkFormat format, VkComponentMapping components, VkImageSubresourceRange subresourceRange)
	{
		VkImageViewCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.image = image;
		createInfo.viewType = viewType;
		createInfo.format = format;
		createInfo.components = components;
		createInfo.subresourceRange = subresourceRange;

		return createInfo;
	}
	inline constexpr VkSamplerCreateInfo SamplerCreateInfo(VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressMode, float maxAnisotropy, float maxLod, VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE)
	{
		VkSamplerCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.magFilter = magFilter;
		createInfo.minFilter = minFilter;
		createInfo.mipmapMode = mipmapMode;
		createInfo.addressModeU = addressMode;
		createInfo.addressModeV = addressMode;
		createInfo.addressModeW = addressMode;
		createInfo.mipLodBias = 0.0f;
		createInfo.anisotropyEnable = maxAnisotropy > 1.0f;
		createInfo.maxAnisotropy = maxAnisotropy;
		createInfo.compareEnable = VK_FALSE;
		createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		createInfo.minLod = 0.0f;
		createInfo.maxLod = maxLod;
		createInfo.borderColor = borderColor;
		createInfo.unnormalizedCoordinates = VK_FALSE;

		return createInfo;
	}
	inline constexpr VkSemaphoreCreateInfo SemaphoreCreateInfo(const void* pNext = nullptr)
	{
		VkSemaphoreCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		createInfo.pNext = pNext;
		createInfo.flags = 0;

		return createInfo;
	}
	inline constexpr VkFenceCreateInfo FenceCreateInfo(bool signaled)
	{
		VkFenceCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = (signaled) ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

		return createInfo;
	}
	inline constexpr VkRenderPassBeginInfo RenderPassBeginInfo(VkRenderPass renderPass, VkFramebuffer framebuffer, VkRect2D renderArea, uint32_t clearValueCount, const VkClearValue* pClearValues)
	{
		VkRenderPassBeginInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		createInfo.pNext = nullptr;
		createInfo.renderPass = renderPass;
		createInfo.framebuffer = framebuffer;
		createInfo.renderArea = renderArea;
		createInfo.clearValueCount = clearValueCount;
		createInfo.pClearValues = pClearValues;

		return createInfo;
	}
	inline constexpr VkViewport Viewport(float x, float y, float width, float height, float minDepth, float maxDepth)
	{
		VkViewport viewport = {};

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
		VkRect2D rect2D = {};

		rect2D.offset = offset;
		rect2D.extent = extent;

		return rect2D;
	}
	inline constexpr VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount, VkDescriptorType descriptorType, const VkDescriptorImageInfo* pImageInfo, const VkDescriptorBufferInfo* pBufferInfo, const VkBufferView* pTexelBufferView)
	{
		VkWriteDescriptorSet descriptorSet = {};

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
	inline constexpr VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount, VkDescriptorType descriptorType, const VkDescriptorImageInfo* pImageInfo)
	{
		return WriteDescriptorSet(dstSet, dstBinding, dstArrayElement, descriptorCount, descriptorType, pImageInfo, nullptr, nullptr);
	}
	inline constexpr VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount, VkDescriptorType descriptorType, const VkDescriptorBufferInfo* pBufferInfo)
	{
		return WriteDescriptorSet(dstSet, dstBinding, dstArrayElement, descriptorCount, descriptorType, nullptr, pBufferInfo, nullptr);
	}
	inline constexpr VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount, VkDescriptorType descriptorType, const VkBufferView* pTexelBufferView)
	{
		return WriteDescriptorSet(dstSet, dstBinding, dstArrayElement, descriptorCount, descriptorType, nullptr, nullptr, pTexelBufferView);
	}
	inline constexpr VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags, const VkSampler* pImmutableSamplers = nullptr)
	{
		VkDescriptorSetLayoutBinding layoutBinding = {};

		layoutBinding.binding = binding;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = descriptorCount;
		layoutBinding.stageFlags = stageFlags;
		layoutBinding.pImmutableSamplers = pImmutableSamplers;

		return layoutBinding;
	}
	template<ValidType<VkDescriptorSetLayoutBinding> Bindings>
	inline constexpr VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(const Bindings& pBindings)
	{
		VkDescriptorSetLayoutCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.bindingCount, Bindings, VkDescriptorSetLayoutBinding, static_cast<uint32_t>(pBindings.size()), pBindings != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.pBindings, Bindings, VkDescriptorSetLayoutBinding, pBindings.data(), pBindings);

		return createInfo;
	}
	inline constexpr VkDescriptorPoolSize DescriptorPoolSize(VkDescriptorType type, uint32_t descriptorCount)
	{
		VkDescriptorPoolSize descriptorPoolSize = {};

		descriptorPoolSize.type = type;
		descriptorPoolSize.descriptorCount = descriptorCount;

		return descriptorPoolSize;
	}
	template<ValidType<VkDescriptorPoolSize> PoolSizes>
	inline constexpr VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo(VkDescriptorPoolCreateFlags flags, uint32_t maxSets, const PoolSizes& pPoolSizes)
	{
		VkDescriptorPoolCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = flags;
		createInfo.maxSets = maxSets;
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.poolSizeCount, PoolSizes, VkDescriptorPoolSize, static_cast<uint32_t>(pPoolSizes.size()), pPoolSizes != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(createInfo.pPoolSizes, PoolSizes, VkDescriptorPoolSize, pPoolSizes.data(), pPoolSizes);

		return createInfo;
	}
	template<ValidType<VkDescriptorSetLayout> SetLayouts>
	inline constexpr VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo(VkDescriptorPool descriptorPool, const SetLayouts& pSetLayouts)
	{
		VkDescriptorSetAllocateInfo allocateInfo = {};

		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.pNext = nullptr;
		allocateInfo.descriptorPool = descriptorPool;
		CS_CREATE_SET_CONCEPT_FIELD(allocateInfo.descriptorSetCount, SetLayouts, VkDescriptorSetLayout, static_cast<uint32_t>(pSetLayouts.size()), pSetLayouts != nullptr);
		CS_CREATE_SET_CONCEPT_FIELD(allocateInfo.pSetLayouts, SetLayouts, VkDescriptorSetLayout, pSetLayouts.data(), pSetLayouts);

		return allocateInfo;
	
	}
	inline constexpr VkDescriptorBufferInfo DescriptorBufferInfo(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range)
	{
		VkDescriptorBufferInfo bufferInfo = {};

		bufferInfo.buffer = buffer;
		bufferInfo.offset = offset;
		bufferInfo.range = range;

		return bufferInfo;
	}
	inline constexpr VkDescriptorImageInfo DescriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout)
	{
		VkDescriptorImageInfo imageInfo = {};

		imageInfo.sampler = sampler;
		imageInfo.imageView = imageView;
		imageInfo.imageLayout = imageLayout;

		return imageInfo;
	}
	inline constexpr VkVertexInputBindingDescription VertexInputBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate)
	{
		VkVertexInputBindingDescription description = {};

		description.binding = binding;
		description.stride = stride;
		description.inputRate = inputRate;

		return description;
	}
	inline constexpr VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo(VkPipelineDepthStencilStateCreateFlags flags, VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp depthCompareOp, VkBool32 depthBoundsTestEnable, VkBool32 stencilTestEnable, VkStencilOpState front, VkStencilOpState back, float minDepthBounds = 0.0f, float maxDepthBounds = 1.0f)
	{
		VkPipelineDepthStencilStateCreateInfo createInfo = {};

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
		VkVertexInputAttributeDescription description = {};

		description.location = location;
		description.binding = binding;
		description.format = format;
		description.offset = offset;

		return description;
	}
	inline constexpr VkPushConstantRange PushConstantRange(VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size)
	{
		VkPushConstantRange pushConstantRange = {};

		pushConstantRange.stageFlags = stageFlags;
		pushConstantRange.offset = offset;
		pushConstantRange.size = size;
		
		return pushConstantRange;
	}
	inline constexpr VkBufferCopy BufferCopy(VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize size)
	{
		VkBufferCopy bufferCopy = {};
		
		bufferCopy.srcOffset = srcOffset;
		bufferCopy.dstOffset = dstOffset;
		bufferCopy.size = size;

		return bufferCopy;
	}
	inline constexpr VkImageSubresourceRange ImageSubresourceRange(VkImageAspectFlags aspectMask, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount)
	{
		VkImageSubresourceRange subresourceRange = {};

		subresourceRange.aspectMask = aspectMask;
		subresourceRange.baseMipLevel = baseMipLevel;
		subresourceRange.levelCount = levelCount;
		subresourceRange.baseArrayLayer = baseArrayLayer;
		subresourceRange.layerCount = layerCount;

		return subresourceRange;
	}
	inline constexpr VkImageBlit ImageBlit(VkImageSubresourceLayers srcSubresource, const VkOffset3D srcOffset0, const VkOffset3D srcOffset1, VkImageSubresourceLayers dstSubresource, const VkOffset3D dstOffset0, const VkOffset3D dstOffset1)
	{
		VkImageBlit imageBlit = {};

		imageBlit.srcSubresource = srcSubresource;
		imageBlit.srcOffsets[0] = srcOffset0;
		imageBlit.srcOffsets[1] = srcOffset1;
		imageBlit.dstSubresource = dstSubresource;
		imageBlit.dstOffsets[0] = dstOffset0;
		imageBlit.dstOffsets[1] = dstOffset1;

		return imageBlit;
	}
	inline constexpr VkImageSubresourceLayers ImageSubresourceLayers(VkImageAspectFlags aspectMask, uint32_t mipLevel, uint32_t baseArrayLayer, uint32_t layerCount)
	{
		VkImageSubresourceLayers subresourceLayers = {};

		subresourceLayers.aspectMask = aspectMask;
		subresourceLayers.mipLevel = mipLevel;
		subresourceLayers.baseArrayLayer = baseArrayLayer;
		subresourceLayers.layerCount = layerCount;

		return subresourceLayers;
	}
	inline constexpr VkExtent3D Extent3D(uint32_t width, uint32_t height, uint32_t depth)
	{
		return { width, height, depth };
	}
	inline constexpr VkOffset3D Offset3D(int32_t x, int32_t y, int32_t z)
	{
		return { x, y, z };
	}
	inline constexpr VkClearValue DefaultDepthClear(float clearValue = 0.0f)
	{
		VkClearValue depthClear = {};
		depthClear.depthStencil.depth = clearValue;
		return depthClear;
	}
}