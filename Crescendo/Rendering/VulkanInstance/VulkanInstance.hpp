#pragma once

#include "../Vulkan/Instance.hpp"

namespace Crescendo
{
	class Texture
	{
	private:
		uint32_t id;
	};


	class VulkanInstance
	{
	private:
		// Fixed setup
		Vulkan::Instance instance;
		Vulkan::Device device;
		Vulkan::TransferCommandQueue transferQueue;

		// Variable setup
		uint8_t frameIndex = 0;
		std::vector<Crescendo::Vulkan::Frame> frameData;
		Crescendo::Vulkan::Swapchain swapchain;
	};
}