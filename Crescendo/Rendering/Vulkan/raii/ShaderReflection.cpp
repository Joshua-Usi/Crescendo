#include "ShaderReflection.hpp"
#include "../Create.hpp"
#include "SPIRV-reflect/spirv_reflect.h"
#include <algorithm>

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	VkFormat GetFormatFromSize(uint32_t size)
	{
		switch (size)
		{
			case 16:
				return VK_FORMAT_R32G32B32A32_SFLOAT;
			case 12:
				return VK_FORMAT_R32G32B32_SFLOAT;
			case 8:
				return VK_FORMAT_R32G32_SFLOAT;
			case 4:
				return VK_FORMAT_R32_SFLOAT;
		}
		return VK_FORMAT_UNDEFINED;
	}
	uint32_t GetTypeSize(const SpvReflectNumericTraits& traits)
	{
		if (traits.matrix.column_count == 0 && traits.matrix.row_count == 0)
			return sizeof(float) * traits.vector.component_count;
		return sizeof(float) * traits.matrix.column_count * traits.matrix.row_count;
	}
	ShaderReflection::DescriptorType GetType(SpvReflectDescriptorType type)
	{
		switch (type)
		{
		case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			return ShaderReflection::DescriptorType::Sampler;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			return ShaderReflection::DescriptorType::Block;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			return ShaderReflection::DescriptorType::Block;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			return ShaderReflection::DescriptorType::Storage;
		}
		return ShaderReflection::DescriptorType::Unknown;
	}
	ShaderReflection::ShaderReflection() {}
	ShaderReflection::ShaderReflection(const std::vector<uint8_t>& code)
	{
		if (code.size() == 0)
			return;

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
				if (inputVariables[i]->location == std::numeric_limits<uint32_t>::max())
					continue;
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
				if (outputVariables[i]->location == std::numeric_limits<uint32_t>::max())
					continue;
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
	constexpr size_t ShaderReflection::GetInputVariableCount() const { return this->inputVariables.size(); }
	constexpr size_t ShaderReflection::GetOutputVariableCount() const { return this->outputVariables.size(); }
	constexpr size_t ShaderReflection::GetDescriptorSetLayoutCount() const { return this->descriptorSetLayouts.size(); }
	constexpr size_t ShaderReflection::GetPushConstantLayoutCount() const { return this->pushConstants.size(); }
	const ShaderReflection::InterfaceVariable& ShaderReflection::GetInputVariable(size_t idx) const { return this->inputVariables[idx]; }
	const ShaderReflection::InterfaceVariable& ShaderReflection::GetOutputVariable(size_t idx) const { return this->outputVariables[idx]; }
	const ShaderReflection::DescriptorSetLayout& ShaderReflection::GetDescriptorSetLayout(size_t idx) const { return this->descriptorSetLayouts[idx]; }
	const ShaderReflection::PushConstantLayout& ShaderReflection::GetPushConstantLayout(size_t idx) const { return this->pushConstants[idx]; }
	const uint32_t ShaderReflection::GetPushConstantSize() const
	{
		uint32_t size = 0;
		for (const auto& pushConstant : this->pushConstants)
			size += pushConstant.GetSize();
		return size;
	
	}
	VkVertexInputBindingDescription ShaderReflection::GenerateVertexBindingDescription(uint32_t idx) const
	{
		return Create::VertexInputBindingDescription(this->inputVariables[idx].location, this->inputVariables[idx].size, VK_VERTEX_INPUT_RATE_VERTEX);
	}
	VkVertexInputAttributeDescription ShaderReflection::GenerateVertexAttributeDescription(uint32_t idx) const
	{
		return Create::VertexInputAttributeDescription(this->inputVariables[idx].location, this->inputVariables[idx].location, GetFormatFromSize(this->inputVariables[idx].size), 0);
	}
	std::vector<VkDescriptorSetLayoutBinding> ShaderReflection::GenerateDescriptorSetLayoutBinding(uint32_t idx, ShaderStage stage) const
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		for (const auto& binding : this->descriptorSetLayouts[idx].bindings)
		{
			const VkDescriptorSetLayoutBinding layoutBinding = Create::DescriptorSetLayoutBinding(
				binding.binding, static_cast<VkDescriptorType>(binding.type), 1, static_cast<VkShaderStageFlags>(stage), nullptr
			);
			bindings.push_back(layoutBinding);
		}
		return bindings;
	}
	VkPushConstantRange ShaderReflection::GeneratePushConstantRange(uint32_t idx, ShaderStage stage, uint32_t offset) const
	{
		return Create::PushConstantRange(static_cast<VkShaderStageFlags>(stage), offset, this->pushConstants[idx].GetSize());
	}
	std::vector<VkVertexInputBindingDescription> ShaderReflection::GenerateVertexBindings() const
	{
		std::vector<VkVertexInputBindingDescription> bindings(this->GetInputVariableCount());
		for (uint32_t i = 0; i < this->inputVariables.size(); i++)
			bindings[i] = this->GenerateVertexBindingDescription(i);
		return bindings;
	}
	std::vector<VkVertexInputAttributeDescription> ShaderReflection::GenerateVertexAttributes() const
	{
		std::vector<VkVertexInputAttributeDescription> attributes(this->GetInputVariableCount());
		for (uint32_t i = 0; i < this->inputVariables.size(); i++)
			attributes[i] = this->GenerateVertexAttributeDescription(i);
		return attributes;
	}
	std::vector<std::vector<VkDescriptorSetLayoutBinding>> ShaderReflection::GenerateDescriptorSetLayoutBindings(ShaderStage stage) const
	{
		std::vector<std::vector<VkDescriptorSetLayoutBinding>> bindings(this->GetDescriptorSetLayoutCount());
		for (uint32_t i = 0; i < this->descriptorSetLayouts.size(); i++)
			bindings[i] = this->GenerateDescriptorSetLayoutBinding(i, stage);
		return bindings;
	}
	std::vector<VkPushConstantRange> ShaderReflection::GeneratePushConstantRanges(ShaderStage stage, uint32_t offset) const
	{
		std::vector<VkPushConstantRange> ranges(this->GetPushConstantLayoutCount());
		for (uint32_t i = 0; i < this->pushConstants.size(); i++)
			ranges[i] = this->GeneratePushConstantRange(i, stage, offset);
		return ranges;
	}
}