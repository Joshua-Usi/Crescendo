#include "DescriptorSetLayout.hpp"
#include "Volk/volk.h"
#include "../Create.hpp"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	DescriptorSetLayout::DescriptorSetLayout() : device(nullptr), layout(nullptr) {}
	// Creates descriptorSetLayout from createInfo
	DescriptorSetLayout::DescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo& createInfo) : device(device)
	{
		CS_ASSERT(vkCreateDescriptorSetLayout(this->device, &createInfo, nullptr, &layout) == VK_SUCCESS, "Failed to create descriptor set layout");
	}
	// Takes ownership of the descriptorSetLayout
	DescriptorSetLayout::DescriptorSetLayout(VkDevice device, VkDescriptorSetLayout layout) : device(device), layout(layout) {}
	DescriptorSetLayout::~DescriptorSetLayout()
	{
		if (this->device != nullptr) vkDestroyDescriptorSetLayout(this->device, this->layout, nullptr);
	}
	DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) noexcept : device(other.device), layout(other.layout)
	{
		other.device = nullptr;
		other.layout = nullptr;
	}
	DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& other) noexcept
	{
		if (this->device != nullptr) vkDestroyDescriptorSetLayout(this->device, this->layout, nullptr);
		this->device = other.device;
		this->layout = other.layout;
		other.device = nullptr;
		other.layout = nullptr;
		return *this;
	}
	DescriptorSetLayout::operator VkDescriptorSetLayout() const { return this->layout; }
	VkDescriptorSetLayout DescriptorSetLayout::GetDescriptorSetLayout() const { return this->layout; }
}