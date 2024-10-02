#include "WindowsWindow.hpp"
#include "Interfaces/Input.hpp"

CS_NAMESPACE_BEGIN
{
	bool WindowsWindow::IsGLFWInitialised = false;
	uint32_t WindowsWindow::windowsOpen = 0;
	// Overload to create WindowsWindow
	std::unique_ptr<Window> Window::Create(const Specification& spec) { return std::make_unique<WindowsWindow>(spec); }
	WindowsWindow::WindowsWindow(const Specification& spec)
	{
		if (!IsGLFWInitialised)
		{
			int result = glfwInit();
			if (result != GLFW_TRUE) cs_std::console::fatal("Failed to initialise GLFW! ", result);
			IsGLFWInitialised = true;
		}

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		this->data.isFullScreen = spec.width == 0 || spec.height == 0;
		this->data.title = spec.title;
		this->data.width = (data.isFullScreen) ? mode->width : spec.width;
		this->data.height = (data.isFullScreen) ? mode->height : spec.height;
		this->data.isOpen = true;
		this->data.windowPointer = this;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		this->window = glfwCreateWindow(data.width, data.height, data.title.c_str(), (data.isFullScreen) ? monitor : nullptr, nullptr);
		if (this->window == nullptr) cs_std::console::fatal("Failed to create GLFW window!");
		glfwSetWindowPos(this->window, spec.positionX, spec.positionY);

		glfwSetWindowUserPointer(this->window, &this->data);
		glfwSetWindowCloseCallback(this->window, [](GLFWwindow* window) { static_cast<Data*>(glfwGetWindowUserPointer(window))->windowPointer->Close(); });

		this->input = Input::Create(Input::Specification(this));

		windowsOpen++;
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
			windowsOpen--;
			if (windowsOpen == 0)
			{
				IsGLFWInitialised = false;
				glfwTerminate();
			}
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
}