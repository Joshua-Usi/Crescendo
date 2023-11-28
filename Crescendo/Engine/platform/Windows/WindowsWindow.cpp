#include "WindowsWindow.hpp"

#include "Engine/interfaces/Input.hpp"

CS_NAMESPACE_BEGIN
{
	static bool IsGLFWInitialised = false;
	WindowsWindow::WindowsWindow(const Properties& props)
	{
		this->data.title = props.title;
		this->data.width = props.width;
		this->data.height = props.height;
		this->data.isOpen = true;
		this->data.windowPointer = this;

		if (!IsGLFWInitialised)
		{
			CS_ASSERT(glfwInit(), "Failed to Initialise GLFW!");
			IsGLFWInitialised = true;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		this->window = glfwCreateWindow(this->data.width, this->data.height, this->data.title.c_str(), NULL, NULL);

		glfwSetWindowUserPointer(this->window, &this->data);
		glfwSetWindowCloseCallback(this->window, [](GLFWwindow* window) { static_cast<Data*>(glfwGetWindowUserPointer(window))->windowPointer->Close(); });
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
	uint32_t WindowsWindow::GetWidth() const
	{
		return this->data.width;
	}
	uint32_t WindowsWindow::GetHeight() const
	{
		return this->data.height;
	}
	float WindowsWindow::GetAspectRatio() const
	{
		return static_cast<float>(this->GetWidth()) / static_cast<float>(this->GetHeight());
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
	// TODO
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
	void WindowsWindow::SetFullScreen(bool isFullScreen)
	{
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		this->data.windowedWidth = this->data.width;
		this->data.windowedHeight = this->data.height;

		if (isFullScreen && !this->data.isFullScreen)
		{
			this->data.isFullScreen = true;
			// Get width and height of monitor
			glfwSetWindowMonitor(this->window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
		}
		else if (this->data.isFullScreen)
		{
			this->data.isFullScreen = false;
			glfwSetWindowMonitor(this->window, nullptr, (mode->width - this->data.windowedWidth) / 2, (mode->height - this->data.windowedHeight) / 2, this->data.windowedWidth, this->data.windowedHeight, GLFW_DONT_CARE);
		}
	}
	void WindowsWindow::SetSize(uint32_t width, uint32_t height)
	{
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		this->data.width = width;
		this->data.height = height;
		glfwSetWindowMonitor(this->window, nullptr, (mode->width - width) / 2, (mode->height - height) / 2, width, height, GLFW_DONT_CARE);
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
	bool WindowsWindow::IsFullScreen() const
	{
		return this->data.isFullScreen;
	}
	std::unique_ptr<Window> Window::Create(const Properties& props)
	{
		return std::make_unique<WindowsWindow>(props);
	}
}