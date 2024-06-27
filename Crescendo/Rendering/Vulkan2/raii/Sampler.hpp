#pragma once

#include "common.hpp"
#include "vulkan/vulkan.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class Sampler
	{
	private:
		VkDevice device;
		VkSampler sampler;
	public:
		Sampler();
		// Create a sampler with the given create info
		Sampler(VkDevice device, VkSamplerCreateInfo createInfo);
		// Takes ownership of the given sampler
		Sampler(VkDevice device, VkSampler sampler);
		~Sampler();
		Sampler(const Sampler&) = delete;
		Sampler& operator=(const Sampler&) = delete;
		Sampler(Sampler&& other) noexcept;
		Sampler& operator=(Sampler&& other) noexcept;
		operator VkSampler() const;
		VkSampler GetSampler() const;
	};
}