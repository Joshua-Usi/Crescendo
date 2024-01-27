#pragma once

#include "common.hpp"
#include "vulkan/vulkan.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class ShaderModule
	{
	private:
		VkDevice device;
		VkShaderModule shaderModule;
	public:
		ShaderModule();
		ShaderModule(VkDevice device, const std::vector<uint8_t>& code);
		~ShaderModule();
		ShaderModule(const ShaderModule&) = delete;
		ShaderModule& operator=(const ShaderModule&) = delete;
		ShaderModule(ShaderModule&& other);
		ShaderModule& operator=(ShaderModule&& other);
		operator VkShaderModule() const;
		VkShaderModule GetShaderModule() const;
	};
}