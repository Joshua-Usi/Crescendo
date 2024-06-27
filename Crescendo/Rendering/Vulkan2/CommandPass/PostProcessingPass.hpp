#pragma once

#include "common.hpp"
#include "CommandPass.hpp"
#include "Rendering/Vulkan2/Surface.hpp"
#include "Rendering/Vulkan2/raii.hpp"

CS_NAMESPACE_BEGIN
{
	class PostProcessPass : public Vulkan::CommandPass
	{
	private:
		Vulkan::Surface& surface;
		Vulkan::Vk::Pipeline pipeline;
	public:
		PostProcessPass(Vulkan::Surface& surface, const std::string& shaderName);
		void Execute(Vulkan::Vk::GraphicsCommandQueue& cmd) override final;
	};
}