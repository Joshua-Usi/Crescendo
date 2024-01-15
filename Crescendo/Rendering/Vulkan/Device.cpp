#include "Device.hpp"

#include "Types/Create.hpp"

#include "cs_std/graphics/model.hpp"
#include "cs_std/algorithms.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	size_t CalculatePaddedSizeForAlignment(size_t size, size_t alignmentRequirement)
	{
		return (alignmentRequirement > 0) ? (size + alignmentRequirement - 1) & ~(alignmentRequirement - 1) : size;
	}
	Device::Device(const vkb::Device& device, VkInstance instance, uint32_t descriptorSetsPerPool, uint32_t minUniformBufferOffsetAlignment)
		: allocator(Allocator(instance, device.physical_device, device)), descriptorManager(device, descriptorSetsPerPool),
		device(device.device), queues(device),
		minUniformBufferOffsetAlignment(minUniformBufferOffsetAlignment)
	{
		// Create universal descriptor sets
		this->fragmentSamplerSetLayout = this->CreateDescriptorSetLayout(Create::DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT));
		this->vertexSSBOSetLayout = this->CreateDescriptorSetLayout(Create::DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT));
		this->fragmentSSBOSetLayout = this->CreateDescriptorSetLayout(Create::DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT));
		this->directionalShadowMapSampler = this->CreateSampler(Create::SamplerCreateInfo(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, 1.0f, 1.0f, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE));
		this->postProcessingSampler = this->CreateSampler(Create::SamplerCreateInfo(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, 1.0f, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK));
	}
	Device::~Device()
	{
		if (this->device == nullptr) return;
		vkDeviceWaitIdle(this->device);
		// Destroy universal resources
		vkDestroyDescriptorSetLayout(this->device, this->fragmentSamplerSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(this->device, this->vertexSSBOSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(this->device, this->fragmentSSBOSetLayout, nullptr);
		vkDestroySampler(this->device, this->directionalShadowMapSampler, nullptr);
		vkDestroySampler(this->device, this->postProcessingSampler, nullptr);

		this->allocator.Destroy();
		this->descriptorManager.Destroy();
		vkDestroyDevice(this->device, nullptr);
	}
	Device::Device(Device&& other) noexcept
		: allocator(std::move(other.allocator)), descriptorManager(std::move(other.descriptorManager)),
		device(other.device), queues(other.queues),
		fragmentSamplerSetLayout(fragmentSamplerSetLayout), vertexSSBOSetLayout(vertexSSBOSetLayout), fragmentSSBOSetLayout(fragmentSSBOSetLayout), directionalShadowMapSampler(directionalShadowMapSampler), postProcessingSampler(postProcessingSampler),
		minUniformBufferOffsetAlignment(other.minUniformBufferOffsetAlignment)
	{
		other.device = nullptr;
		other.fragmentSamplerSetLayout = nullptr;
		other.vertexSSBOSetLayout = nullptr;
		other.fragmentSSBOSetLayout = nullptr;
		other.directionalShadowMapSampler = nullptr;
		other.postProcessingSampler = nullptr;

	}
	Device& Device::operator=(Device&& other) noexcept
	{
		if (this != &other)
		{
			this->allocator = std::move(other.allocator);
			this->descriptorManager = std::move(other.descriptorManager);
			this->device = other.device; other.device = nullptr;
			this->queues = other.queues;
			this->fragmentSamplerSetLayout = other.fragmentSamplerSetLayout; other.fragmentSamplerSetLayout = nullptr;
			this->vertexSSBOSetLayout = other.vertexSSBOSetLayout; other.vertexSSBOSetLayout = nullptr;
			this->fragmentSSBOSetLayout = other.fragmentSSBOSetLayout; other.fragmentSSBOSetLayout = nullptr;
			this->directionalShadowMapSampler = other.directionalShadowMapSampler; other.directionalShadowMapSampler = nullptr;
			this->postProcessingSampler = other.postProcessingSampler; other.postProcessingSampler = nullptr;
			this->minUniformBufferOffsetAlignment = other.minUniformBufferOffsetAlignment;
		}
		return *this;
	}
	Buffer Device::CreateBuffer(size_t allocationSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
	{
		return this->allocator.CreateBuffer(allocationSize, usage, memoryUsage);
	}
	Image Device::CreateImage(const VkImageCreateInfo& imageInfo, VmaMemoryUsage memoryUsage)
	{
		return this->allocator.CreateImage(imageInfo, memoryUsage);
	}
	GraphicsCommandQueue Device::CreateGraphicsCommandQueue()
	{
		return GraphicsCommandQueue(*this, this->queues.universal, true);
	}
	TransferCommandQueue Device::CreateTransferCommandQueue()
	{
		// Once I figure out how to do queue transfers, I'll switch to using the transfer specific queue
		return TransferCommandQueue(*this, this->queues.universal, false);
	}
	RenderPass Device::CreateRenderPass(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& subpassDependencies)
	{
		VkRenderPass renderPass = nullptr;
		const VkRenderPassCreateInfo renderPassInfo = Create::RenderPassCreateInfo(attachments, subpasses, subpassDependencies);
		CS_ASSERT(vkCreateRenderPass(this->device, &renderPassInfo, nullptr, &renderPass) == VK_SUCCESS, "Failed to create render pass");
		return RenderPass(this->device, renderPass);
	}
	RenderPass Device::CreateDefaultRenderPass(VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits samples)
	{
		bool isMultiSampling = samples != VK_SAMPLE_COUNT_1_BIT;

		VkAttachmentDescription colorAttachment = Create::AttachmentDescription(
			colorFormat, samples,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, (isMultiSampling) ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
		VkAttachmentReference colorAttachmentRef = Create::AttachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		VkAttachmentDescription depthAttachment = Create::AttachmentDescription(
			depthFormat, samples,
			VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);
		VkAttachmentReference depthAttachmentRef = Create::AttachmentReference(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		VkAttachmentDescription colorAttachmentResolve = Create::AttachmentDescription(
			colorFormat, VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
		VkAttachmentReference colorAttachmentResolveRef = Create::AttachmentReference((isMultiSampling) ? 2 : VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		const VkSubpassDescription subpass = Create::SubpassDescription(
			VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &colorAttachmentRef, (isMultiSampling) ? &colorAttachmentResolveRef : nullptr, &depthAttachmentRef, 0, nullptr
		);

		VkSubpassDependency colorDependency = Create::SubpassDependency(
			VK_SUBPASS_EXTERNAL, 0,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT
		);
		VkSubpassDependency colorDependency2 = Create::SubpassDependency(
			0, VK_SUBPASS_EXTERNAL,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT
		);
		VkSubpassDependency depthDependency = Create::SubpassDependency(
			VK_SUBPASS_EXTERNAL, 0,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT
		);

		std::vector attachments = { colorAttachment, depthAttachment };
		if (isMultiSampling) attachments.push_back(colorAttachmentResolve);

		return this->CreateRenderPass(attachments, { subpass }, { colorDependency, colorDependency2, depthDependency });
	}
	RenderPass Device::CreateDefaultShadowRenderPass(VkFormat depthFormat, VkSampleCountFlagBits samples)
	{
		VkAttachmentDescription shadowMapAttachment = Create::AttachmentDescription(
			depthFormat, samples,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
		);
		VkAttachmentReference shadowMapAttachmentRef = Create::AttachmentReference(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		VkSubpassDescription shadowMapSubpass = Create::SubpassDescription(
			VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, &shadowMapAttachmentRef, 0, nullptr
		);

		VkSubpassDependency shadowMapDependency = Create::SubpassDependency(
			VK_SUBPASS_EXTERNAL, 0,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		);
		VkSubpassDependency shadowMapDependency2 = Create::SubpassDependency(
			0, VK_SUBPASS_EXTERNAL,
			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		);

		return this->CreateRenderPass({ shadowMapAttachment }, { shadowMapSubpass }, { shadowMapDependency, shadowMapDependency2 });
	}
	RenderPass Device::CreateDefaultDepthPrePassRenderPass(VkFormat depthFormat, VkSampleCountFlagBits samples)
	{
		VkAttachmentDescription depthAttachment = Create::AttachmentDescription(
			depthFormat, samples,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);
		VkAttachmentReference depthAttachmentRef = Create::AttachmentReference(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		VkSubpassDescription depthSubpass = Create::SubpassDescription(
			VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, &depthAttachmentRef, 0, nullptr
		);

		VkSubpassDependency depthDependency = Create::SubpassDependency(
			VK_SUBPASS_EXTERNAL, 0,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		);
		VkSubpassDependency depthDependency2 = Create::SubpassDependency(
			0, VK_SUBPASS_EXTERNAL,
			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		);

		return this->CreateRenderPass({ depthAttachment }, { depthSubpass }, { depthDependency, depthDependency2 });
	}
	RenderPass Device::CreateDefaultPostProcessingRenderPass(VkFormat colorFormat, VkSampleCountFlagBits samples)
	{
		VkAttachmentDescription attachment = Create::AttachmentDescription(
			colorFormat, samples,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		);
		VkAttachmentReference attachmentRef = Create::AttachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		VkSubpassDescription subpass = Create::SubpassDescription(
			VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &attachmentRef, nullptr, nullptr, 0, nullptr
		);

		VkSubpassDependency dependency = Create::SubpassDependency(
			VK_SUBPASS_EXTERNAL, 0,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0
		);

		return this->CreateRenderPass({ attachment }, { subpass }, { dependency });
	}
	Framebuffer Device::CreateFramebuffer(VkRenderPass renderPass, const std::vector<VkImageView>& attachments, VkExtent2D extent, bool hasColorAttachment, bool hasDepthAttachment)
	{
		VkFramebuffer fb = nullptr;
		VkFramebufferCreateInfo framebufferInfo = Create::FramebufferCreateInfo(renderPass, attachments, extent, 1);
		CS_ASSERT(vkCreateFramebuffer(this->device, &framebufferInfo, nullptr, &fb) == VK_SUCCESS, "Failed to create framebuffer!");
		return Framebuffer(this->device, fb, renderPass, extent, hasColorAttachment, hasDepthAttachment);
	}
	ShaderModule Device::CreateShaderModule(const std::vector<uint8_t>& code)
	{
		if (code.size() == 0) return ShaderModule(this->device, nullptr);

		VkShaderModule shaderModule = nullptr;
		const VkShaderModuleCreateInfo createInfo = Create::ShaderModuleCreateInfo(code);
		CS_ASSERT(vkCreateShaderModule(this->device, &createInfo, nullptr, &shaderModule) == VK_SUCCESS, "Failed to create shader module!");
		return ShaderModule(this->device, shaderModule);
	}
	ShaderReflection Device::CreateShaderReflection(const std::vector<uint8_t>& code)
	{
		return ShaderReflection(code);
	}
	Pipelines Device::CreatePipelines(const std::vector<uint8_t>& vertexCode, const std::vector<uint8_t>& fragmentCode, const PipelineVariants& variant)
	{
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

		// TODO when an empty shader is passed in, don't create a module for it
		const ShaderModule	vertexModule = this->CreateShaderModule(vertexCode),
						fragmentModule = this->CreateShaderModule(fragmentCode);
		const ShaderReflection vertexReflection = this->CreateShaderReflection(vertexCode),
						 fragmentReflection = this->CreateShaderReflection(fragmentCode);

		/* ---------------------------------------------------------------- 0 - Determine input flags ---------------------------------------------------------------- */

		std::vector<cs_std::graphics::Attribute> vertexAttributes;
		// O(n^2) algo, not the fastest, but not the end of the world, since n is small
		for (const auto& inputVariable : vertexReflection.inputVariables)
		{
			for (const auto& attribute : ATTRIBUTE_MAP)
			{
				if (inputVariable.name == attribute.first)
				{
					vertexAttributes.push_back(attribute.second);
					break;
				}
			}
		}

		/* ---------------------------------------------------------------- 1 -Descriptor layouts ---------------------------------------------------------------- */

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		std::vector<Pipelines::Set> setData;

		auto separateDescriptors = [&](const ShaderReflection& reflection, uint32_t shaderStage, uint32_t setOffset)
		{
			for (const auto& set : reflection.descriptorSetLayouts)
			{
				switch (set.bindings[0].type)
				{
					case ShaderReflection::DescriptorType::Block:
					{
						std::vector<VkDescriptorSetLayoutBinding> binding = reflection.GetDescriptorSetLayoutBindings(shaderStage, set.set + setOffset);
						descriptorSetLayouts.push_back(this->CreateDescriptorSetLayout(binding));
						std::vector<Pipelines::Set::Binding> bindings;
						for (const auto& binding : set.bindings) bindings.emplace_back(binding.binding, CalculatePaddedSizeForAlignment(binding.GetSize(), this->minUniformBufferOffsetAlignment));
						setData.emplace_back(bindings, set.set, Pipelines::Set::DescriptorType::Block);
						break;
					}
					case ShaderReflection::DescriptorType::Storage:
					{
						descriptorSetLayouts.push_back(this->GetSSBOLayout(shaderStage));
						setData.emplace_back(std::vector<Pipelines::Set::Binding>(), set.set, Pipelines::Set::DescriptorType::Storage);
						break;
					}
					case ShaderReflection::DescriptorType::Sampler:
					{
						descriptorSetLayouts.push_back(this->GetFragmentSamplerLayout());
						setData.emplace_back(std::vector<Pipelines::Set::Binding>(), set.set, Pipelines::Set::DescriptorType::Sampler);
						break;
					}
				}
			}
		};

		separateDescriptors(vertexReflection, VK_SHADER_STAGE_VERTEX_BIT, 0);
		separateDescriptors(fragmentReflection, VK_SHADER_STAGE_FRAGMENT_BIT, -vertexReflection.descriptorSetLayouts.size());

		/* ---------------------------------------------------------------- 2 - Variant building ---------------------------------------------------------------- */

		// Get binding and attribute descriptions
		const std::vector<VkVertexInputBindingDescription> bindingDescriptions = vertexReflection.GetVertexBindings();
		const std::vector<VkVertexInputAttributeDescription> attributeDescriptions = vertexReflection.GetVertexAttributes();

		// Get push constant range
		const std::vector<VkPushConstantRange> pushConstantRanges = cs_std::combine(
			vertexReflection.GetPushConstantRanges(VK_SHADER_STAGE_VERTEX_BIT),
			fragmentReflection.GetPushConstantRanges(VK_SHADER_STAGE_FRAGMENT_BIT)
		);

		// Create the pipeline layout
		const VkPipelineLayout pipelineLayout = this->CreatePipelineLayout(descriptorSetLayouts, pushConstantRanges);

		// Generate each pipeline
		std::vector<VkPipeline> pipelines;
		
		// Cache stages
		std::vector<VkPipelineShaderStageCreateInfo> stages;
		if (vertexCode.size() > 0) stages.push_back(Create::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertexModule));
		if (fragmentCode.size() > 0) stages.push_back(Create::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentModule));

		for (const auto& thisVariant : variant)
		{
			const PipelineBuilderInfo pipelineBuilderInfo = {
				.dynamicState = Create::PipelineDynamicStateCreateInfo(dynamicStates),
				.shaderStagesInfo = stages,
				.vertexInputInfo = Create::PipelineVertexInputStateCreateInfo(bindingDescriptions, attributeDescriptions),
				.inputAssemblyInfo = Create::PipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE),
				.rasterizerInfo = Create::PipelineRasterizationStateCreateInfo(
					VK_FALSE, VK_FALSE, PipelineVariants::GetPolygonMode(thisVariant.fillModeFlags), PipelineVariants::GetCullMode(thisVariant.cullModeFlags),
					VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f
				),
				.multisamplingInfo = Create::PipelineMultisampleStateCreateInfo(
					PipelineVariants::GetMultisamples(thisVariant.multisampleFlags), VK_TRUE, 1.0f, nullptr, VK_FALSE, VK_FALSE
				),
				.colorBlendAttachment = Create::PipelineColorBlendAttachmentState(
					VK_TRUE, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD,
					VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
					VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
				),
				.depthStencilInfo = Create::PipelineDepthStencilStateCreateInfo(
					0, PipelineVariants::GetDepthTest(thisVariant.depthTestFlags), PipelineVariants::GetDepthWrite(thisVariant.depthWriteFlags),
					PipelineVariants::GetDepthFunc(thisVariant.depthFuncFlags),
					VK_FALSE, VK_FALSE, {}, {},
					1.0f, 0.0f // For reverse Z
				),
				.pipelineLayout = pipelineLayout,
				.renderPass = thisVariant.renderPass
			};

			pipelines.push_back(this->CreatePipeline(pipelineBuilderInfo));
		}

		return Pipelines(*this, pipelines, descriptorSetLayouts, setData, vertexAttributes, variant, pipelineLayout);
	}
	SSBO Device::CreateSSBO(size_t allocationSize, VkShaderStageFlags shaderStage, VmaMemoryUsage memoryUsage)
	{
		Vulkan::SSBO ssbo(
			this->CreateBuffer(allocationSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU),
			this->AllocateDescriptorSet(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, this->GetSSBOLayout(shaderStage)),
			allocationSize
		);
		VkDescriptorBufferInfo bufferInfo = Create::DescriptorBufferInfo(ssbo.buffer, 0, allocationSize);
		VkWriteDescriptorSet write = Create::WriteDescriptorSet(ssbo.set, 0, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &bufferInfo);
		this->WriteDescriptorSet(write);

		return ssbo;
	}
	VkDescriptorSet Device::CreateTextureDescriptorSet(VkSampler sampler, const Vulkan::Image& image, VkImageLayout layout)
	{
		const VkDescriptorSet set = this->AllocateDescriptorSet(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->GetFragmentSamplerLayout());
		const VkDescriptorImageInfo imageInfo = Create::DescriptorImageInfo(sampler, image.imageView, layout);
		const VkWriteDescriptorSet write = Create::WriteDescriptorSet(set, 0, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &imageInfo);
		this->WriteDescriptorSet(write);

		return set;
	}
	VkCommandPool Device::CreateCommandPool(uint32_t queueFamilyIndex)
	{
		VkCommandPool pool = nullptr;
		const VkCommandPoolCreateInfo poolInfo = Create::CommandPoolCreateInfo(queueFamilyIndex);
		CS_ASSERT(vkCreateCommandPool(this->device, &poolInfo, nullptr, &pool) == VK_SUCCESS, "Failed to create command pool!");
		return pool;
	}
	VkCommandBuffer Device::AllocateCommandBuffer(VkCommandPool commandPool, VkCommandBufferLevel level)
	{
		VkCommandBuffer cmd = nullptr;
		const VkCommandBufferAllocateInfo cmdBufferInfo = Create::CommandBufferAllocateInfo(commandPool, level, 1);
		CS_ASSERT(vkAllocateCommandBuffers(this->device, &cmdBufferInfo, &cmd) == VK_SUCCESS, "Failed to allocate command buffers!");
		return cmd;
	}
	VkFence Device::CreateFence(bool signaled)
	{
		VkFence fence = nullptr;
		const VkFenceCreateInfo fenceInfo = Create::FenceCreateInfo(signaled);
		CS_ASSERT(vkCreateFence(this->device, &fenceInfo, nullptr, &fence) == VK_SUCCESS, "Failed to create fence");
		return fence;
	}
	VkSemaphore Device::CreateSemaphore()
	{
		VkSemaphore semaphore = nullptr;
		const VkSemaphoreCreateInfo semaphoreInfo = Create::SemaphoreCreateInfo();
		CS_ASSERT(vkCreateSemaphore(this->device, &semaphoreInfo, nullptr, &semaphore) == VK_SUCCESS, "Failed to create semaphore");
		return semaphore;
	}
	VkPipeline Device::CreatePipeline(const PipelineBuilderInfo& info)
	{
		VkPipeline pipeline = nullptr;
		// We use always use dynamic states for viewports and scissors
		const VkPipelineViewportStateCreateInfo viewportState = Create::PipelineViewportStateCreateInfo(nullptr, 1, nullptr, 1, nullptr);

		// Blending ops
		const VkPipelineColorBlendStateCreateInfo colorBlending = Create::PipelineColorBlendStateCreateInfo(
			VK_FALSE, VK_LOGIC_OP_COPY, 1, &info.colorBlendAttachment, { 0.0f, 0.0f, 0.0f, 0.0f }
		);

		// Fill pipeline info
		const VkGraphicsPipelineCreateInfo pipelineInfo = Create::GraphicsPipelineCreateInfo(
			info.shaderStagesInfo,
			&info.vertexInputInfo, &info.inputAssemblyInfo, &info.tessellationInfo,
			&viewportState, &info.rasterizerInfo, &info.multisamplingInfo,
			&info.depthStencilInfo, &colorBlending, &info.dynamicState,
			info.pipelineLayout, info.renderPass, 0, VK_NULL_HANDLE, 0
		);

		// Sometimes pipelines can fail to generate, for now we'll treat it as a critical error
		CS_ASSERT(vkCreateGraphicsPipelines(this->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) == VK_SUCCESS, "Failed to create pipeline!");
		return pipeline;
	}
	VkPipelineLayout Device::CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, const std::vector<VkPushConstantRange>& pushConstantRanges)
	{
		VkPipelineLayout pipelineLayout = nullptr;
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = Create::PipelineLayoutCreateInfo(
			descriptorSetLayouts, pushConstantRanges
		);
		CS_ASSERT(vkCreatePipelineLayout(this->device, &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS, "Failed to create pipeline layout!");
		return pipelineLayout;
	}
	VkDescriptorSetLayout Device::CreateDescriptorSetLayout(const VkDescriptorSetLayoutBinding& binding)
	{
		VkDescriptorSetLayout layout = nullptr;
		const VkDescriptorSetLayoutCreateInfo setInfo = Create::DescriptorSetLayoutCreateInfo(binding);
		vkCreateDescriptorSetLayout(this->device, &setInfo, nullptr, &layout);
		return layout;
	}
	VkDescriptorSetLayout Device::CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
	{
		VkDescriptorSetLayout layout = nullptr;
		const VkDescriptorSetLayoutCreateInfo setInfo = Create::DescriptorSetLayoutCreateInfo(bindings);
		vkCreateDescriptorSetLayout(this->device, &setInfo, nullptr, &layout);
		return layout;
	}
	VkSampler Device::CreateSampler(const VkSamplerCreateInfo& info)
	{
		VkSampler sampler = nullptr;
		vkCreateSampler(this->device, &info, nullptr, &sampler);
		return sampler;
	}
	VkDescriptorSet Device::AllocateDescriptorSet(VkDescriptorType type, VkDescriptorSetLayout layout)
	{
		return this->descriptorManager.AllocateSet(type, layout);
	}
	void Device::WriteDescriptorSet(const VkWriteDescriptorSet& descriptorWrite)
	{
		vkUpdateDescriptorSets(this->device, 1, &descriptorWrite, 0, nullptr);
	}
	void Device::WriteDescriptorSets(const std::vector<VkWriteDescriptorSet>& descriptorWrites)
	{
		vkUpdateDescriptorSets(this->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
	void Device::WaitIdle() const
	{
		vkDeviceWaitIdle(this->device);
	}
}