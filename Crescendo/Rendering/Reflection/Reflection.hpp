#pragma once

#include <vector>
#include <string>

struct VkDescriptorSetLayoutBinding;
struct VkDescriptorSetLayoutCreateInfo;
struct VkVertexInputBindingDescription;
struct VkVertexInputAttributeDescription;
struct VkPushConstantRange;

namespace Crescendo
{
	struct SpirvReflection
	{
		// We only ever use block and sampler descriptors
		enum class DescriptorType : uint32_t { Unknown = 0, Block = 1, Sampler = 2, All = 3 };
		struct InterfaceVariable { std::string name;  uint32_t location, size; };
		struct BlockMember { uint32_t offset, size; };
		struct DescriptorSetBinding
		{
			std::vector<BlockMember> members;
			uint32_t binding;
			DescriptorType type; // Block or sampler
			inline uint32_t GetSize() const
			{
				uint32_t size = 0;
				for (const auto& member : members) size += member.size;
				return size;
			}
			inline uint32_t IsSampler() const { return type == DescriptorType::Sampler; }
		};
		struct DescriptorSetLayout
		{
			std::vector<DescriptorSetBinding> bindings;
			uint32_t set;
		};
		struct PushConstantLayout
		{
			std::vector<BlockMember> members;
			inline uint32_t GetSize() const
			{
				uint32_t size = 0;
				for (const auto& member : members) size += member.size;
				return size;
			}
		};

		std::vector<InterfaceVariable> inputVariables;
		std::vector<InterfaceVariable> outputVariables;
		std::vector<DescriptorSetLayout> descriptorSetLayouts;
		std::vector<PushConstantLayout> pushConstants;

		std::vector<DescriptorSetLayout> GetDescriptorSetLayouts(SpirvReflection::DescriptorType descriptorType = DescriptorType::All) const;

		std::vector<std::vector<VkDescriptorSetLayoutBinding>> GetDescriptorSetLayoutBindings(SpirvReflection::DescriptorType descriptorType, uint32_t shaderStage) const;
		std::vector<VkVertexInputBindingDescription> GetVertexBindings() const;
		std::vector<VkVertexInputAttributeDescription> GetVertexAttributes() const;
		std::vector<VkPushConstantRange> GetPushConstantRanges(uint32_t shaderStage) const;
	};

	SpirvReflection ReflectSpirv(const std::vector<uint8_t>& code);
}