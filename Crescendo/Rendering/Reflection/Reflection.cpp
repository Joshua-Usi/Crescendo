#include "Reflection.hpp"

#include "Core/common.hpp"

#include "vulkan/vulkan.h"
#include "SPIRV-reflect/spirv_reflect.h"
namespace Crescendo
{
	VkFormat GetFormatFromSize(uint32_t size)
	{
		switch (size)
		{
			case 16: return VK_FORMAT_R32G32B32A32_SFLOAT;
			case 12: return VK_FORMAT_R32G32B32_SFLOAT;
			case 8: return VK_FORMAT_R32G32_SFLOAT;
			case 4: return VK_FORMAT_R32_SFLOAT;
		}
		CS_ASSERT(false, "Unknown vertex attribute format");
		return VK_FORMAT_UNDEFINED;
	}
	uint32_t GetTypeSize(const SpvReflectNumericTraits& traits)
	{
		if (traits.matrix.column_count == 0 && traits.matrix.row_count == 0) return sizeof(float) * traits.vector.component_count;
		else return sizeof(float) * traits.matrix.column_count * traits.matrix.row_count;
	}
	DescriptorType GetType(SpvReflectDescriptorType type)
	{
		switch (type)
		{
			case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return DescriptorType::Sampler;
			case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return DescriptorType::Block;
		}
		CS_ASSERT(false, "Unsupported / Unknown descriptor type");
		return DescriptorType::Unknown;
	}
	SpirvReflection Crescendo::ReflectSpirv(const std::vector<uint8_t>& code)
	{
		SpirvReflection reflection;
		SpvReflectShaderModule reflectionModule;
		CS_ASSERT(spvReflectCreateShaderModule(code.size(), code.data(), &reflectionModule) == SPV_REFLECT_RESULT_SUCCESS, "Failed to create shader reflection module!");
		// Reflect input variables
		{
			uint32_t inputCount;
			CS_ASSERT(spvReflectEnumerateInputVariables(&reflectionModule, &inputCount, nullptr) == SPV_REFLECT_RESULT_SUCCESS, "Failed to enumerate input variables!");
			std::vector<SpvReflectInterfaceVariable*> inputVariables(inputCount);
			CS_ASSERT(spvReflectEnumerateInputVariables(&reflectionModule, &inputCount, inputVariables.data()) == SPV_REFLECT_RESULT_SUCCESS, "Failed to enumerate input variables!");
			for (uint32_t i = 0; i < inputCount; i++)
			{
				if (inputVariables[i]->location == std::numeric_limits<uint32_t>::max()) continue;
				reflection.inputVariables.emplace_back(inputVariables[i]->location, GetTypeSize(inputVariables[i]->numeric));
			}
		}
		// Reflect output variables
		{
			uint32_t outputCount;
			CS_ASSERT(spvReflectEnumerateOutputVariables(&reflectionModule, &outputCount, nullptr) == SPV_REFLECT_RESULT_SUCCESS, "Failed to enumerate output variables!");
			std::vector<SpvReflectInterfaceVariable*> outputVariables(outputCount);
			CS_ASSERT(spvReflectEnumerateOutputVariables(&reflectionModule, &outputCount, outputVariables.data()) == SPV_REFLECT_RESULT_SUCCESS, "Failed to enumerate output variables!");
			for (uint32_t i = 0; i < outputCount; i++)
			{
				if (outputVariables[i]->location == std::numeric_limits<uint32_t>::max()) continue;
				reflection.outputVariables.emplace_back(outputVariables[i]->location, GetTypeSize(outputVariables[i]->numeric));
			}
		}
		// Reflect descriptor sets
		{
			uint32_t setCount;
			CS_ASSERT(spvReflectEnumerateDescriptorSets(&reflectionModule, &setCount, nullptr) == SPV_REFLECT_RESULT_SUCCESS, "Failed to enumerate descriptor sets!");
			std::vector<SpvReflectDescriptorSet*> sets(setCount);
			CS_ASSERT(spvReflectEnumerateDescriptorSets(&reflectionModule, &setCount, sets.data()) == SPV_REFLECT_RESULT_SUCCESS, "Failed to enumerate descriptor sets!");
			
			for (uint32_t i = 0; i < setCount; i++)
			{
				SpvReflectDescriptorSet* set = sets[i];
				DescriptorSetLayout descriptorSet;
				descriptorSet.set = set->set;
				for (uint32_t j = 0; j < set->binding_count; j++)
				{
					SpvReflectDescriptorBinding* binding = set->bindings[j];
					descriptorSet.binding = binding->binding;
					descriptorSet.type = GetType(binding->descriptor_type);
					for (uint32_t k = 0; k < binding->block.member_count; k++)
					{
						SpvReflectBlockVariable member = binding->block.members[k];
						descriptorSet.members.emplace_back(member.offset, member.size);
					}
				}
				reflection.descriptorSets.push_back(descriptorSet);
			}
		}
		// Reflect push constants
		{
			uint32_t pushConstantCount;
			CS_ASSERT(spvReflectEnumeratePushConstantBlocks(&reflectionModule, &pushConstantCount, nullptr) == SPV_REFLECT_RESULT_SUCCESS, "Failed to enumerate push constants!");
			std::vector<SpvReflectBlockVariable*> pushConstants(pushConstantCount);
			CS_ASSERT(spvReflectEnumeratePushConstantBlocks(&reflectionModule, &pushConstantCount, pushConstants.data()) == SPV_REFLECT_RESULT_SUCCESS, "Failed to enumerate push constants!");
			for (uint32_t i = 0; i < pushConstantCount; i++)
			{
				SpvReflectBlockVariable* pushConstant = pushConstants[i];
				reflection.pushConstant.members.resize(pushConstant->member_count);
				for (uint32_t j = 0; j < pushConstant->member_count; j++)
				{
					SpvReflectBlockVariable member = pushConstant->members[j];
					reflection.pushConstant.members[j] = { member.offset, member.size };
				}
			}
		}
		spvReflectDestroyShaderModule(&reflectionModule);
		return reflection;
	}
	const std::vector<VkDescriptorSetLayoutBinding> SpirvReflection::GetDescriptorBindings(uint32_t shaderStage) const
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		for (const auto& layout : this->descriptorSets)
		{
			VkDescriptorType descriptorType = layout.IsSampler() ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			bindings.emplace_back(layout.binding, descriptorType, 1, shaderStage, nullptr);
		}
		return bindings;
	}
	const std::vector<VkVertexInputBindingDescription> SpirvReflection::GetVertexBindings() const
	{
		std::vector<VkVertexInputBindingDescription> bindings(this->inputVariables.size());
		for (uint32_t i = 0; i < this->inputVariables.size(); i++)
		{
			bindings[i] = { i, this->inputVariables[i].size, VK_VERTEX_INPUT_RATE_VERTEX };
		}
		return bindings;
	}
	const std::vector<VkVertexInputAttributeDescription> SpirvReflection::GetVertexAttributes() const
	{
		std::vector<VkVertexInputAttributeDescription> attributes(this->inputVariables.size());
		for (uint32_t i = 0; i < this->inputVariables.size(); i++)
		{
			attributes[i] = { this->inputVariables[i].location, 0, GetFormatFromSize(this->inputVariables[i].size), 0};
		}
		return attributes;
	}
	VkPushConstantRange SpirvReflection::GetPushConstantRange(uint32_t shaderStage) const
	{
		return { shaderStage, this->pushConstant.members[0].offset, this->pushConstant.GetSize() };
	}
}