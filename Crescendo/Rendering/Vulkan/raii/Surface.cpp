#include "Surface.hpp"
#include "volk/volk.h"
#include "glfw/glfw3.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	Surface::Surface() : instance(nullptr), surface(nullptr) {}
	Surface::Surface(VkInstance instance, void* window) : instance(instance)
	{
		if (glfwCreateWindowSurface(instance, static_cast<GLFWwindow*>(window), nullptr, &this->surface) != VK_SUCCESS) cs_std::console::fatal("Failed to create window surface!");
	}
	Surface::~Surface()
	{
		if (this->surface == nullptr) return;
		vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
	}
	Surface::Surface(Surface&& other) noexcept : instance(other.instance), surface(other.surface)
	{
		other.instance = nullptr;
		other.surface = nullptr;
	}
	Surface& Surface::operator=(Surface&& other) noexcept
	{
		if (this == &other) return *this;
		this->instance = other.instance; other.instance = nullptr;
		this->surface = other.surface; other.surface = nullptr;
		return *this;
	}
	Surface::operator VkSurfaceKHR() const { return surface; }
	VkSurfaceKHR Surface::GetSurface() const { return surface; }
}