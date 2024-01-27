#include "Device.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	Device::Device() :
		device(), allocator(), descriptorManager(),
		vertexSamplerSetLayout(), fragmentSamplerSetLayout(), vertexSSBOSetLayout(), fragmentSSBOSetLayout(), postProcessingSampler(nullptr) {}
	Device::Device(const Vk::Instance& instance, const Vk::PhysicalDevice& physicalDevice, const DeviceSpecification& spec) :
		device(physicalDevice, spec.drawParametersFeatures), allocator(instance, physicalDevice, this->device), descriptorManager(this->device, spec.descriptorPoolMaxSets)
	{

	}
	VkDescriptorSetLayout Device::GetSamplerLayout(VkShaderStageFlags shaderStage) const { return (shaderStage == VK_SHADER_STAGE_VERTEX_BIT) ? vertexSamplerSetLayout : fragmentSamplerSetLayout; }
	VkDescriptorSetLayout Device::GetSSBOLayout(VkShaderStageFlags shaderStage) const { return (shaderStage == VK_SHADER_STAGE_VERTEX_BIT) ? vertexSSBOSetLayout : fragmentSSBOSetLayout; }
	VkSampler Device::GetPostProcessingSampler() const { return postProcessingSampler; }
	void Device::WaitIdle() { this->device.WaitIdle(); }
	Device::operator Vk::Device& () { return this->device; }
	Device::operator VkDevice() const { return this->device; }
}