#pragma once

#include "common.hpp"
#include "raii/CommandQueue.hpp"
#include "raii/Semaphore.hpp"
#include "raii/Fence.hpp"
#include "Device.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	struct FrameResources
	{
		Vk::GraphicsCommandQueue commandBuffer;
		Vk::Semaphore imageAvailable, renderFinished;
		Vk::Fence inFlight;
		VkDescriptorSet globalDescriptorSet;
	};

	class FrameManager
	{
	private:
		std::vector<FrameResources> frames;
		uint32_t currentFrame;
	public:
		FrameManager();
		FrameManager(const Device& device, uint32_t frameCount);
		~FrameManager();
		FrameManager(const FrameManager&) = delete;
		FrameManager& operator=(const FrameManager&) = delete;
		FrameManager(FrameManager&& other) noexcept;
		FrameManager& operator=(FrameManager&& other) noexcept;
	public:
		FrameResources& GetCurrentFrame();
		uint32_t GetCurrentFrameIndex();
		uint32_t GetFrameCount();
		void AdvanceFrame();

	};
}