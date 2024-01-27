#pragma once

#include "common.hpp"
#include "vulkan/vulkan.h"
#include <vector>

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class PipelineLayout
	{
	private:
		VkDevice device;
		VkPipelineLayout layout;
	public:
		PipelineLayout();
		PipelineLayout(VkDevice& device, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, std::vector<VkPushConstantRange>& pushConstantRanges);
		~PipelineLayout();
		PipelineLayout(const PipelineLayout&) = delete;
		PipelineLayout& operator=(const PipelineLayout&) = delete;
		PipelineLayout(PipelineLayout&& other);
		PipelineLayout& operator=(PipelineLayout&& other);
		operator VkPipelineLayout() const;
		VkPipelineLayout GetLayout() const;
	};
}