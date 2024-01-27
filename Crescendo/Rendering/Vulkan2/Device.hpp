#pragma once

#include "common.hpp"
#include "RAII.hpp"
#include "Allocator.hpp"
#include "DescriptorManager.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	class Device
	{
	private:
		Vk::Device device;
		Allocator allocator;
		DescriptorManager descriptorManager;

		// Universally shared resources
		Vk::DescriptorSetLayout vertexSamplerSetLayout, fragmentSamplerSetLayout;
		Vk::DescriptorSetLayout vertexSSBOSetLayout, fragmentSSBOSetLayout;
		VkSampler postProcessingSampler;
	public:
		struct DeviceSpecification
		{
			VkPhysicalDeviceShaderDrawParametersFeatures drawParametersFeatures;
			uint32_t descriptorPoolMaxSets;
		};
		struct PipelineBuilderInfo
		{
			VkPipelineDynamicStateCreateInfo dynamicState;
			std::vector<VkPipelineShaderStageCreateInfo> shaderStagesInfo;
			VkPipelineVertexInputStateCreateInfo vertexInputInfo;
			VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
			VkPipelineTessellationStateCreateInfo tessellationInfo;
			VkPipelineRasterizationStateCreateInfo rasterizerInfo;
			VkPipelineMultisampleStateCreateInfo multisamplingInfo;
			VkPipelineColorBlendAttachmentState colorBlendAttachment;
			VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
			VkPipelineLayout pipelineLayout;
			VkRenderPass renderPass;
		};
	public:
		Device();
		Device(const Vk::Instance& instance, const Vk::PhysicalDevice& physicalDevice, const DeviceSpecification& spec);
		VkDescriptorSetLayout GetSamplerLayout(VkShaderStageFlags shaderStage) const;
		VkDescriptorSetLayout GetSSBOLayout(VkShaderStageFlags shaderStage) const;
		VkSampler GetPostProcessingSampler() const;
		void WaitIdle();
		operator Vk::Device&();
		operator VkDevice() const;
	};
}