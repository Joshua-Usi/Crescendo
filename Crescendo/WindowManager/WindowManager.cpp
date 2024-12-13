#include "WindowManager.hpp"
#include "Console.hpp"
#include "Core.hpp"
#include "glfw/glfw3.h"

void WindowManager::OnLoad()
{
	int result = glfwInit();
	if (!result)
	{
		Console::Error("Failed to initialize GLFW");
		Core::Get()->RequestShutdown();
	}
}
void WindowManager::OnUpdate(double dt)
{
	glfwPollEvents();
}
void WindowManager::OnUnload()
{
	glfwTerminate();
}
ModuleMetadata WindowManager::GetMetadata()
{
	return
	{
		"WindowManager",
		"0.1.0",
		"GLFW window manager for Crescendo",
		"Joshua Usi",
		"",
		0.001 // 1000 Hz
	};
}

Window WindowManager::CreateWindow(int width, int height, const char* title)
{
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	// Vulkan hints
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (!window)
	{
		Console::Error("Failed to create window");
		Core::Get()->RequestShutdown();
	}

	s_windowCounts++;
	return { window };
}
void WindowManager::DestroyWindow(Window window)
{
	if (!window)
		return;
	glfwDestroyWindow(window);
	s_windowCounts--;
}