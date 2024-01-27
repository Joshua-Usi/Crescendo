#pragma once

#include "common.hpp"
#include "vulkan/vulkan.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class DescriptorSetLayout
	{
	private:
		VkDevice device;
		VkDescriptorSetLayout layout;
	public:
		DescriptorSetLayout();
		// Creates descriptorSetLayout from createInfo
		DescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo& createInfo);
		// Takes ownership of the descriptorSetLayout
		explicit DescriptorSetLayout(VkDevice device, VkDescriptorSetLayout layout);
		~DescriptorSetLayout();
		DescriptorSetLayout(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout(DescriptorSetLayout&& other) noexcept;
		DescriptorSetLayout& operator=(DescriptorSetLayout&& other) noexcept;
		operator VkDescriptorSetLayout() const;
		VkDescriptorSetLayout GetDescriptorSetLayout() const;
	};
}