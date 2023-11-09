#include "ShaderReflection.hpp"
#include "Core/common.hpp"
#include "Create.hpp"

#include "SPIRV-reflect/spirv_reflect.h"

#include <algorithm>

namespace Crescendo::Vulkan
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
		return sizeof(float) * traits.matrix.column_count * traits.matrix.row_count;
	}
	ShaderReflection::DescriptorType GetType(SpvReflectDescriptorType type)
	{
		switch (type)
		{
		case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return ShaderReflection::DescriptorType::Sampler;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return ShaderReflection::DescriptorType::Block;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return ShaderReflection::DescriptorType::Block;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER: return ShaderReflection::DescriptorType::Storage;
		}
		CS_ASSERT(false, "Unsupported / Unknown descriptor type");
		return ShaderReflection::DescriptorType::Unknown;
	}
	VkDescriptorType GetType(ShaderReflection::DescriptorType type)
	{
		switch (type)
		{
		case ShaderReflection::DescriptorType::Sampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case ShaderReflection::DescriptorType::Block: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		case ShaderReflection::DescriptorType::Storage: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		}
		CS_ASSERT(false, "Unsupported / Unknown descriptor type");
		return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}
	std::vector<std::vector<VkDescriptorSetLayoutBinding>> ShaderReflection::GetDescriptorSetLayoutBindings(ShaderReflection::DescriptorType descriptorType, uint32_t shaderStage) const
	{
		std::vector<std::vector<VkDescriptorSetLayoutBinding>> bindings;
		for (const auto& setLayout : this->descriptorSetLayouts)
		{
			std::vector<VkDescriptorSetLayoutBinding> setBindings;
			for (const auto& binding : setLayout.bindings)
			{
				if (binding.type != descriptorType) break;
				VkDescriptorType descriptorType = GetType(binding.type);
				setBindings.emplace_back(binding.binding, descriptorType, 1, shaderStage, nullptr);
			}
			if (setBindings.size() > 0) bindings.push_back(setBindings);
		}
		return bindings;
	}
	ShaderReflection::ShaderReflection(const std::vector<uint8_t>& code)
	{
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
				this->inputVariables.emplace_back(inputVariables[i]->name, inputVariables[i]->location, GetTypeSize(inputVariables[i]->numeric));
			}
			// Sort in location ascending
			std::sort(this->inputVariables.begin(), this->inputVariables.end(), [](const ShaderReflection::InterfaceVariable& a, const ShaderReflection::InterfaceVariable& b) { return a.location < b.location; });
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
				this->outputVariables.emplace_back(outputVariables[i]->name, outputVariables[i]->location, GetTypeSize(outputVariables[i]->numeric));
			}
			// Sort in location ascending
			std::sort(this->outputVariables.begin(), this->outputVariables.end(), [](const ShaderReflection::InterfaceVariable& a, const ShaderReflection::InterfaceVariable& b) { return a.location < b.location; });
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

				ShaderReflection::DescriptorSetLayout descriptorSet;
				descriptorSet.set = set->set;

				for (uint32_t j = 0; j < set->binding_count; j++)
				{
					SpvReflectDescriptorBinding* binding = set->bindings[j];

					ShaderReflection::DescriptorSetBinding bindingSet;
					bindingSet.binding = binding->binding;
					bindingSet.type = GetType(binding->descriptor_type);
					
					if (bindingSet.type == ShaderReflection::DescriptorType::Storage)
					{
						cs_std::console::log("Type is storage");
					}

					for (uint32_t k = 0; k < binding->block.member_count; k++)
					{
						SpvReflectBlockVariable member = binding->block.members[k];

						bindingSet.members.emplace_back(member.offset, member.size);
					}
					descriptorSet.bindings.push_back(bindingSet);
				}
				this->descriptorSetLayouts.push_back(descriptorSet);
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

				ShaderReflection::PushConstantLayout pushConstantLayout;

				for (uint32_t j = 0; j < pushConstant->member_count; j++)
				{
					SpvReflectBlockVariable member = pushConstant->members[j];
					pushConstantLayout.members.emplace_back(member.offset, member.size);
				}

				this->pushConstants.push_back(pushConstantLayout);
			}
		}
		spvReflectDestroyShaderModule(&reflectionModule);
	}
	std::vector<ShaderReflection::DescriptorSetLayout> ShaderReflection::GetDescriptorSetLayouts(ShaderReflection::DescriptorType descriptorType) const
	{
		std::vector<ShaderReflection::DescriptorSetLayout> layouts;
		for (const auto& setLayout : this->descriptorSetLayouts)
		{
			if (setLayout.bindings.front().type != descriptorType) continue;
			layouts.push_back(setLayout);
		}
		return layouts;
	}
	size_t ShaderReflection::GetDescriptorSetLayoutCount(DescriptorType descriptorType) const
	{
		size_t count = 0;
		for (const auto& setLayout : this->descriptorSetLayouts)
		{
			if (setLayout.bindings.front().type == descriptorType) count++;
		}
		return count;
	}
	std::vector<VkVertexInputBindingDescription> ShaderReflection::GetVertexBindings() const
	{
		std::vector<VkVertexInputBindingDescription> bindings(this->inputVariables.size());
		for (uint32_t i = 0; i < this->inputVariables.size(); i++)
		{
			bindings[i] = { this->inputVariables[i].location, this->inputVariables[i].size, VK_VERTEX_INPUT_RATE_VERTEX};
		}
		return bindings;
	}
	std::vector<VkVertexInputAttributeDescription> ShaderReflection::GetVertexAttributes() const
	{
		std::vector<VkVertexInputAttributeDescription> attributes(this->inputVariables.size());
		for (uint32_t i = 0; i < this->inputVariables.size(); i++)
		{
			attributes[i] = { this->inputVariables[i].location, this->inputVariables[i].location, GetFormatFromSize(this->inputVariables[i].size), 0};
		}
		return attributes;
	}
	std::vector<VkPushConstantRange> ShaderReflection::GetPushConstantRanges(uint32_t shaderStage) const
	{
		std::vector<VkPushConstantRange> ranges;
		for (const auto& pushConstant : this->pushConstants)
		{
			ranges.push_back({ shaderStage, pushConstant.members[0].offset, pushConstant.GetSize() });
		}
		return ranges;
	}
}