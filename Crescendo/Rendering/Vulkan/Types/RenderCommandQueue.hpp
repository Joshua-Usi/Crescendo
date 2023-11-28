#pragma once

#include "common.hpp"

#include "Volk/volk.h"

#include "../CommandQueue.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	struct RenderCommandQueue
	{
	public:
		GraphicsCommandQueue cmd;
		VkSemaphore presentReady, renderFinish;
	public:
		// Constructors
		RenderCommandQueue() = default;
		RenderCommandQueue(GraphicsCommandQueue&& cmd, VkSemaphore&& presentReady, VkSemaphore&& renderFinish) noexcept : cmd(std::move(cmd)), presentReady(presentReady), renderFinish(renderFinish) {}
		// Destructors
		~RenderCommandQueue()
		{
			vkDestroySemaphore(this->cmd.GetDevice(), this->presentReady, nullptr);
			vkDestroySemaphore(this->cmd.GetDevice(), this->renderFinish, nullptr);
		}
		// No copy
		RenderCommandQueue(const RenderCommandQueue&) = delete;
		RenderCommandQueue& operator=(const RenderCommandQueue&) = delete;
		// Move
		RenderCommandQueue(RenderCommandQueue&& other) noexcept : cmd(std::move(other.cmd)), presentReady(other.presentReady), renderFinish(other.renderFinish)
		{
			other.presentReady = nullptr;
			other.renderFinish = nullptr;
		}
		RenderCommandQueue& operator=(RenderCommandQueue&& other) noexcept
		{
			if (this != &other)
			{
				this->cmd = std::move(other.cmd);
				this->presentReady = other.presentReady;
				this->renderFinish = other.renderFinish;
				other.presentReady = nullptr;
				other.renderFinish = nullptr;
			}
			return *this;
		}
		operator GraphicsCommandQueue& () noexcept { return this->cmd; }
	};
}