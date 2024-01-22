#pragma once

#include "common.hpp"

#include "Volk/volk.h"
#include "VkBootstrap/VkBootstrap.h"


CS_NAMESPACE_BEGIN::Vulkan
{
	class Instance
	{
	friend class Device;
	friend class Swapchain;
	private:
		static bool isVolkInitialised;
		/* 
		 *	Stores useful information about the physical device
		 *	As well as allows us to construct devices, Takes up 2kb of stack space
		 *	But I reckon it's fine
		 */
		vkb::PhysicalDevice vkbPhysicalDevice;
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

		VkSurfaceKHR GetSurface() const { return surface; }
		VkPhysicalDevice GetPhysicalDevice() const { return vkbPhysicalDevice.physical_device; }
		void* GetWindow() const { return windowPtr; }

		operator VkInstance() const { return instance; }

		const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() const { return vkbPhysicalDevice.properties; }
	};
}