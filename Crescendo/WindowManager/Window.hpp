#pragma once
#include "glfw/glfw3.h"

class Window
{
private:
	GLFWwindow* m_window;
public:
	Window() : m_window(nullptr) {}
	Window(GLFWwindow* window) : m_window(window) {}
	operator GLFWwindow* () { return m_window; }
	operator const GLFWwindow* () const { return m_window; }
	operator bool() const { return m_window; }
};