#pragma once

#include "Volk/volk.h"

namespace Crescendo::Vulkan
{
	struct ShaderModule
	{
	private:
		VkDevice device;
		VkShaderModule module;
	public:
		// Constructors
		ShaderModule() = default;
		// Destructors
		inline ShaderModule(VkDevice device, VkShaderModule module) : device(device), module(module) {}
		inline ~ShaderModule() { vkDestroyShaderModule(this->device, this->module, nullptr); }
		// No copy
		ShaderModule(const ShaderModule&) = delete;
		ShaderModule& operator=(const ShaderModule&) = delete;
		// Move
		ShaderModule(ShaderModule&& other) noexcept : device(other.device), module(other.module) { other.module = nullptr; }
		ShaderModule& operator=(ShaderModule&& other) noexcept { if (this != &other) { device = other.device; module = other.module; other.module = nullptr; } return *this; }
	public:
		inline operator VkShaderModule() const { return module; }
	};
}