#include "PipelineLayout.hpp"
#include "Volk/volk.h"
#include "../Create.hpp"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	PipelineLayout::PipelineLayout() : device(nullptr), layout(nullptr) {}
	PipelineLayout::PipelineLayout(VkDevice& device, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, std::vector<VkPushConstantRange>& pushConstantRanges) : device(device)
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = Create::PipelineLayoutCreateInfo(descriptorSetLayouts, pushConstantRanges);
		CS_ASSERT(vkCreatePipelineLayout(this->device, &pipelineLayoutInfo, nullptr, &this->layout) == VK_SUCCESS, "Failed to create pipeline layout!");
	}
	PipelineLayout::~PipelineLayout()
	{
		if (this->device != nullptr) vkDestroyPipelineLayout(this->device, this->layout, nullptr);
	}
	PipelineLayout::PipelineLayout(PipelineLayout&& other) : device(other.device), layout(other.layout)
	{
		other.device = nullptr;
		other.layout = nullptr;
	}
	PipelineLayout& PipelineLayout::operator=(PipelineLayout&& other)
	{
		if (this->device != nullptr) vkDestroyPipelineLayout(this->device, this->layout, nullptr);
		this->device = other.device;
		this->layout = other.layout;
		other.device = nullptr;
		other.layout = nullptr;
		return *this;
	}
	PipelineLayout::operator VkPipelineLayout() const { return this->layout; }
	VkPipelineLayout PipelineLayout::GetLayout() const { return this->layout; }
}