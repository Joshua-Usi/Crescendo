#pragma once

#include "Volk/volk.h"
#include "VkBootstrap/VkBootstrap.h"

#include "cs_std/function_queue.hpp"

#include "Device.hpp"
#include "Swapchain.hpp"

#include <string>
#include <iostream>

namespace Crescendo::Vulkan
{
	class Instance
	{
	private:
		static bool isVolkInitialised;
		/* 
		 *	Stores useful information about the physical device
		 *	As well as allows us to construct devices, Takes up 2kb of stack space
		 *	But I reckon it's fine
		 */
		vkb::PhysicalDevice vkbPhysicalDevice;
		cs_std::function_queue deletionQueue;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;
		void* windowPtr;
	public:
		Instance() = default;
		Instance(bool useValidationLayers, const std::string& appName, const std::string& engineName, void* windowPtr);
		~Instance();
		// No copy
		Instance(const Instance&) = delete;
		Instance& operator=(const Instance&) = delete;
		// Move
		Instance(Instance&& other) noexcept;
		Instance& operator=(Instance&& other) noexcept;
		// Create a new device, the device will destroy itself implicitly
		Device CreateDevice(uint32_t descriptorSetsPerPool);
		// Create a new swapchain, the swapchain will destroy itself implicitly
		Swapchain CreateSwapchain(VkDevice device, VkPresentModeKHR presentMode, VkExtent2D extent);

		VkInstance GetInstance() const { return instance; }
		VkSurfaceKHR GetSurface() const { return surface; }
		VkPhysicalDevice GetPhysicalDevice() const { return vkbPhysicalDevice.physical_device; }
		void* GetWindow() const { return windowPtr; }
		operator VkInstance() const { return GetInstance(); }

		const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() const { return vkbPhysicalDevice.properties; }
	};
}