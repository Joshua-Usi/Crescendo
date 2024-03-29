#pragma once

#include "common.hpp"

#include "Volk/volk.h"

#include "vulkan/vulkan_raii.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	struct RenderPass
	{
	private:
		VkDevice device;
		VkRenderPass pass;
	public:
		// Constructors
		RenderPass() = default;
		RenderPass(VkDevice device, VkRenderPass pass) : device(device), pass(pass) {}
		// Destructors
		~RenderPass()
		{
			if (!this->device) return;
			vkDestroyRenderPass(this->device, this->pass, nullptr);
		}
		// No copy
		RenderPass(const RenderPass&) = delete;
		RenderPass& operator=(const RenderPass&) = delete;
		// Move
		RenderPass(RenderPass&& other) noexcept : device(other.device), pass(other.pass) { other.pass = nullptr; }
		RenderPass& operator=(RenderPass&& other) noexcept { if (this != &other) { device = other.device; pass = other.pass; other.pass = nullptr; } return *this; }
	public:
		operator VkRenderPass() const { return pass; }
	};
}