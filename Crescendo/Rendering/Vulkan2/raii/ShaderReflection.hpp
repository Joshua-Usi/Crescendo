#pragma once
#include "common.hpp"
#include "vulkan/vulkan.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class ShaderReflection
	{
	public:
		enum class DescriptorType : uint32_t
		{
			Unknown = VK_DESCRIPTOR_TYPE_MAX_ENUM,
			Block = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			Sampler = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			Storage = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
		};
		enum class ShaderStage : uint32_t
		{
			Unknown = 0,
			Vertex = VK_SHADER_STAGE_VERTEX_BIT,
			Fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
			Compute = VK_SHADER_STAGE_COMPUTE_BIT
		};
		struct InterfaceVariable { std::string name; uint32_t location, size; };
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
		ShaderReflection();
		ShaderReflection(const std::vector<uint8_t>& code);
		~ShaderReflection() = default;
		ShaderReflection(const ShaderReflection&) = default;
		ShaderReflection(ShaderReflection&&) = default;
		ShaderReflection& operator=(const ShaderReflection&) noexcept = default;
		ShaderReflection& operator=(ShaderReflection&&) noexcept = default;
	public:
		constexpr size_t GetInputVariableCount() const;
		constexpr size_t GetOutputVariableCount() const;
		constexpr size_t GetDescriptorSetLayoutCount() const;
		constexpr size_t GetPushConstantLayoutCount() const;

		const InterfaceVariable& GetInputVariable(size_t idx) const;
		const InterfaceVariable& GetOutputVariable(size_t idx) const;
		const DescriptorSetLayout& GetDescriptorSetLayout(size_t idx) const;
		const PushConstantLayout& GetPushConstantLayout(size_t idx) const;

		VkVertexInputBindingDescription GenerateVertexBindingDescription(uint32_t idx) const;
		VkVertexInputAttributeDescription GenerateVertexAttributeDescription(uint32_t idx) const;
		std::vector<VkDescriptorSetLayoutBinding> GenerateDescriptorSetLayoutBinding(uint32_t idx, ShaderStage stage) const;
		VkPushConstantRange GeneratePushConstantRange(uint32_t idx, ShaderStage stage) const;

		std::vector<VkVertexInputBindingDescription> GenerateVertexBindings() const;
		std::vector<VkVertexInputAttributeDescription> GenerateVertexAttributes() const;
		std::vector<std::vector<VkDescriptorSetLayoutBinding>> GenerateDescriptorSetLayoutBindings(ShaderStage stage) const;
		std::vector<VkPushConstantRange> GeneratePushConstantRanges(ShaderStage stage) const;
	};
	CS_DEFINE_ENUM_CLASS_OR_OPERATOR(ShaderReflection::ShaderStage);
}