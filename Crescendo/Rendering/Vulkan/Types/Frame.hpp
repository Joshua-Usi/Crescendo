#pragma once

#include "Volk/volk.h"

#include "CommandQueue.hpp"

namespace Crescendo::Vulkan
{
	struct Frame
	{
	public:
		Crescendo::Vulkan::GraphicsCommandQueue cmd;
		VkSemaphore presentReady, renderFinish;
	public:
		// Constructors
		Frame() = default;
		Frame(Crescendo::Vulkan::GraphicsCommandQueue&& cmd, VkSemaphore&& presentReady, VkSemaphore&& renderFinish) noexcept : cmd(std::move(cmd)), presentReady(presentReady), renderFinish(renderFinish) {}
		// Destructors
		~Frame()
		{
			vkDestroySemaphore(this->cmd.GetDevice(), this->presentReady, nullptr);
			vkDestroySemaphore(this->cmd.GetDevice(), this->renderFinish, nullptr);
		}
		// No copy
		Frame(const Frame&) = delete;
		Frame& operator=(const Frame&) = delete;
		// Move
		Frame(Frame&& other) noexcept : cmd(std::move(other.cmd)), presentReady(other.presentReady), renderFinish(other.renderFinish)
		{
			other.presentReady = nullptr;
			other.renderFinish = nullptr;
		}
		Frame& operator=(Frame&& other) noexcept
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
	};
}