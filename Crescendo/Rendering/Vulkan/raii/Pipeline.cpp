#include "Pipeline.hpp"
#include "Volk/volk.h"
#include "../Create.hpp"
#include "ShaderModule.hpp"
#include "ShaderReflection.hpp"
#include "cs_std/algorithms.hpp"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	Pipeline::Pipeline() : device(nullptr), layout(nullptr), variants(), descriptorSetLayout(nullptr), renderPass(), pipelines() {}
	Pipeline::Pipeline(VkDevice device, const PipelineCreateInfo& createInfo) : device(device), descriptorSetLayout(descriptorSetLayout), variants(createInfo.variants), renderPass(createInfo.renderPass), pipelines()
	{
		/* ---------------------------------------------------------------- 0. Constant Data ---------------------------------------------------------------- */

		constexpr std::array<std::pair<const char*, cs_std::graphics::Attribute>, 12> ATTRIBUTE_MAP {
			std::make_pair("iPosition",		cs_std::graphics::Attribute::POSITION),
			std::make_pair("iNormal",		cs_std::graphics::Attribute::NORMAL),
			std::make_pair("iTangent",		cs_std::graphics::Attribute::TANGENT),
			std::make_pair("iTexCoord",		cs_std::graphics::Attribute::TEXCOORD_0), // Alternative name
			std::make_pair("iTexCoord0",	cs_std::graphics::Attribute::TEXCOORD_0),
			std::make_pair("iTexCoord1",	cs_std::graphics::Attribute::TEXCOORD_1),
			std::make_pair("iColor",		cs_std::graphics::Attribute::COLOR_0), // Alternative name
			std::make_pair("iColor0",		cs_std::graphics::Attribute::COLOR_0),
			std::make_pair("iJoints",		cs_std::graphics::Attribute::JOINTS_0), // Alternative name
			std::make_pair("iJoints0",		cs_std::graphics::Attribute::JOINTS_0),
			std::make_pair("iWeights",		cs_std::graphics::Attribute::WEIGHTS_0), // Alternative name
			std::make_pair("iWeights0",		cs_std::graphics::Attribute::WEIGHTS_0)
		};
		// We we always use dynamic states, there is really no performance penalty for just viewports and scissors and it means we don't need to recreate pipelines when resizing
		constexpr std::array<VkDynamicState, 2> dynamicStates { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		/* ---------------------------------------------------------------- 1. Shader Module and Reflection ---------------------------------------------------------------- */

		const ShaderModule	vertexModule(device, createInfo.vertexCode), fragmentModule(device, createInfo.fragmentCode);
		const ShaderReflection vertexReflection(createInfo.vertexCode), fragmentReflection(createInfo.fragmentCode);

		/* ---------------------------------------------------------------- 2 - Variant building ---------------------------------------------------------------- */

		// Get binding and attribute descriptions
		const std::vector<VkVertexInputBindingDescription> bindingDescriptions = vertexReflection.GenerateVertexBindings();
		const std::vector<VkVertexInputAttributeDescription> attributeDescriptions = vertexReflection.GenerateVertexAttributes();

		// Get push constant range
		const std::vector<VkPushConstantRange> pushConstantRanges = cs_std::combine(
			vertexReflection.GeneratePushConstantRanges(ShaderReflection::ShaderStage::Vertex),
			fragmentReflection.GeneratePushConstantRanges(ShaderReflection::ShaderStage::Fragment, vertexReflection.GetPushConstantSize())
		);

		// Create the pipeline layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = Create::PipelineLayoutCreateInfo(&descriptorSetLayout, pushConstantRanges);
		CS_ASSERT(vkCreatePipelineLayout(this->device, &pipelineLayoutInfo, nullptr, &layout) == VK_SUCCESS, "Failed to create pipeline layout!");

		// Cache stages
		std::vector<VkPipelineShaderStageCreateInfo> stages;
		if (createInfo.vertexCode.size() > 0) stages.push_back(Create::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertexModule));
		if (createInfo.fragmentCode.size() > 0) stages.push_back(Create::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentModule));

		for (const auto& variant : createInfo.variants)
		{

			VkPipeline pipeline = nullptr;
			constexpr std::array<float, 4> blendConstants{ 0.0f, 0.0f, 0.0f, 0.0f };
			const VkPipelineColorBlendAttachmentState colorBlendAttachment = Create::PipelineColorBlendAttachmentState(VK_TRUE, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
			const VkPipelineColorBlendStateCreateInfo colorBlending = Create::PipelineColorBlendStateCreateInfo(VK_FALSE, VK_LOGIC_OP_COPY, &colorBlendAttachment, blendConstants);
			const VkPipelineVertexInputStateCreateInfo vertexInputInfo = Create::PipelineVertexInputStateCreateInfo(bindingDescriptions, attributeDescriptions);
			const VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = Create::PipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
			const VkPipelineViewportStateCreateInfo viewportState = Create::PipelineViewportStateCreateInfo();
			const VkPipelineRasterizationStateCreateInfo rasterizerInfo = Create::PipelineRasterizationStateCreateInfo(VK_FALSE, VK_FALSE, PipelineVariants::GetPolygonMode(variant.fillModeFlags), PipelineVariants::GetCullMode(variant.cullModeFlags), VK_FRONT_FACE_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
			const VkPipelineMultisampleStateCreateInfo multisamplingInfo = Create::PipelineMultisampleStateCreateInfo(PipelineVariants::GetMultisamples(variant.multisampleFlags), VK_TRUE, 1.0f, nullptr, VK_FALSE, VK_FALSE);
			const VkPipelineDepthStencilStateCreateInfo depthStencilInfo = Create::PipelineDepthStencilStateCreateInfo(0, PipelineVariants::GetDepthTest(variant.depthTestFlags), PipelineVariants::GetDepthWrite(variant.depthWriteFlags), PipelineVariants::GetDepthFunc(variant.depthFuncFlags), VK_FALSE, VK_FALSE, {}, {}, 1.0f, 0.0f); // For reverse Z 
			const VkPipelineDynamicStateCreateInfo dynamicState = Create::PipelineDynamicStateCreateInfo(dynamicStates);

			const VkGraphicsPipelineCreateInfo pipelineInfo = Create::GraphicsPipelineCreateInfo(
				stages, &vertexInputInfo, &inputAssemblyInfo, nullptr, &viewportState, &rasterizerInfo, &multisamplingInfo,
				&depthStencilInfo, &colorBlending, &dynamicState, this->layout, createInfo.renderPass, 0, VK_NULL_HANDLE, 0
			);
			CS_ASSERT(vkCreateGraphicsPipelines(this->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) == VK_SUCCESS, "Failed to create pipeline!");
			this->pipelines.push_back(pipeline);
		}
	}
	Pipeline::~Pipeline()
	{
		if (device == nullptr) return;
		for (auto pipeline : pipelines) vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyPipelineLayout(device, layout, nullptr);
	}
	Pipeline::Pipeline(Pipeline&& other) noexcept
		: device(other.device), layout(other.layout), variants(other.variants), descriptorSetLayout(other.descriptorSetLayout),
		renderPass(other.renderPass), pipelines(std::move(other.pipelines))
	{
		other.device = nullptr;
	}
	Pipeline& Pipeline::operator=(Pipeline&& other) noexcept
	{
		if (this != &other)
		{
			device = other.device; other.device = nullptr;
			layout = other.layout;
			renderPass = other.renderPass;
			variants = other.variants;
			descriptorSetLayout = other.descriptorSetLayout;
			pipelines = std::move(other.pipelines);
		}
		return *this;
	}
	const PipelineVariants& Pipeline::GetVariants() const { return variants; }
	Pipeline::operator VkPipelineLayout() const { return layout; }
	Pipeline::operator VkPipeline() const { return pipelines[0]; }
	VkPipeline Pipeline::operator [](uint32_t index) const { return pipelines[index]; }
	VkDescriptorSetLayout Pipeline::GetDescriptorSetLayout() const { return descriptorSetLayout; }
}