#pragma once

#include "Volk/volk.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Crescendo::Vulkan
{
	struct ShaderReflection
	{
	public:
		// We only ever use block and sampler descriptors
		enum class DescriptorType : uint32_t { Unknown = 0, Block, Sampler, Storage};
		struct InterfaceVariable { std::string name;  uint32_t location, size; };
		struct BlockMember { uint32_t offset, size; };
		struct DescriptorSetBinding
		{
			std::vector<BlockMember> members;
			uint32_t binding;
			DescriptorType type; // Block or sampler
			uint32_t GetSize() const
			{
				uint32_t size = 0;
				for (const auto& member : members) size += member.size;
				return size;
			}
		};
		struct DescriptorSetLayout
		{
			std::vector<DescriptorSetBinding> bindings;
			uint32_t set;
		};
		struct PushConstantLayout
		{
			std::vector<BlockMember> members;
			uint32_t GetSize() const
			{
				uint32_t size = 0;
				for (const auto& member : members) size += member.size;
				return size;
			}
		};
	public:
		std::vector<InterfaceVariable> inputVariables;
		std::vector<InterfaceVariable> outputVariables;
		std::vector<DescriptorSetLayout> descriptorSetLayouts;
		std::vector<PushConstantLayout> pushConstants;
	public:
		ShaderReflection(const std::vector<uint8_t>& code);
	public:
		std::vector<DescriptorSetLayout> GetDescriptorSetLayouts(DescriptorType descriptorType) const;
		size_t GetDescriptorSetLayoutCount(DescriptorType descriptorType) const;

		std::vector<std::vector<VkDescriptorSetLayoutBinding>> GetDescriptorSetLayoutBindings(DescriptorType descriptorType, uint32_t shaderStage) const;
		std::vector<VkVertexInputBindingDescription> GetVertexBindings() const;
		std::vector<VkVertexInputAttributeDescription> GetVertexAttributes() const;
		std::vector<VkPushConstantRange> GetPushConstantRanges(uint32_t shaderStage) const;
	};
}