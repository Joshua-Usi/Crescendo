#pragma once

#include <vector>

struct VkDescriptorSetLayoutBinding;
struct VkDescriptorSetLayoutCreateInfo;
struct VkVertexInputBindingDescription;
struct VkVertexInputAttributeDescription;
struct VkPushConstantRange;

namespace Crescendo
{
	enum class DescriptorType : uint32_t { Unknown = 0, Block = 1, Sampler = 2 };
	struct InterfaceVariable { uint32_t location, size; };
	struct BlockMember { uint32_t offset, size; };
	struct DescriptorSetLayout
	{
		std::vector<BlockMember> members;
		uint32_t set, binding;
		DescriptorType type; // Block or sampler
		inline uint32_t GetSize() const
		{
			uint32_t size = 0;
			for (const auto& member : members) size += member.size;
			return size;
		}
		inline uint32_t IsSampler() const { return type == DescriptorType::Sampler; }
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

	struct SpirvReflection
	{
		std::vector<InterfaceVariable> inputVariables;
		std::vector<InterfaceVariable> outputVariables;
		std::vector<DescriptorSetLayout> descriptorSets;
		PushConstantLayout pushConstant;
		const std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetBindings(uint32_t shaderStage) const;
		VkDescriptorSetLayoutCreateInfo GetDescriptorSetLayouts(uint32_t shaderStage) const;
		const std::vector<VkVertexInputBindingDescription> GetVertexBindings() const;
		const std::vector<VkVertexInputAttributeDescription> GetVertexAttributes() const;
		VkPushConstantRange GetPushConstantRange(uint32_t shaderStage) const;
	};

	SpirvReflection ReflectSpirv(const std::vector<uint8_t>& code);
}