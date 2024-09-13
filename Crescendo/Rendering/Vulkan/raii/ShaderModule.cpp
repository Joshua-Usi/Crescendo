#include "ShaderModule.hpp"
#include "Volk/volk.h"
#include "../Create.hpp"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	ShaderModule::ShaderModule() : device(nullptr), shaderModule(nullptr) {}
	ShaderModule::ShaderModule(VkDevice device, const std::vector<uint8_t>& code) : device(device)
	{
		if (code.size() == 0)
		{
			this->shaderModule = nullptr;
			cs_std::console::warn("Shader code is empty!");
			return;
		}
		const VkShaderModuleCreateInfo createInfo = Create::ShaderModuleCreateInfo(code);
		CS_ASSERT(vkCreateShaderModule(this->device, &createInfo, nullptr, &this->shaderModule) == VK_SUCCESS, "Failed to create shader module!");
	}
	ShaderModule::~ShaderModule()
	{
		if (this->device != nullptr && this->shaderModule != nullptr)
			vkDestroyShaderModule(this->device, this->shaderModule, nullptr);	
	}
	ShaderModule::ShaderModule(ShaderModule&& other) : device(other.device), shaderModule(other.shaderModule)
	{
		other.device = nullptr;
		other.shaderModule = nullptr;
	}
	ShaderModule& ShaderModule::operator=(ShaderModule&& other)
	{
		if (this->device != nullptr && this->shaderModule != nullptr)
			vkDestroyShaderModule(this->device, this->shaderModule, nullptr);
		this->device = other.device;
		this->shaderModule = other.shaderModule;
		other.device = nullptr;
		other.shaderModule = nullptr;
		return *this;
	}
	ShaderModule::operator VkShaderModule() const { return this->shaderModule; }
	VkShaderModule ShaderModule::GetShaderModule() const { return this->shaderModule; }
}