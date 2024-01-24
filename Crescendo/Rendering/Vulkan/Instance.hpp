#pragma once

#include "common.hpp"

#include "Volk/volk.h"
#include "VkBootstrap/VkBootstrap.h"

#include "Surface.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	class Instance
	{
	friend class Surface;
	friend class Device;
	friend class Swapchain;
	private:
		static bool isVolkInitialised;
		vkb::Instance vkbInstance;
		std::vector<Surface> surfaces;
	public:
		Instance() = default;
		Instance(bool useValidationLayers, const std::string& appName, const std::string& engineName, const std::vector<void*>& windows);
		~Instance();
		// No copy
		Instance(const Instance&) = delete;
		Instance& operator=(const Instance&) = delete;
		// Move
		Instance(Instance&& other) noexcept;
		Instance& operator=(Instance&& other) noexcept;

		operator VkInstance() const { return vkbInstance.instance; }
		operator const vkb::Instance&() const { return vkbInstance; }
		VkInstance GetInstance() const { return vkbInstance.instance; }

		Surface& GetSurface(uint32_t index) { return surfaces[index]; }
	};
}