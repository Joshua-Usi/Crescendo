#include "WindowsWindow.h"

#include "console/console.h"
#include "glad/glad.h"

namespace Crescendo::Engine
{
	static bool InitialisedGLFW = false;
	Window* Window::Create(const WindowProps& props)
	{
		return new WindowsWindow(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		data.title = props.title;
		data.width = props.width;
		data.height = props.height;
		data.isOpen = true;
		data.windowPointer = this;

		Console::EngineInfo("Creating window \"{}\" of size {}x{}", props.title, props.width, props.height);

		// Initialise glfw if it hasn't been yet
		if (!InitialisedGLFW)
		{
			CS_ASSERT(glfwInit(), "GLFW Initialisation Failed!");
			InitialisedGLFW = true;
		}

		// first thing after initialisation
		glfwSetErrorCallback([](int error_code, const char* description)
		{
			Engine::Console::EngineError("GLFW Error ({}): {}", error_code, description);
		});

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		window = glfwCreateWindow(data.width, data.height, data.title.c_str(), NULL, NULL);
		glfwMakeContextCurrent(window);

		// why does the glad loader check look like this????
		CS_ASSERT(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "GLAD Initialisation Failed!");

		glfwSetWindowUserPointer(window, &data);

		glfwSetWindowCloseCallback(window, [](GLFWwindow* window)
		{
			WindowData windowPointer = CastVoid(WindowData, glfwGetWindowUserPointer(window));
			windowPointer.windowPointer->Shutdown();
		});

		SetVSync(false);
	}

	void WindowsWindow::Shutdown()
	{
		data.isOpen = false;
		glfwDestroyWindow(window);
	}

	void WindowsWindow::OnUpdate()
	{
		if (data.isOpen)
		{
			glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			//glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}

	gt::Int32 WindowsWindow::GetWidth() const
	{
		return data.width;
	}

	gt::Int32 WindowsWindow::GetHeight() const
	{
		return data.height;
	}

	gt::Int32 WindowsWindow::GetRefreshRate() const
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		return glfwGetVideoMode(monitor)->refreshRate;
	}

	void WindowsWindow::SetVSync(bool isEnabled)
	{
		if (isEnabled)
		{
			glfwSwapInterval(1);
		}
		else
		{
			glfwSwapInterval(0);
		}
		data.vSync = isEnabled;
	}

	bool WindowsWindow::IsVSynced() const
	{
		return data.vSync;
	}

	bool WindowsWindow::IsOpen() const
	{
		return data.isOpen;
	}

	void* WindowsWindow::GetNative() const
	{
		return window;
	}
}