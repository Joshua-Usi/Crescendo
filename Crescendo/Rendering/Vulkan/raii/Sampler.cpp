#include "Sampler.hpp"
#include "Volk/volk.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	Sampler::Sampler() : device(nullptr), sampler(nullptr) {}
	Sampler::Sampler(VkDevice device, VkSamplerCreateInfo createInfo) : device(device)
	{
		CS_ASSERT(vkCreateSampler(this->device, &createInfo, nullptr, &this->sampler) == VK_SUCCESS, "Failed to create sampler!");
	}
	Sampler::Sampler(VkDevice device, VkSampler sampler) : device(device), sampler(sampler) {}
	Sampler::~Sampler()
	{
		if (this->device == nullptr) return;
		vkDestroySampler(this->device, this->sampler, nullptr);
	}
	Sampler::Sampler(Sampler&& other) noexcept : device(other.device), sampler(other.sampler)
	{
		other.device = nullptr;
		other.sampler = nullptr;
	}
	Sampler& Sampler::operator=(Sampler&& other) noexcept
	{
		if (this == &other) return *this;
		this->device = other.device;
		this->sampler = other.sampler;
		other.device = nullptr;
		other.sampler = nullptr;
		return *this;
	}
	Sampler::operator VkSampler() const { return this->sampler; }
	VkSampler Sampler::GetSampler() const { return this->sampler; }
}