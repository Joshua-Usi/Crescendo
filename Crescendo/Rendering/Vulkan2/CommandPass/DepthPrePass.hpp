#pragma once

#include "common.hpp"
#include "CommandPass.hpp"
#include "Rendering/Vulkan2/Surface.hpp"
#include "Rendering/Vulkan2/raii.hpp"

CS_NAMESPACE_BEGIN
{
	class DepthPrePass : public Vulkan::CommandPass
	{
	private:
		Vulkan::Surface& surface;
		Vulkan::Vk::RenderPass renderPass;
		Vulkan::Vk::Framebuffer framebuffer;
		Vulkan::Vk::Pipeline depthPipeline;
	public:
		DepthPrePass(Vulkan::Surface& surface, const std::string& shaderName, VkFormat depthFormat, VkSampleCountFlagBits samples);
		void Execute(Vulkan::Vk::GraphicsCommandQueue& cmd) override final;
	};
}