#pragma once

#include "common.hpp"
#include "vulkan/vulkan.h"
#include "cs_std/graphics/algorithms.hpp"
#include "PipelineVariants.hpp"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class Pipeline
	{
	public:
		struct PipelineCreateInfo
		{
			std::vector<uint8_t> vertexCode, fragmentCode;
			PipelineVariants variants;
			VkDescriptorSetLayout descriptorSetLayout;
			VkRenderPass renderPass;
		};
	private:	
		VkDevice device;
		VkPipelineLayout layout;
		PipelineVariants variants;
		VkDescriptorSetLayout descriptorSetLayout; // Does not own the layout, hence doesn't destroy it
		VkRenderPass renderPass; // Does not own the render pass, hence doesn't destroy it
		std::vector<VkPipeline> pipelines; // List of pipelines that are created, each one based on the index from PipelineVariants
		std::vector<cs_std::graphics::Attribute> vertexAttributes; // Attributes that are required for the pipeline
	public:
		Pipeline();
		Pipeline(VkDevice device, const PipelineCreateInfo& createInfo);
		~Pipeline();
		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;
		Pipeline(Pipeline&& other) noexcept;
		Pipeline& operator=(Pipeline&& other) noexcept;
	public:
		const PipelineVariants& GetVariants() const;
		operator VkPipelineLayout() const;
		// If used in this way, returns the first pipeline in the list, useful for pipelines with single variants
		operator VkPipeline() const;
		VkPipeline operator [](uint32_t index) const;
		VkDescriptorSetLayout GetDescriptorSetLayout() const;
	};
}