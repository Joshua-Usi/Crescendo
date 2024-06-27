#pragma once

#include "common.hpp"
#include "Rendering/Vulkan2/raii/CommandQueue.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	class CommandPass
	{
	public:
		virtual ~CommandPass() = default;
		virtual void Execute(Vk::GraphicsCommandQueue& cmd) = 0;
	};
}