#include "FrameManager.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	FrameManager::FrameManager() : frames(), currentFrame(0) {}
	FrameManager::FrameManager(const Device& device, uint32_t frameCount) : currentFrame(0)
	{
		for (uint32_t i = 0; i < frameCount; i++)
		{
			this->frames.emplace_back(
				Vk::GraphicsCommandQueue(device, device.GetUniversalQueue(), true),
				Vk::Semaphore(device), Vk::Semaphore(device),
				Vk::Fence(device, VK_FENCE_CREATE_SIGNALED_BIT)
			);
		}
	}
	FrameManager::~FrameManager()
	{
		this->frames.clear();
	}
	FrameManager::FrameManager(FrameManager&& other) noexcept : frames(std::move(other.frames)), currentFrame(other.currentFrame) {}
	FrameManager& FrameManager::operator=(FrameManager&& other) noexcept
	{
		this->frames = std::move(other.frames);
		this->currentFrame = other.currentFrame;
		return *this;
	}
	FrameResources& FrameManager::GetCurrentFrame() { return this->frames[this->currentFrame]; }
	uint32_t FrameManager::GetCurrentFrameIndex() { return this->currentFrame; }
	void FrameManager::AdvanceFrame() { this->currentFrame = (this->currentFrame + 1) % this->frames.size(); }
}