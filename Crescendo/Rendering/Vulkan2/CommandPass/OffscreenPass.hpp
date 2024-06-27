#pragma once

#include "common.hpp"
#include "CommandPass.hpp"

CS_NAMESPACE_BEGIN
{
	class OffscreenPass : public Vulkan::CommandPass
	{
	private:
		//Vulkan::Surface& surface;
		Vulkan::Vk::RenderPass renderPass;
		Vulkan::Vk::Framebuffer framebuffer;
		Vulkan::Vk::Pipeline skyboxPipeline;
	public:
		OffscreenPass();
		virtual void Execute(Vulkan::Vk::GraphicsCommandQueue& cmd) override final;
	};
}