#include "WindowsWindow.hpp"

namespace Crescendo::Engine
{
	static bool IsGLFWInitialised = false;
	WindowsWindow::WindowsWindow(const WindowProperties& props)
	{
		// Set window properties
		this->data.title = props.title;
		this->data.width = props.width;
		this->data.height = props.height;
		this->data.isOpen = true;
		this->data.windowPointer = this;

		if (!IsGLFWInitialised)
		{
			bool success = glfwInit();
			CS_ASSERT(success, "Failed to Initialise GLFW!");
			IsGLFWInitialised = true;
		}
		// Hint no api for creating a vulkan interface
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		// Disable resizing for now
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		// Create window
		this->window = glfwCreateWindow(this->data.width, this->data.height, this->data.title.c_str(), NULL, NULL);
	
		// Hook user pointer
		glfwSetWindowUserPointer(this->window, &this->data);

		// Create window close callback, hook to window
		glfwSetWindowCloseCallback(this->window, [](GLFWwindow* window)
		{
			WindowData* windowPointer = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
			windowPointer->windowPointer->Close();
		});
	}
	WindowsWindow::~WindowsWindow()
	{
		this->Close();
	}
	void WindowsWindow::Close()
	{
		if (this->data.isOpen)
		{
			this->data.isOpen = false;
			glfwDestroyWindow(this->window);
			glfwTerminate();
			IsGLFWInitialised = false;
		}
	}
	void WindowsWindow::OnUpdate()
	{
		if (this->data.isOpen)
		{
			glfwPollEvents();
		}
	}
	void WindowsWindow::OnLateUpdate()
	{
		if (this->data.isOpen)
		{
			// Should be buffer swap
		}
	}
	int32_t WindowsWindow::GetWidth() const
	{
		return this->data.width;
	}
	int32_t WindowsWindow::GetHeight() const
	{
		return this->data.height;
	}
	void* WindowsWindow::GetNative() const
	{
		return this->window;
	}
	int32_t WindowsWindow::GetRefreshRate() const
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		// Sometimes monitor is nullptr on release builds, if it does, just return 0
		if (monitor == nullptr) return 0;
		return glfwGetVideoMode(monitor)->refreshRate;
	}
	std::string WindowsWindow::GetName() const
	{
		return this->data.title;
	}
	void WindowsWindow::SetVSync(bool isEnabled)
	{
		if (isEnabled)
		{
			// Enable vsync
		}
		else
		{
			// Disable vsync
		}
		this->data.vSync = isEnabled;
	}
	void WindowsWindow::SetCursorLock(bool isEnabled)
	{
		glfwSetInputMode(window, GLFW_CURSOR, (isEnabled) ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
		this->data.isCursorLocked = isEnabled;
	}
	void WindowsWindow::SetName(const std::string& name)
	{
		glfwSetWindowTitle(this->window, name.c_str());
	}
	bool WindowsWindow::IsVSynced() const
	{
		return this->data.vSync;
	}
	bool WindowsWindow::IsCursorLocked() const
	{
		return this->data.isCursorLocked;
	}
	bool WindowsWindow::IsOpen() const
	{
		return this->data.isOpen;
	}
	shared<Window> Window::Create(const WindowProperties& props)
	{
		return std::make_shared<WindowsWindow>(props);
	}
}