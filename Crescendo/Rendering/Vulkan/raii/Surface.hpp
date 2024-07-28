#pragma once

#include "common.hpp"
#include "vulkan/vulkan.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class Surface
	{
	private:
		VkInstance instance;
		VkSurfaceKHR surface;
	public:
		Surface();
		Surface(VkInstance instance, void* window);
		~Surface();
		Surface(const Surface&) = delete;
		Surface& operator=(const Surface&) = delete;
		Surface(Surface&& other) noexcept;
		Surface& operator=(Surface&& other) noexcept;
	public:
		operator VkSurfaceKHR() const;
		VkSurfaceKHR GetSurface() const;
	};
}