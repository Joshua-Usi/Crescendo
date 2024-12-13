#pragma once
#include "Interfaces/Module.hpp"
#include "Console.hpp"
#include "Core.hpp"
#include "glfw/glfw3.h"
using namespace CrescendoEngine;

class Window
{
private:
	GLFWwindow* m_window;
public:
	Window() : m_window(nullptr) {}
	Window(GLFWwindow* window) : m_window(window) {}
	operator GLFWwindow*() { return m_window; }
	operator const GLFWwindow*() const { return m_window; }
	operator bool() const { return m_window; }
};

class WindowManager : public Module
{
private:
	size_t s_windowCounts = 0;
public:
	void OnLoad() override
	{
		int result = glfwInit();
		if (!result)
		{
			Console::Error("Failed to initialize GLFW");
			Core::Get()->RequestShutdown();
		}
	}
	void OnUnload() override
	{
		glfwTerminate();
	}
	void OnUpdate(double dt) override {}
	static ModuleMetadata GetMetadata()
	{
		return
		{
			"WindowManager",
			"0.1.0",
			"GLFW window manager for Crescendo",
			"Joshua Usi",
			"",
			-1.0
		};
	}

	Window CreateWindow(int width, int height, const char* title)
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
	void DestroyWindow(Window window)
	{
		if (!window)
			return;
		glfwDestroyWindow(window);
		s_windowCounts--;
	}
};